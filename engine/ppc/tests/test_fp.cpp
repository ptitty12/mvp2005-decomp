// The integer floating-point core (fp.cpp) against independent references:
// host IEEE double/float arithmetic (round to nearest), std::fma, and a
// round-to-odd reference for fused single-precision results. Plus fixed vectors
// for the conversions and the Gekko-specific pieces.
#include <doctest/doctest.h>

#include <cmath>
#include <cstring>
#include <initializer_list>

#include "ppc/fp.h"

using namespace mvp::ppc;
using namespace mvp::ppc::fp;

namespace {

// These tests are about the exact Gekko model (Q4, F1: the engine's default is the
// fast model, which the last test case here checks).
const bool g_exact = (g_model = Model::Exact, true);

u64 bits(double d) {
  u64 b;
  std::memcpy(&b, &d, 8);
  return b;
}
double dbl(u64 b) {
  double d;
  std::memcpy(&d, &b, 8);
  return d;
}
u32 fbits(float f) {
  u32 b;
  std::memcpy(&b, &f, 4);
  return b;
}
float flt(u32 b) {
  float f;
  std::memcpy(&f, &b, 4);
  return f;
}

// Deterministic generator (xorshift64*).
struct Rng {
  u64 s;
  explicit Rng(u64 seed) : s(seed) {}
  u64 next() {
    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    return s * 0x2545F4914F6CDD1Dull;
  }
  // A finite double with exponent in [emin, emax] (unbiased), random sign and fraction.
  double finite(int emin, int emax) {
    u64 r = next();
    int e = emin + static_cast<int>(r % static_cast<u64>(emax - emin + 1));
    u64 frac = next() & kFracMask;
    // Sometimes few significant bits, to hit exact cases and ties.
    if ((r >> 40) % 4 == 0) frac &= ~((1ull << (r >> 48) % 52) - 1);
    u64 b = (static_cast<u64>(e + 1023) << 52) | frac | ((r >> 63) << 63);
    return dbl(b);
  }
  float single(int emin, int emax) {
    u64 r = next();
    int e = emin + static_cast<int>(r % static_cast<u64>(emax - emin + 1));
    u32 frac = static_cast<u32>(next()) & 0x7FFFFF;
    if ((r >> 40) % 4 == 0) frac &= ~((1u << (r >> 48) % 23) - 1);
    return flt((static_cast<u32>(e + 127) << 23) | frac | static_cast<u32>((r >> 63) << 31));
  }
};

// Reference for a single-precision fused a*c+b with single inputs: the product is
// exact in double; TwoSum gives the exact error of the double sum; making the sum
// round-to-odd from that error, then rounding to float, rounds once.
float ref_fmadds(float a, float c, float b) {
  double p = static_cast<double>(a) * static_cast<double>(c);  // exact (48 bits)
  double s = p + b;
  double bb = s - p;
  double e = (p - (s - bb)) + (b - bb);  // exact error of s
  if (e != 0) {
    u64 sb = bits(s);
    if (!(sb & 1)) {
      // Move one ulp toward the exact value, giving an odd last bit.
      bool toward_larger_magnitude = (e > 0) == (s > 0);
      sb += toward_larger_magnitude ? 1 : static_cast<u64>(-1);
      s = dbl(sb);
    }
  }
  return static_cast<float>(s);
}

}  // namespace

TEST_CASE("double add/sub/mul/div match host IEEE arithmetic") {
  Rng r(0x1234);
  int checked = 0;
  for (int k = 0; k < 200000; ++k) {
    int range = static_cast<int>(r.next() % 3);
    double a = range == 0 ? r.finite(-20, 20) : range == 1 ? r.finite(-1074 + 52, 1023) : r.finite(-1022, -900);
    double b = range == 0 ? r.finite(-20, 20) : range == 1 ? r.finite(-1074 + 52, 1023) : r.finite(-1022, -900);
    if (k % 7 == 0) b = -a * (1.0 + std::ldexp(static_cast<double>(r.next() % 1000), -50));  // cancellation
    struct Case {
      u64 got;
      double want;
    } cases[] = {
        {add(bits(a), bits(b), Prec::Double, 0), a + b},
        {sub(bits(a), bits(b), Prec::Double, 0), a - b},
        {mul(bits(a), bits(b), Prec::Double, 0), a * b},
        {div(bits(a), bits(b), Prec::Double, 0), a / b},
    };
    for (const auto& c : cases) {
      if (std::isnan(c.want)) continue;
      ++checked;
      if (c.got != bits(c.want)) {
        FAIL_CHECK("a=", a, " b=", b, " got ", dbl(c.got), " want ", c.want);
        return;
      }
    }
  }
  CHECK(checked > 700000);
}

