#include "ppc/fp.h"

#include <bit>
#include <cmath>
#include <cstring>

namespace mvp::ppc::fp {

namespace {

// ---- 128-bit unsigned arithmetic (portable; no compiler extensions) ----

struct U128 {
  u64 hi = 0, lo = 0;
};

bool nz(U128 a) { return (a.hi | a.lo) != 0; }

U128 mul64(u64 a, u64 b) {
  u64 a0 = a & 0xFFFFFFFFu, a1 = a >> 32, b0 = b & 0xFFFFFFFFu, b1 = b >> 32;
  u64 p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  u64 mid = (p00 >> 32) + (p01 & 0xFFFFFFFFu) + (p10 & 0xFFFFFFFFu);
  U128 r;
  r.lo = (mid << 32) | (p00 & 0xFFFFFFFFu);
  r.hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
  return r;
}

U128 shl(U128 a, int n) {
  if (n <= 0) return a;
  if (n >= 128) return {};
  if (n >= 64) return {a.lo << (n - 64), 0};
  return {(a.hi << n) | (a.lo >> (64 - n)), a.lo << n};
}

// Right shift; any 1 bit shifted out is OR-ed into bit 0 ("jamming"), which keeps
// rounding exact as long as bit 0 stays below the rounding position.
U128 shr_jam(U128 a, int n) {
  if (n <= 0) return a;
  if (n >= 128) return {0, nz(a) ? 1u : 0u};
  U128 r;
  bool lost;
  if (n >= 64) {
    lost = a.lo != 0 || (n > 64 && (a.hi << (128 - n)) != 0);
    r = {0, n == 64 ? a.hi : a.hi >> (n - 64)};
  } else {
    lost = (a.lo << (64 - n)) != 0;
    r = {a.hi >> n, (a.lo >> n) | (a.hi << (64 - n))};
  }
  if (lost) r.lo |= 1;
  return r;
}

U128 shr(U128 a, int n) {
  if (n <= 0) return a;
  if (n >= 128) return {};
  if (n >= 64) return {0, a.hi >> (n - 64)};
  return {a.hi >> n, (a.lo >> n) | (a.hi << (64 - n))};
}

U128 add128(U128 a, U128 b) {
  U128 r{a.hi + b.hi, a.lo + b.lo};
  if (r.lo < a.lo) ++r.hi;
  return r;
}

U128 sub128(U128 a, U128 b) {  // requires a >= b
  U128 r{a.hi - b.hi, a.lo - b.lo};
  if (a.lo < b.lo) --r.hi;
  return r;
}

bool less128(U128 a, U128 b) { return a.hi < b.hi || (a.hi == b.hi && a.lo < b.lo); }

int clz64(u64 x) {
  if (!x) return 64;
  int n = 0;
  while (!(x & 0x8000000000000000ull)) {
    x <<= 1;
    ++n;
  }
  return n;
}

int msb128(U128 a) {  // index of the highest set bit; a != 0
  return a.hi ? 127 - clz64(a.hi) : 63 - clz64(a.lo);
}

// Bits [0, n) of a, as U128.
U128 low_bits(U128 a, int n) {
  if (n <= 0) return {};
  if (n >= 128) return a;
  if (n >= 64) return {n == 64 ? 0 : a.hi & ((1ull << (n - 64)) - 1), a.lo};
  return {0, a.lo & ((1ull << n) - 1)};
}

// ---- unpacked numbers ----

enum Kind { kZero, kFinite, kInf, kNaN };

struct Num {
  bool sign = false;
  Kind kind = kZero;
  u64 m = 0;  // finite: value = m * 2^e, m != 0
  int e = 0;
};

Num unpack(u64 x) {
  Num n;
  n.sign = (x & kSign) != 0;
  u32 ef = static_cast<u32>((x >> 52) & 0x7FF);
  u64 f = x & kFracMask;
  if (ef == 0x7FF) {
    n.kind = f ? kNaN : kInf;
  } else if (ef == 0) {
    if (f) {
      n.kind = kFinite;
      n.m = f;
      n.e = -1074;
    }
  } else {
    n.kind = kFinite;
    n.m = f | (1ull << 52);
    n.e = static_cast<int>(ef) - 1075;
  }
  return n;
}

u64 signed_zero(bool sign) { return sign ? kSign : 0; }
u64 signed_inf(bool sign) { return (sign ? kSign : 0) | kExpMask; }

int rounding_mode(u32 fpscr) { return static_cast<int>(fpscr & 3); }
bool ni(u32 fpscr) { return (fpscr & 4) != 0; }

// Rounds (-1)^sign * m * 2^e (m != 0; bit 0 may be a jammed sticky bit) once to
// the target precision and packs it as double bits.
u64 round_pack(bool sign, U128 m, int e, Prec p, u32 fpscr) {
  const int P = p == Prec::Single ? 24 : 53;
  const int emin = p == Prec::Single ? -126 : -1022;
  const int emax = p == Prec::Single ? 127 : 1023;
  int n = msb128(m);
  int q = n + e;  // exponent of the leading bit
  if (ni(fpscr) && p == Prec::Single && q < emin) return signed_zero(sign);
  int lsb_e = q - (P - 1);
  if (lsb_e < emin - (P - 1)) lsb_e = emin - (P - 1);
  int shift = lsb_e - e;
  u64 mant;
  if (shift <= 0) {
    mant = shl(m, -shift).lo;  // exact; fits in P bits
  } else {
    U128 keep = shr(m, shift);
    U128 rem = low_bits(m, shift);
    bool inexact = nz(rem);
    // Compare the discarded bits with one half unit in the last place.
    bool above_half, at_half;
    if (shift - 1 >= 128) {
      above_half = at_half = false;  // m < 2^128 <= half
    } else {
      U128 half = shl(U128{0, 1}, shift - 1);
      above_half = less128(half, rem);
      at_half = !above_half && !less128(rem, half);
    }
    bool up = false;
    switch (rounding_mode(fpscr)) {
      case 0:  // nearest even
        up = above_half || (at_half && (keep.lo & 1));
        break;
      case 1:  // toward zero
        break;
      case 2:  // toward +inf
        up = inexact && !sign;
        break;
      case 3:  // toward -inf
        up = inexact && sign;
        break;
    }
    mant = keep.lo + (up ? 1 : 0);
    if (mant >> P) {  // carried into a new binade
      mant >>= 1;
      ++lsb_e;
    }
  }
  if (mant == 0) return signed_zero(sign);
  int top = 63 - clz64(mant) + lsb_e;
  if (top > emax) {
    int rm = rounding_mode(fpscr);
    bool to_inf = rm == 0 || (rm == 2 && !sign) || (rm == 3 && sign);
    if (to_inf) return signed_inf(sign);
    // Largest finite value of the target precision.
    u64 maxm = (1ull << P) - 1;
    mant = maxm;
    lsb_e = emax - (P - 1);
    top = emax;
  }
  // Pack as a double.
  u64 bits;
  if (top >= -1022) {
    int s = 52 - (63 - clz64(mant));
    bits = (static_cast<u64>(top + 1023) << 52) | ((mant << s) & kFracMask);
  } else {
    // Double subnormal (only reachable for Prec::Double): lsb_e == -1074.
    if (ni(fpscr)) return signed_zero(sign);
    bits = mant << (lsb_e + 1074);
  }
  return bits | (sign ? kSign : 0);
}

u64 round_pack(bool sign, u64 m, int e, Prec p, u32 fpscr) { return round_pack(sign, U128{0, m}, e, p, fpscr); }

// A NaN going to a result of precision p.
u64 nan_result(u64 x, Prec p) {
  u64 q = x | kQuietBit;
  return p == Prec::Single ? (q & 0xFFFFFFFFE0000000ull) : q;
}

// Exact zero result of an addition (x + y == 0 with x, y not both zero of the same sign).
u64 exact_zero_sum(u32 fpscr) { return rounding_mode(fpscr) == 3 ? kSign : 0; }

// Exact a + b for finite nonzero operands (signs included), then rounding.
u64 add_finite(const Num& a, const Num& b, Prec p, u32 fpscr) {
  // Put the operand with the larger exponent first; place its mantissa at bit 70.
  const Num* x = &a;
  const Num* y = &b;
  if (y->e > x->e) {
    const Num* t = x;
    x = y;
    y = t;
  }
  int d = x->e - y->e;
  U128 X = shl(U128{0, x->m}, 70);
  U128 Y = shr_jam(shl(U128{0, y->m}, 70), d);
  int e = x->e - 70;
  if (x->sign == y->sign) return round_pack(x->sign, add128(X, Y), e, p, fpscr);
  if (less128(X, Y)) return round_pack(y->sign, sub128(Y, X), e, p, fpscr);
  U128 r = sub128(X, Y);
  if (!nz(r)) return exact_zero_sum(fpscr);
  return round_pack(x->sign, r, e, p, fpscr);
}

}  // namespace

// ---- conversions ----

u64 single_to_double(u32 s) {
  u64 sign = static_cast<u64>(s & 0x80000000u) << 32;
  u32 exp = (s >> 23) & 0xFF;
  u64 frac = s & 0x7FFFFF;
  if (exp == 0xFF) return sign | kExpMask | (frac << 29);  // inf / NaN: payload kept
  if (exp == 0) {
    if (!frac) return sign;
    // Denormalized single: normalize into a double.
    int e = -126;
    while (!(frac & 0x800000)) {
      frac <<= 1;
      --e;
    }
    return sign | (static_cast<u64>(e + 1023) << 52) | ((frac & 0x7FFFFF) << 29);
  }
  return sign | (static_cast<u64>(static_cast<int>(exp) - 127 + 1023) << 52) | (frac << 29);
}

u32 double_to_single(u64 d) {
  u32 exp = static_cast<u32>((d >> 52) & 0x7FF);
  u32 hi2 = static_cast<u32>((d >> 32) & 0xC0000000u);
  if (exp > 896 || (d & ~kSign) == 0) return hi2 | static_cast<u32>((d >> 29) & 0x3FFFFFFF);
  if (exp >= 874) {
    // Denormalize: shift the significand (with its hidden bit) right; truncate.
    u32 t = static_cast<u32>(0x80000000u | ((d & kFracMask) >> 21));
    t >>= (905 - exp);
    return t | static_cast<u32>((d >> 32) & 0x80000000u);
  }
  // Below the single denormal range: the architecture leaves this undefined; the
  // plain bit selection matches what the hardware stores (as Dolphin measured).
  return hi2 | static_cast<u32>((d >> 29) & 0x3FFFFFFF);
}

Model g_model = Model::Fast;

u64 force25(u64 x) { return (x & 0xFFFFFFFFF8000000ull) + (x & 0x8000000ull); }

// ---- arithmetic ----

namespace {

u64 add_impl(u64 ab, u64 bb, bool negate_b, Prec p, u32 fpscr) {
  if (is_nan(ab)) return nan_result(ab, p);
  if (is_nan(bb)) return nan_result(bb, p);
  Num a = unpack(ab), b = unpack(bb);
  if (negate_b) b.sign = !b.sign;
  if (a.kind == kInf || b.kind == kInf) {
    if (a.kind == kInf && b.kind == kInf && a.sign != b.sign) return kDefaultNaN;
    return signed_inf(a.kind == kInf ? a.sign : b.sign);
  }
  if (a.kind == kZero && b.kind == kZero) return a.sign == b.sign ? signed_zero(a.sign) : exact_zero_sum(fpscr);
  if (a.kind == kZero) return round_pack(b.sign, b.m, b.e, p, fpscr);
  if (b.kind == kZero) return round_pack(a.sign, a.m, a.e, p, fpscr);
  return add_finite(a, b, p, fpscr);
}

}  // namespace

namespace ref {

u64 add(u64 a, u64 b, Prec p, u32 fpscr) { return add_impl(a, b, false, p, fpscr); }
u64 sub(u64 a, u64 b, Prec p, u32 fpscr) { return add_impl(a, b, true, p, fpscr); }

u64 mul(u64 ab, u64 cb, Prec p, u32 fpscr) {
  if (is_nan(ab)) return nan_result(ab, p);
  if (is_nan(cb)) return nan_result(cb, p);
  Num a = unpack(ab), c = unpack(cb);
  bool sign = a.sign != c.sign;
  if (a.kind == kInf || c.kind == kInf) {
    if (a.kind == kZero || c.kind == kZero) return kDefaultNaN;
    return signed_inf(sign);
  }
  if (a.kind == kZero || c.kind == kZero) return signed_zero(sign);
  return round_pack(sign, mul64(a.m, c.m), a.e + c.e, p, fpscr);
}

u64 div(u64 ab, u64 bb, Prec p, u32 fpscr) {
  if (is_nan(ab)) return nan_result(ab, p);
  if (is_nan(bb)) return nan_result(bb, p);
  Num a = unpack(ab), b = unpack(bb);
  bool sign = a.sign != b.sign;
  if (a.kind == kInf) return b.kind == kInf ? kDefaultNaN : signed_inf(sign);
  if (b.kind == kInf) return signed_zero(sign);
  if (b.kind == kZero) return a.kind == kZero ? kDefaultNaN : signed_inf(sign);
  if (a.kind == kZero) return signed_zero(sign);
  // Normalize both significands to 53 bits.
  u64 ma = a.m, mb = b.m;
  int ea = a.e, eb = b.e;
  while (!(ma >> 52)) {
    ma <<= 1;
    --ea;
  }
  while (!(mb >> 52)) {
    mb <<= 1;
    --eb;
  }
  // q = floor(ma * 2^60 / mb), by restoring division; the remainder is sticky.
  u64 q = 0, r = ma;
  if (r >= mb) {
    q = 1;
    r -= mb;
  }
  for (int i = 0; i < 60; ++i) {
    r <<= 1;
    q <<= 1;
    if (r >= mb) {
      r -= mb;
      q |= 1;
    }
  }
  // q has 60 or 61 bits; jam the remainder into a new bit 0.
  U128 m{0, q};
  m = shl(m, 1);
  if (r) m.lo |= 1;
  return round_pack(sign, m, ea - eb - 61, p, fpscr);
}

u64 madd(u64 ab, u64 cb, u64 bb, bool negate_b, bool negate_result, Prec p, u32 fpscr) {
  if (is_nan(ab)) return nan_result(ab, p);
  if (is_nan(bb)) return nan_result(bb, p);
  if (is_nan(cb)) return nan_result(cb, p);
  Num a = unpack(ab), c = unpack(cb), b = unpack(bb);
  if (negate_b) b.sign = !b.sign;
  bool ps = a.sign != c.sign;  // product sign
  u64 r;
  if (a.kind == kInf || c.kind == kInf) {
    if (a.kind == kZero || c.kind == kZero) return kDefaultNaN;
    if (b.kind == kInf && b.sign != ps) return kDefaultNaN;
    r = signed_inf(ps);
  } else if (b.kind == kInf) {
    r = signed_inf(b.sign);
  } else if (a.kind == kZero || c.kind == kZero) {
    // Zero product: the sum is b (or a zero with the addition sign rules).
    if (b.kind == kZero) r = ps == b.sign ? signed_zero(ps) : exact_zero_sum(fpscr);
    else r = round_pack(b.sign, b.m, b.e, p, fpscr);
  } else {
    U128 prod = mul64(a.m, c.m);
    int pe = a.e + c.e;
    if (b.kind == kZero) {
      r = round_pack(ps, prod, pe, p, fpscr);
    } else {
      // Align the larger-magnitude operand's leading bit at bit 125; shift the
      // other right with jamming (exact whenever cancellation can occur).
      int pm = msb128(prod), bm = 63 - clz64(b.m);
      int ptop = pm + pe, btop = bm + b.e;
      U128 P, B;
      int e;
      if (ptop >= btop) {
        e = pe - (125 - pm);
        P = shl(prod, 125 - pm);
        int sh = (b.e - e);  // position of b's bit 0 relative to e
        B = sh >= 0 ? shl(U128{0, b.m}, sh) : shr_jam(U128{0, b.m}, -sh);
      } else {
        e = b.e - (125 - bm);
        B = shl(U128{0, b.m}, 125 - bm);
        int sh = (pe - e);
        P = sh >= 0 ? shl(prod, sh) : shr_jam(prod, -sh);
      }
      if (ps == b.sign) {
        r = round_pack(ps, add128(P, B), e, p, fpscr);
      } else if (less128(P, B)) {
        r = round_pack(b.sign, sub128(B, P), e, p, fpscr);
      } else {
        U128 d = sub128(P, B);
        r = nz(d) ? round_pack(ps, d, e, p, fpscr) : exact_zero_sum(fpscr);
      }
    }
  }
  if (negate_result && !is_nan(r)) r ^= kSign;
  return r;
}

u64 round_to_single(u64 x, u32 fpscr) {
  if (is_nan(x)) return nan_result(x, Prec::Single);
  Num n = unpack(x);
  if (n.kind == kInf || n.kind == kZero) return x;
  return round_pack(n.sign, n.m, n.e, Prec::Single, fpscr);
}

}  // namespace ref

u64 to_int32(u64 x, bool toward_zero, u32 fpscr) {
  u32 value;
  Num n = unpack(x);
  if (n.kind == kNaN) {
    value = 0x80000000u;
  } else if (n.kind == kInf) {
    value = n.sign ? 0x80000000u : 0x7FFFFFFFu;
  } else if (n.kind == kZero) {
    value = 0;
  } else {
    // Integer part and the bits below it.
    int rm = toward_zero ? 1 : rounding_mode(fpscr);
    u64 ip;  // integer part, saturated at 2^63 (far beyond the int32 range)
    bool frac_nz = false, above_half = false, at_half = false;
    if (n.e >= 0) {
      ip = n.e > 10 ? (1ull << 63) : n.m << n.e;
    } else if (n.e <= -64) {
      ip = 0;
      frac_nz = true;  // 0 < |x| < 1/2
    } else {
      int s = -n.e;
      ip = s >= 64 ? 0 : n.m >> s;
      u64 fr = n.m & ((1ull << s) - 1);
      u64 half = 1ull << (s - 1);
      frac_nz = fr != 0;
      above_half = fr > half;
      at_half = fr == half;
    }
    bool up = false;
    switch (rm) {
      case 0: up = above_half || (at_half && (ip & 1)); break;
      case 1: break;
      case 2: up = frac_nz && !n.sign; break;
      case 3: up = frac_nz && n.sign; break;
    }
    if (up) ++ip;
    if (!n.sign) value = ip > 0x7FFFFFFFull ? 0x7FFFFFFFu : static_cast<u32>(ip);
    else value = ip > 0x80000000ull ? 0x80000000u : static_cast<u32>(0 - ip);
  }
  u64 r = 0xFFF8000000000000ull | value;
  if (value == 0 && (x & kSign)) r |= 0x100000000ull;  // hardware quirk for negative sources
  return r;
}

namespace {

// Correctly rounded square root of a finite positive value.
u64 sqrt_rn(u64 x) {
  Num n = unpack(x);
  u64 m = n.m;
  int e = n.e;
  while (!(m >> 52)) {
    m <<= 1;
    --e;
  }
  // Make the exponent even, then take isqrt(m * 2^k) with k even and large enough
  // for 55 result bits.
  if (e & 1) {
    m <<= 1;
    --e;
  }
  U128 v = shl(U128{0, m}, 56);  // m has 53-54 bits: v has 109-110 bits
  int ve = e - 56;
  // Digit-by-digit integer square root of v (two bits per step).
  U128 res{}, rem{};
  for (int i = 63; i >= 0; --i) {
    int pos = 2 * i;
    u64 bits = pos >= 64 ? (v.hi >> (pos - 64)) & 3 : (v.lo >> pos) & 3;
    rem = shl(rem, 2);
    rem.lo |= bits;
    U128 trial = add128(shl(res, 2), U128{0, 1});
    res = shl(res, 1);
    if (!less128(rem, trial)) {
      rem = sub128(rem, trial);
      res.lo |= 1;
    }
  }
  U128 m2 = shl(res, 1);
  if (nz(rem)) m2.lo |= 1;
  return round_pack(false, m2, ve / 2 - 1, Prec::Double, 0);
}

}  // namespace

u64 reciprocal_estimate(u64 b, u32 fpscr) { return div(0x3FF0000000000000ull, b, Prec::Single, fpscr); }

u64 fast_rsqrt(u64 b);  // Q4: the fast model's (below)

u64 rsqrt_estimate(u64 b, u32 fpscr) {
  if (fast_model(fpscr)) return fast_rsqrt(b);
  if (is_nan(b)) return nan_result(b, Prec::Double);
  if (is_zero(b)) return signed_inf((b & kSign) != 0);
  if (b & kSign) return kDefaultNaN;
  if (is_inf(b)) return 0;
  return div(0x3FF0000000000000ull, sqrt_rn(b), Prec::Double, fpscr);
}

// ---- quantization ----

namespace {
int scale_exp(u32 scale) { return (scale & 0x20) ? static_cast<int>(scale) - 64 : static_cast<int>(scale); }
}  // namespace

namespace ref {

u64 dequantize(u32 raw, u32 type, u32 scale) {
  long v;
  switch (type) {
    case 4: v = static_cast<long>(raw & 0xFF); break;
    case 5: v = static_cast<long>(raw & 0xFFFF); break;
    case 6: v = static_cast<long>(static_cast<signed char>(raw & 0xFF)); break;
    case 7: v = static_cast<long>(static_cast<short>(raw & 0xFFFF)); break;
    default: return single_to_double(raw);
  }
  if (v == 0) return 0;
  bool sign = v < 0;
  u64 m = static_cast<u64>(sign ? -v : v);
  // value = v * 2^-scale: exact (|v| < 2^16 and |scale| <= 32 stay in single range).
  return round_pack(sign, m, -scale_exp(scale), Prec::Single, 0);
}

u32 quantize(u64 value, u32 type, u32 scale) {
  if (type < 4) return double_to_single_ftz(value);
  long lo, hi;
  switch (type) {
    case 4: lo = 0; hi = 255; break;
    case 5: lo = 0; hi = 65535; break;
    case 6: lo = -128; hi = 127; break;
    default: lo = -32768; hi = 32767; break;
  }
  long r;
  if (is_nan(value)) {
    r = hi;  // NaN saturates to the positive end (Gekko manual, quantized stores)
  } else {
    // The hardware first converts to single (round to nearest), then scales by 2^S
    // in single precision, then truncates toward zero with saturation.
    u64 s = round_to_single(value, 0);
    Num n = unpack(s);
    if (n.kind == kZero) {
      r = 0;
    } else if (n.kind == kInf) {
      r = n.sign ? lo : hi;
    } else {
      int e = n.e + scale_exp(scale);
      // Single overflow after scaling: infinity, so saturation.
      u64 m = n.m;
      int top = 63 - clz64(m) + e;
      if (top > 127) {
        r = n.sign ? lo : hi;
      } else {
        u64 ip = e >= 0 ? (e > 10 ? ~0ull : m << e) : (e <= -64 ? 0 : m >> -e);
        if (!n.sign) r = ip > static_cast<u64>(hi) ? hi : static_cast<long>(ip);
        else r = ip > static_cast<u64>(-lo) ? lo : -static_cast<long>(ip);
      }
    }
  }
  return static_cast<u32>(r) & (type == 4 || type == 6 ? 0xFFu : 0xFFFFu);
}

}  // namespace ref

// ---- shortcuts through host double arithmetic (E3a) ----
//
// The exact integer code above costs tens of nanoseconds per operation; paired-
// single code (the movie decoder, the game's matrix math) runs millions of them.
// Each function below first tries the host's IEEE double arithmetic and keeps its
// result only where it is provably the result of the exact code, else it calls
// ref::. The conditions:
// - round to nearest (FPSCR[RN] = 0) and finite operands, so no NaN (whose host
//   payload is not deterministic) can arise;
// - double results: the host result is correctly rounded; it is kept when it is
//   normal or zero (a subnormal result is where NI flushes);
// - single results: the host double result r = RN53(x) of the exact value x is
//   rounded again to single. That equals RN24(x) unless r lies exactly on a
//   midpoint between two singles (a midpoint is a double, so rounding x to double
//   can reach it but never cross it); midpoints go to ref::, and so do results
//   near or outside the normal single range (NI flushing, overflow);
// - fused multiply-add: only when the product is exact in double (the operands'
//   significant bits add up to at most 53), so one rounding of p + b is the fused
//   result;
// - zero results carry IEEE's signs, as the exact code's do.
// The host must round to nearest (the default; nothing in the engine changes it)
// and must not flush subnormals; WebAssembly does both. test_fp compares every
// shortcut with ref:: on millions of operands, midpoints included.

namespace {

inline double as_double(u64 x) {
  double d;
  std::memcpy(&d, &x, 8);
  return d;
}
inline u64 as_bits(double d) {
  u64 x;
  std::memcpy(&x, &d, 8);
  return x;
}
inline bool finite(u64 x) { return (x & kExpMask) != kExpMask; }
inline u32 biased_exp(u64 x) { return static_cast<u32>((x >> 52) & 0x7FF); }
// 2^k as a double, for |k| <= 64.
inline double pow2(int k) { return as_double(static_cast<u64>(1023 + k) << 52); }

// The result of precision p for the host double result r, or false.
inline bool finish(double r, Prec p, u64* out) {
  const u64 b = as_bits(r);
  if ((b & ~kSign) == 0) {  // zero
    *out = b;
    return true;
  }
  const u32 e = biased_exp(b);
  if (p == Prec::Double) {
    if (e == 0 || e == 0x7FF) return false;  // subnormal (NI), infinity, NaN
    *out = b;
    return true;
  }
  if (e <= 1023 - 126 || e >= 1023 + 127) return false;  // |r| < 2^-125 or >= 2^127
  if ((b & 0x1FFFFFFFull) == 0x10000000ull) return false;  // on a midpoint between two singles
  *out = as_bits(static_cast<double>(static_cast<float>(r)));
  return true;
}

// Whether a * c is exact in double: both normal or zero, significant bits <= 53.
inline bool product_exact(u64 a, u64 c) {
  if ((a & ~kSign) == 0 || (c & ~kSign) == 0) return true;
  if (biased_exp(a) == 0 || biased_exp(c) == 0) return false;
  const int sa = 53 - std::countr_zero((a & kFracMask) | (1ull << 52));
  const int sc = 53 - std::countr_zero((c & kFracMask) | (1ull << 52));
  return sa + sc <= 53;
}

// Q4, F1: the fast model's result for the double r (fp.h).
inline u64 fast_finish(double r, Prec p) { return fast_result(r, p); }

}  // namespace

u64 fast_rsqrt(u64 b) {
  const double s = std::sqrt(as_double(b));
  return fast_finish(1.0 / s, Prec::Double);
}

u64 add(u64 a, u64 b, Prec p, u32 fpscr) {
  if (fast_model(fpscr)) return fast_finish(as_double(a) + as_double(b), p);
  u64 r;
  if (!(fpscr & 3) && finite(a) && finite(b) && finish(as_double(a) + as_double(b), p, &r)) return r;
  return ref::add(a, b, p, fpscr);
}

u64 sub(u64 a, u64 b, Prec p, u32 fpscr) {
  if (fast_model(fpscr)) return fast_finish(as_double(a) - as_double(b), p);
  u64 r;
  if (!(fpscr & 3) && finite(a) && finite(b) && finish(as_double(a) - as_double(b), p, &r)) return r;
  return ref::sub(a, b, p, fpscr);
}

u64 mul(u64 a, u64 c, Prec p, u32 fpscr) {
  if (fast_model(fpscr)) return fast_finish(as_double(a) * as_double(c), p);
  u64 r;
  if (!(fpscr & 3) && finite(a) && finite(c) && finish(as_double(a) * as_double(c), p, &r)) return r;
  return ref::mul(a, c, p, fpscr);
}

u64 div(u64 a, u64 b, Prec p, u32 fpscr) {
  if (fast_model(fpscr)) return fast_finish(as_double(a) / as_double(b), p);
  u64 r;
  if (!(fpscr & 3) && finite(a) && finite(b) && finish(as_double(a) / as_double(b), p, &r)) return r;
  return ref::div(a, b, p, fpscr);
}

u64 madd(u64 a, u64 c, u64 b, bool negate_b, bool negate_result, Prec p, u32 fpscr) {
  if (fast_model(fpscr)) {  // two roundings: the product, then the sum
    const double prod = as_double(a) * as_double(c);
    double r = negate_b ? prod - as_double(b) : prod + as_double(b);
    if (negate_result) r = -r;
    return fast_finish(r, p);
  }
  if (!(fpscr & 3) && finite(a) && finite(c) && finite(b) && product_exact(a, c)) {
    const double prod = as_double(a) * as_double(c);
    const u64 pb = as_bits(prod);
    // Exact: normal, or zero because an operand is zero (not an underflow).
    const bool zero_operand = (a & ~kSign) == 0 || (c & ~kSign) == 0;
    if (zero_operand || biased_exp(pb) != 0) {
      const double addend = negate_b ? -as_double(b) : as_double(b);
      u64 r;
      if (finish(prod + addend, p, &r)) return negate_result ? r ^ kSign : r;
    }
  }
  return ref::madd(a, c, b, negate_b, negate_result, p, fpscr);
}

u64 round_to_single(u64 x, u32 fpscr) {
  if (fast_model(fpscr)) return fast_finish(as_double(x), Prec::Single);
  u64 r;
  if (!(fpscr & 3) && finite(x) && finish(as_double(x), Prec::Single, &r)) return r;
  return ref::round_to_single(x, fpscr);
}

u64 dequantize(u32 raw, u32 type, u32 scale) {
  int v;
  switch (type) {
    case 4: v = static_cast<int>(raw & 0xFF); break;
    case 5: v = static_cast<int>(raw & 0xFFFF); break;
    case 6: v = static_cast<int>(static_cast<signed char>(raw & 0xFF)); break;
    case 7: v = static_cast<int>(static_cast<short>(raw & 0xFFFF)); break;
    default: return single_to_double(raw);
  }
  // v * 2^-scale is exact in double and in single (|v| < 2^16, |scale| <= 32).
  return as_bits(static_cast<double>(v) * pow2(-scale_exp(scale)));
}

u32 quantize(u64 value, u32 type, u32 scale) {
  if (type < 4) return double_to_single_ftz(value);
  // A value that is already a normal single (or zero) is its own rounding to
  // single; scaling by a power of two is exact; then truncate and saturate.
  const u32 e = biased_exp(value);
  const bool zero = (value & ~kSign) == 0;
  if (zero || (e >= 1023 - 126 && e <= 1023 + 127 && (value & 0x1FFFFFFFull) == 0)) {
    double lo, hi;
    switch (type) {
      case 4: lo = 0; hi = 255; break;
      case 5: lo = 0; hi = 65535; break;
      case 6: lo = -128; hi = 127; break;
      default: lo = -32768; hi = 32767; break;
    }
    double t = as_double(value) * pow2(scale_exp(scale));
    t = t < lo ? lo : t > hi ? hi : t;
    const long r = static_cast<long>(t);  // toward zero
    return static_cast<u32>(r) & (type == 4 || type == 6 ? 0xFFu : 0xFFFFu);
  }
  return ref::quantize(value, type, scale);
}

}  // namespace mvp::ppc::fp