TEST_CASE("single add/sub/mul/div of single inputs match host float arithmetic") {
  Rng r(0x777);
  for (int k = 0; k < 200000; ++k) {
    bool tiny = r.next() % 4 == 0;
    float a = tiny ? r.single(-126, -100) : r.single(-30, 30);
    float b = tiny ? r.single(-126, -100) : r.single(-30, 30);
    u64 A = bits(static_cast<double>(a)), B = bits(static_cast<double>(b));
    struct Case {
      u64 got;
      float want;
    } cases[] = {
        {add(A, B, Prec::Single, 0), a + b},
        {sub(A, B, Prec::Single, 0), a - b},
        {mul(A, B, Prec::Single, 0), a * b},
        {div(A, B, Prec::Single, 0), a / b},
    };
    for (const auto& c : cases) {
      if (std::isnan(c.want)) continue;
      if (c.got != bits(static_cast<double>(c.want))) {
        FAIL_CHECK("a=", a, " b=", b, " got ", dbl(c.got), " want ", c.want);
        return;
      }
    }
  }
}

TEST_CASE("double madd matches std::fma") {
  Rng r(0x99);
  for (int k = 0; k < 200000; ++k) {
    double a = r.finite(-60, 60), c = r.finite(-60, 60);
    double b = r.next() % 3 ? r.finite(-120, 120) : -(a * c);  // include near-total cancellation
    double want = std::fma(a, c, b);
    if (std::isnan(want)) continue;
    u64 got = madd(bits(a), bits(c), bits(b), false, false, Prec::Double, 0);
    if (got != bits(want)) {
      FAIL_CHECK("a=", a, " c=", c, " b=", b, " got ", dbl(got), " want ", want);
      return;
    }
    // msub / nmadd / nmsub are sign variants.
    CHECK(madd(bits(a), bits(c), bits(b), true, false, Prec::Double, 0) == bits(std::fma(a, c, -b)));
    u64 nm = madd(bits(a), bits(c), bits(b), false, true, Prec::Double, 0);
    CHECK(nm == (bits(want) ^ kSign));
  }
}

TEST_CASE("single madd of single inputs matches a round-to-odd reference") {
  Rng r(0x4242);
  for (int k = 0; k < 200000; ++k) {
    float a = r.single(-20, 20), c = r.single(-20, 20);
    float b = r.next() % 3 ? r.single(-40, 40) : -(a * c);
    float want = ref_fmadds(a, c, b);
    u64 got = madd(bits(a), bits(c), bits(b), false, false, Prec::Single, 0);
    if (got != bits(static_cast<double>(want))) {
      FAIL_CHECK("a=", a, " c=", c, " b=", b, " got ", dbl(got), " want ", want);
      return;
    }
  }
}

TEST_CASE("round_to_single matches host double-to-float conversion") {
  Rng r(0x5150);
  for (int k = 0; k < 200000; ++k) {
    double d = r.next() % 5 ? r.finite(-160, 140) : r.finite(-1022, 1023);
    float want = static_cast<float>(d);
    CHECK_MESSAGE(round_to_single(bits(d), 0) == bits(static_cast<double>(want)), d);
  }
}

TEST_CASE("single <-> double load/store conversions") {
  Rng r(0xABCD);
  for (int k = 0; k < 100000; ++k) {
    u32 s = static_cast<u32>(r.next());
    u64 d = single_to_double(s);
    // Storing what was loaded gives the same word back (NaN payloads included).
    CHECK(double_to_single(d) == s);
    float f = flt(s);
    if (!std::isnan(f)) CHECK(d == bits(static_cast<double>(f)));
  }
  CHECK(single_to_double(0x00000001) == bits(0x1p-149));
  CHECK(single_to_double(0x80000000) == kSign);
  CHECK(single_to_double(0x7F800000) == 0x7FF0000000000000ull);
  CHECK(single_to_double(0x7FC00001) == 0x7FF8000020000000ull);
  // Stores below the normal single range denormalize (truncating).
  CHECK(double_to_single(bits(0x1p-130)) == 0x00080000);
  CHECK(double_to_single(bits(0x1p-130 * 1.75)) == 0x000E0000);
  CHECK(double_to_single_ftz(bits(0x1p-130)) == 0);
  CHECK(double_to_single_ftz(bits(-0x1p-130)) == 0x80000000);
  CHECK(double_to_single_ftz(bits(1.0)) == 0x3F800000);
}

TEST_CASE("force25 rounds the significand to 25 bits, ties away") {
  CHECK(force25(bits(1.0)) == bits(1.0));
  CHECK(force25(bits(1.0 + 0x1p-25)) == bits(1.0 + 0x1p-24));  // half: rounds up
  CHECK(force25(bits(1.0 + 0x1p-26)) == bits(1.0));            // below half
  CHECK(force25(bits(1.0 + 0x1p-24 + 0x1p-25)) == bits(1.0 + 0x1p-23));
  CHECK(force25(bits(-(1.0 + 0x1p-25))) == bits(-(1.0 + 0x1p-24)));
  // Singles are unchanged.
  Rng r(3);
  for (int k = 0; k < 10000; ++k) {
    double d = static_cast<double>(r.single(-126, 127));
    CHECK(force25(bits(d)) == bits(d));
  }
}

TEST_CASE("zero signs and rounding modes") {
  const u64 pz = 0, nz = kSign, one = bits(1.0), mone = bits(-1.0);
  CHECK(add(pz, nz, Prec::Double, 0) == pz);
  CHECK(add(nz, nz, Prec::Double, 0) == nz);
  CHECK(add(one, mone, Prec::Double, 0) == pz);
  CHECK(add(one, mone, Prec::Double, 3) == nz);  // round toward -inf
  CHECK(mul(mone, pz, Prec::Double, 0) == nz);
  CHECK(madd(mone, pz, pz, false, false, Prec::Double, 0) == pz);  // -0 + +0
  CHECK(madd(mone, pz, nz, false, false, Prec::Double, 0) == nz);  // -0 + -0
  // Directed rounding of 1 + 2^-60.
  u64 x = bits(1.0), y = bits(0x1p-60);
  CHECK(add(x, y, Prec::Double, 0) == bits(1.0));
  CHECK(add(x, y, Prec::Double, 1) == bits(1.0));
  CHECK(add(x, y, Prec::Double, 2) == bits(1.0 + 0x1p-52));
  CHECK(add(x, y, Prec::Double, 3) == bits(1.0));
  CHECK(sub(x, y, Prec::Double, 3) == bits(1.0 - 0x1p-53));
  // Overflow: inf for nearest, max finite toward zero.
  CHECK(mul(bits(1e300), bits(1e300), Prec::Double, 0) == 0x7FF0000000000000ull);
  CHECK(mul(bits(1e300), bits(1e300), Prec::Double, 1) == 0x7FEFFFFFFFFFFFFFull);
  CHECK(mul(bits(1e30), bits(1e30), Prec::Single, 0) == 0x7FF0000000000000ull);
  CHECK(mul(bits(1e30), bits(1e30), Prec::Single, 1) == bits(static_cast<double>(3.40282347e38f)));
}

TEST_CASE("fctiw and fctiwz conversions") {
  CHECK(to_int32(bits(0.0), true, 0) == 0xFFF8000000000000ull);
  CHECK(to_int32(bits(-0.0), true, 0) == 0xFFF8000100000000ull);
  CHECK(to_int32(bits(2147483647.9), true, 0) == 0xFFF800007FFFFFFFull);
  CHECK(to_int32(bits(-2147483648.9), true, 0) == 0xFFF8000080000000ull);
  CHECK(to_int32(bits(-1.5), false, 0) == 0xFFF80000FFFFFFFEull);  // nearest even: -2
  CHECK(to_int32(bits(-2.5), false, 0) == 0xFFF80000FFFFFFFEull);  // nearest even: -2
  CHECK(to_int32(bits(0.5), false, 0) == 0xFFF8000000000000ull);
  CHECK(to_int32(bits(1.5), false, 2) == 0xFFF8000000000002ull);   // toward +inf
  CHECK(to_int32(bits(-1.5), false, 3) == 0xFFF80000FFFFFFFEull);  // toward -inf
  CHECK(to_int32(bits(1e-300), false, 2) == 0xFFF8000000000001ull);
  CHECK(to_int32(0x7FF0000000000000ull, true, 0) == 0xFFF800007FFFFFFFull);
  Rng r(77);
  for (int k = 0; k < 100000; ++k) {
    double d = r.finite(-10, 30);
    long long t = static_cast<long long>(std::trunc(d));
    if (t > 0x7FFFFFFF || t < -0x80000000LL) continue;
    u64 want = 0xFFF8000000000000ull | static_cast<u32>(static_cast<int>(t));
    if (t == 0 && std::signbit(d)) want |= 0x100000000ull;
    CHECK(to_int32(bits(d), true, 0) == want);
  }
}

TEST_CASE("estimates are deterministic and correctly rounded") {
  CHECK(rsqrt_estimate(bits(4.0), 0) == bits(0.5));
  CHECK(rsqrt_estimate(bits(0.0), 0) == 0x7FF0000000000000ull);
  CHECK(rsqrt_estimate(bits(-0.0), 0) == 0xFFF0000000000000ull);
  CHECK(rsqrt_estimate(bits(-4.0), 0) == kDefaultNaN);
  CHECK(rsqrt_estimate(0x7FF0000000000000ull, 0) == 0);
  Rng r(5);
  for (int k = 0; k < 100000; ++k) {
    double d = std::fabs(r.finite(-1000, 1000));
    CHECK(rsqrt_estimate(bits(d), 0) == bits(1.0 / std::sqrt(d)));
  }
  CHECK(reciprocal_estimate(bits(3.0), 0) == bits(static_cast<double>(1.0f / 3.0f)));
}

TEST_CASE("quantization") {
  // dequantize: integer * 2^-scale; scale is a 6-bit two's complement field.
  CHECK(dequantize(0xFF, 4, 0) == bits(255.0));
  CHECK(dequantize(0xFF, 6, 0) == bits(-1.0));
  CHECK(dequantize(0x8000, 7, 1) == bits(-16384.0));
  CHECK(dequantize(0x8000, 5, 0x3F) == bits(65536.0));  // scale -1: times 2
  CHECK(dequantize(0x3F800000, 0, 5) == bits(1.0));     // float type ignores the scale
  // quantize: round to single, scale, truncate toward zero, saturate.
  CHECK(quantize(bits(1.99999999999), 7, 0) == 2);  // rounds to the single 2.0 first
  CHECK(quantize(bits(1.999), 7, 0) == 1);
  CHECK(quantize(bits(-1.999), 7, 0) == 0xFFFF);  // -1
  CHECK(quantize(bits(40000.0), 7, 0) == 0x7FFF);
  CHECK(quantize(bits(-40000.0), 7, 0) == 0x8000);
  CHECK(quantize(bits(70000.0), 5, 0) == 0xFFFF);
  CHECK(quantize(bits(-1.0), 5, 0) == 0);
  CHECK(quantize(bits(0.75), 4, 2) == 3);  // 0.75 * 4
  CHECK(quantize(kDefaultNaN, 6, 0) == 0x7F);
  CHECK(quantize(0x7FF0000000000000ull, 4, 0) == 0xFF);
  CHECK(quantize(0xFFF0000000000000ull, 6, 0) == 0x80);
  CHECK(quantize(bits(1e38), 7, 10) == 0x7FFF);  // scaled past the single range
  CHECK(quantize(bits(1.0), 0, 0) == 0x3F800000);
}

TEST_CASE("NI flushing") {
  const u32 ni = 4;
  // Single result below 2^-126 (exact value) flushes, sign kept.
  CHECK(mul(bits(-0x1p-70), bits(0x1p-60), Prec::Single, ni) == kSign);
  CHECK(mul(bits(-0x1p-70), bits(0x1p-60), Prec::Single, 0) == bits(-0x1p-130));
  // A value just below 2^-126 that would round up to it still flushes.
  CHECK(round_to_single(bits(0x1p-126 * (1 - 0x1p-30)), ni) == 0);
  CHECK(round_to_single(bits(0x1p-126 * (1 - 0x1p-30)), 0) == bits(0x1p-126));
  // Double: flushes only results that round to a denormal double.
  CHECK(mul(bits(0x1p-600), bits(0x1p-450), Prec::Double, ni) == 0);
  CHECK(mul(bits(0x1p-600), bits(0x1p-422), Prec::Double, ni) == bits(0x1p-1022));
  // Inputs are not flushed.
  CHECK(add(bits(0x1p-1060), bits(1.0), Prec::Double, ni) == bits(1.0));
  CHECK(add(bits(0x1p-1060), bits(0x1p-1060), Prec::Double, 0) == bits(0x1p-1059));
}

// ---- the shortcuts through host double arithmetic (fp.cpp) against the exact code ----

namespace {

// Operands from several families: single values (as the paired-single unit holds
// them), doubles over the whole exponent range, subnormals, small integers and
// dyadic fractions (pixel arithmetic), values near the single range limits, and
// specials.
u64 operand(Rng& r) {
  const u64 k = r.next() % 16;
  switch (k) {
    case 0: case 1: case 2: case 3:
      return bits(static_cast<double>(r.single(-30, 30)));
    case 4: return bits(static_cast<double>(r.single(-140, 127)));  // single subnormal range included
    case 5: case 6: return bits(r.finite(-60, 60));
    case 7: return bits(r.finite(-1074 + 52, 1023));
    case 8: return r.next() & kFracMask;  // double subnormal (or zero)
    case 9: case 10: {  // small integers and eighths / 128ths
      const double v = static_cast<double>(static_cast<std::int64_t>(r.next() % 1024) - 512);
      return bits(v / static_cast<double>(1u << (r.next() % 8)));
    }
    case 11: return bits(r.finite(-127, -124));  // around the smallest normal single
    case 12: return bits(r.finite(125, 129));    // around the largest single
    case 13: {
      const u64 specials[] = {0, kSign, kExpMask, kExpMask | kSign, kDefaultNaN, 0x7FF4000000000001ull,
                              0x3FF0000000000000ull, 0x0010000000000000ull, 0x7FEFFFFFFFFFFFFFull,
                              bits(0x1p-126), bits(0x1p128), bits(0x1.fffffep127)};
      return specials[r.next() % (sizeof specials / sizeof specials[0])];
    }
    default: return bits(r.finite(-10, 10));
  }
}

// A pair whose double sum (or difference) lands on, or next to, a midpoint between
// two singles: the case where rounding twice could differ from rounding once.
void midpoint_pair(Rng& r, u64* a, u64* b) {
  const float s = r.single(-20, 20);
  const int e = std::ilogb(s);  // the single's ulp is 2^(e - 23)
  const double half = std::ldexp(1.0, e - 24);
  const double tweak[] = {0.0, std::ldexp(1.0, e - 60), -std::ldexp(1.0, e - 60), std::ldexp(1.0, e - 100),
                          -std::ldexp(1.0, e - 79)};
  *a = bits(static_cast<double>(s));
  *b = bits(half + tweak[r.next() % 5]);
}

}  // namespace

TEST_CASE("host-arithmetic shortcuts give exactly the results of the exact code") {
  Rng r(0xE3A0E3A0);
  const u32 fpscrs[] = {0, 4, 1, 2, 3, 4 | 3};
  const Prec kPrecs[] = {Prec::Single, Prec::Double};
  long long checked = 0;
  for (int n = 0; n < 600000; ++n) {
    u64 a = operand(r), b = operand(r), c = operand(r);
    if (n % 8 == 0) midpoint_pair(r, &a, &b);
    const u32 fs = fpscrs[n % 6 < 4 ? (n % 2 ? 0 : 1) : static_cast<std::size_t>(n % 6)];
    for (Prec p : kPrecs) {
      if (add(a, b, p, fs) != ref::add(a, b, p, fs)) FAIL_CHECK("add ", a, " ", b, " fpscr ", fs);
      if (sub(a, b, p, fs) != ref::sub(a, b, p, fs)) FAIL_CHECK("sub ", a, " ", b, " fpscr ", fs);
      const u64 c25 = p == Prec::Single ? force25(c) : c;
      if (mul(a, c25, p, fs) != ref::mul(a, c25, p, fs)) FAIL_CHECK("mul ", a, " ", c25, " fpscr ", fs);
      if (div(a, b, p, fs) != ref::div(a, b, p, fs)) FAIL_CHECK("div ", a, " ", b, " fpscr ", fs);
      for (int k = 0; k < 4; ++k)
        if (madd(a, c25, b, k & 1, k & 2, p, fs) != ref::madd(a, c25, b, k & 1, k & 2, p, fs))
          FAIL_CHECK("madd ", a, " ", c25, " ", b, " forms ", k, " fpscr ", fs);
      checked += 8;
    }
    if (round_to_single(a, fs) != ref::round_to_single(a, fs)) FAIL_CHECK("frsp ", a, " fpscr ", fs);
    for (u32 type = 4; type < 8; ++type) {
      const u32 scale = static_cast<u32>(r.next() % 64);
      if (quantize(a, type, scale) != ref::quantize(a, type, scale)) FAIL_CHECK("quantize ", a, " ", type, " ", scale);
    }
    checked += 5;
  }
  // Every quantized load: all raw values of the integer types at every scale.
  for (u32 type = 4; type < 8; ++type)
    for (u32 scale = 0; scale < 64; ++scale)
      for (u32 raw = 0; raw < (type == 4 || type == 6 ? 0x100u : 0x10000u); ++raw) {
        if (dequantize(raw, type, scale) != ref::dequantize(raw, type, scale)) {
          FAIL_CHECK("dequantize ", raw, " ", type, " ", scale);
        }
        ++checked;
      }
  MESSAGE("shortcut comparisons: ", checked);
}

// Q4, Q6 (F1): the fast model's rules, the engine's default with FPSCR[RN] = 0 and
// NI = 1: one IEEE double operation per step in a fixed order, single results
// rounded to f32, NaN results the default NaN, subnormal results kept (no flush).
TEST_CASE("the fast model: host IEEE steps, singles rounded, NaNs canonical, no flush") {
  g_model = Model::Fast;
  constexpr u32 kNI = 4;  // round to nearest, NI set
  std::uint64_t s = 0x9E3779B97F4A7C15ull;
  auto next = [&] {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return s;
  };
  auto pick = [&]() -> u64 {
    const u64 r = next();
    switch (r % 10) {
      case 0: return r & kSign;                                        // +-0
      case 1: return (r & kSign) | kExpMask;                           // +-inf
      case 2: return (r & kSign) | kExpMask | ((r >> 12) & kFracMask) | 1;  // NaN (any payload)
      case 3: return (r & (kSign | kFracMask));                        // subnormal double
      case 4: return bits(static_cast<double>(flt(static_cast<u32>(r >> 9) & 0x807FFFFFu)));  // subnormal single
      case 5: return bits(static_cast<double>(flt((static_cast<u32>(r >> 7) & 0x80FFFFFFu) | 0x00800000u)));  // tiny single
      case 6: return r;                                                // any double
      default: return bits(static_cast<double>(flt(static_cast<u32>(r >> 5) & 0xBFFFFFFFu)));  // a single, |x| < 2
    }
  };
  auto expect = [](double r, bool single) -> u64 {
    if (r != r) return kDefaultNaN;
    if (single) r = static_cast<double>(static_cast<float>(r));
    return bits(r);
  };
  int failures = 0;
  auto check = [&](u64 got, u64 want, const char* what) {
    if (got != want && ++failures <= 5) MESSAGE(what, ": got ", got, " want ", want);
  };
  for (int k = 0; k < 200000; ++k) {
    const u64 a = pick(), b = pick(), c = pick();
    const double da = dbl(a), db = dbl(b), dc = dbl(c);
    for (bool single : {false, true}) {
      const Prec p = single ? Prec::Single : Prec::Double;
      check(add(a, b, p, kNI), expect(da + db, single), "add");
      check(sub(a, b, p, kNI), expect(da - db, single), "sub");
      check(mul(a, c, p, kNI), expect(da * dc, single), "mul");
      check(div(a, b, p, kNI), expect(da / db, single), "div");
      const double prod = da * dc;
      check(madd(a, c, b, false, false, p, kNI), expect(prod + db, single), "madd");
      check(madd(a, c, b, true, false, p, kNI), expect(prod - db, single), "msub");
      check(madd(a, c, b, false, true, p, kNI), expect(-(prod + db), single), "nmadd");
      check(madd(a, c, b, true, true, p, kNI), expect(-(prod - db), single), "nmsub");
    }
    check(force25(c, kNI), c, "force25");
    check(round_to_single(a, kNI), expect(da, true), "frsp");
    check(reciprocal_estimate(b, kNI), expect(1.0 / db, true), "fres");
    check(rsqrt_estimate(b, kNI), expect(1.0 / std::sqrt(db), false), "frsqrte");
  }
  CHECK(failures == 0);
  // No flush: a single product just below 2^-126 is the subnormal IEEE gives.
  CHECK(mul(bits(0x1p-126), bits(0.5), Prec::Single, kNI) == bits(0x1p-127));
  CHECK(add(bits(0x1p-1022), bits(-0x1p-1023), Prec::Double, kNI) == bits(0x1p-1023));
  // Any other FPSCR keeps the exact model (NI clear: no flush either way; round toward zero).
  CHECK(mul(bits(0x1p-126), bits(0.5), Prec::Single, kNI | 1) == 0);  // exact model, NI: flushed
  g_model = Model::Exact;
}
