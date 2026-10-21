// Gekko floating-point semantics, computed exactly in integer arithmetic.
//
// Every result is the infinitely precise result rounded once to the target
// precision (24 or 53 bits) with the FPSCR rounding mode. The model is integer
// code (namespace ref), so native and WebAssembly builds agree bit for bit, NaN
// payloads included (WebAssembly arithmetic NaNs are nondeterministic;
// E0_SURVEY §3). For speed, the arithmetic first tries host double arithmetic and
// keeps it only where it provably equals the model: round to nearest, finite
// operands, no NaN, normal or exactly-zero results, never a single-precision
// midpoint (fp.cpp, "shortcuts"; test_fp compares the two).
//
// The model (each point is checked by tests/test_fp.cpp):
// - Values are raw IEEE double bits (u64). A single-precision result is the double
//   whose value is that single.
// - NaN operands propagate in the PowerPC order (frA, frB, frC), made quiet; an
//   invalid operation (inf-inf, 0*inf, 0/0, inf/inf) gives the default QNaN
//   0x7FF8000000000000. A NaN going to a single-precision result keeps the sign
//   and the top 23 fraction bits (truncated, like the hardware's conversion).
// - NI mode (FPSCR[NI], which the game sets in OSInit): a single-precision result
//   whose exact value is below the smallest normal single flushes to a signed
//   zero; a double result that rounds to a subnormal double flushes to zero.
//   Inputs are not flushed.
// - Single-precision instructions multiply by frC rounded to a 25-bit significand
//   (force25), as the Gekko multiplier does.
// - fmadd-family results are fused: one rounding of a*c+b.
//
// Q4, F1 (owner, 2026-10-16: the engine must be the same on every host, not
// bit-exact to the Gekko): the **fast model**, the default, replaces the
// arithmetic above whenever FPSCR[RN] = 0 and FPSCR[NI] = 1 (round to nearest
// with NI, the game's setting; anything else keeps the exact model):
// - a, b, c are used as the doubles they are; one IEEE double operation per
//   step, in the order written: a*c, then +-b (no fused multiply-add), then the
//   negation of the nm forms; no force25;
// - a single-precision result is that double rounded to single (round to
//   nearest), as a double; results below the normal range stay as IEEE gives
//   them (Q6: NI's flush to zero is not modeled; IEEE subnormals are the same
//   on every host);
// - a NaN result is the default NaN 0x7FF8000000000000 (operand payloads do not
//   propagate: hosts differ there). Q6: this stays per result, not only where
//   bits leave the registers: lfd/stfd, lfs/stfs and psq_l/psq_st move NaN bit
//   patterns exactly (copies through FP registers of integer data such as -1),
//   so a store cannot tell a loaded NaN, which it must keep, from a computed one,
//   whose bits differ between hosts; the check costs about 1 %, the flush 5 %;
// - reciprocal and reciprocal square root estimates: 1/b rounded to single, and
//   1/sqrt(b) in double.
// Every step is IEEE round to nearest without flush to zero, so the results are
// the same on every host (x86, ARM; WebAssembly, native without contraction).
// The WebAssembly emitter computes the same inline (translator/wasm_emit.cpp);
// the exact model stays for tests and as the fallback (g_model = Model::Exact).
#pragma once

#include <bit>
#include <cstdint>

namespace mvp::ppc::fp {

using u32 = std::uint32_t;
using u64 = std::uint64_t;

enum class Prec { Single, Double };

enum class Model : std::uint8_t { Exact, Fast };
extern Model g_model;  // the interpreter's and C++-emitted code's (default: Fast)
// Whether an instruction with this FPSCR uses the fast model.
inline bool fast_model(u32 fpscr) { return g_model == Model::Fast && (fpscr & 7) == 4; }

inline constexpr u64 kSign = 0x8000000000000000ull;
inline constexpr u64 kExpMask = 0x7FF0000000000000ull;
inline constexpr u64 kFracMask = 0x000FFFFFFFFFFFFFull;
inline constexpr u64 kQuietBit = 0x0008000000000000ull;
inline constexpr u64 kDefaultNaN = 0x7FF8000000000000ull;

// Q4, F1: the fast model's last step for the double r one IEEE operation gave
// (fp.cpp's arithmetic, and host code that runs a game function natively,
// runtime/hle_game.cpp): a NaN made the default NaN, rounded to single for
// Single (Q6: no flush below the normal range).
inline double to_f64(u64 x) { return std::bit_cast<double>(x); }
inline u64 to_bits(double d) { return std::bit_cast<u64>(d); }
inline u64 fast_result(double r, Prec p) {
  if (r != r) return kDefaultNaN;
  if (p == Prec::Single) r = static_cast<double>(static_cast<float>(r));
  return to_bits(r);
}

inline bool is_nan(u64 x) { return (x & ~kSign) > kExpMask; }
inline bool is_snan(u64 x) { return is_nan(x) && !(x & kQuietBit); }
inline bool is_inf(u64 x) { return (x & ~kSign) == kExpMask; }
inline bool is_zero(u64 x) { return (x & ~kSign) == 0; }

// Load/store conversions between single format in memory and the FPR's double
// format (PowerPC Programming Environments Manual, "Floating-Point Load" and
// "Floating-Point Store" conversion algorithms). Exact bit manipulation.
u64 single_to_double(u32 s);  // lfs, psq_l type 0
// The same, with a normal single (the common case) converted inline.
inline u64 single_to_double_inline(u32 s) {
  const u32 e = (s >> 23) & 0xFF;
  if (e - 1 < 254)  // the exponent rebiased by 1023 - 127
    return (static_cast<u64>(s & 0x80000000u) << 32) | ((static_cast<u64>(s & 0x7FFFFFFFu) << 29) + (896ull << 52));
  return single_to_double(s);
}
u32 double_to_single(u64 d);  // stfs (no rounding: truncates; denormalizes small values)
// psq_st type 0: below the single normal range -> signed zero (V4: inline, the native
// skinning code's stores call it for every word).
inline u32 double_to_single_ftz(u64 d) {
  const u32 exp = static_cast<u32>((d >> 52) & 0x7FF);
  if (exp > 896 || (d & ~kSign) == 0)
    return static_cast<u32>((d >> 32) & 0xC0000000u) | static_cast<u32>((d >> 29) & 0x3FFFFFFF);
  return static_cast<u32>((d >> 32) & 0x80000000u);
}

// The Gekko's rounding of a multiplier operand to a 25-bit significand.
u64 force25(u64 x);
// force25 for an instruction with this FPSCR: the fast model does not apply it.
inline u64 force25(u64 x, u32 fpscr) { return fast_model(fpscr) ? x : force25(x); }

// Arithmetic. `fpscr` supplies the rounding mode and NI.
u64 add(u64 a, u64 b, Prec p, u32 fpscr);
u64 sub(u64 a, u64 b, Prec p, u32 fpscr);
u64 mul(u64 a, u64 c, Prec p, u32 fpscr);
u64 div(u64 a, u64 b, Prec p, u32 fpscr);
// a*c + b, with b negated for the msub forms and the result negated for the nm
// forms (a NaN result keeps its sign).
u64 madd(u64 a, u64 c, u64 b, bool negate_b, bool negate_result, Prec p, u32 fpscr);
// frsp: round a double to single precision.
u64 round_to_single(u64 x, u32 fpscr);

// fctiw / fctiwz: the FPR result (0xFFF80000 in the high word, the integer in the
// low word; a negative-zero source sets bit 32). `toward_zero` selects fctiwz.
u64 to_int32(u64 x, bool toward_zero, u32 fpscr);

// Estimates. The hardware uses lookup tables that are not reproduced yet: these
// return the correctly rounded reciprocal (single) and reciprocal square root
// (double). Deterministic, but not bit-exact with hardware (E0B_LOG, item 4).
u64 reciprocal_estimate(u64 b, u32 fpscr);
u64 rsqrt_estimate(u64 b, u32 fpscr);

// fsel: c if a >= 0 (including -0), else b; a NaN selects b.
inline u64 select(u64 a, u64 c, u64 b) {
  if (is_nan(a)) return b;
  if ((a & kSign) && !is_zero(a)) return b;
  return c;
}

// Paired-single quantization (GQR). Types: 0 single float, 4 u8, 5 u16, 6 s8,
// 7 s16 (1-3 are reserved and behave like 0). `scale` is the 6-bit field.
inline u32 quant_size(u32 type) { return type == 4 || type == 6 ? 1 : type == 5 || type == 7 ? 2 : 4; }
u64 dequantize(u32 raw, u32 type, u32 scale);
u32 quantize(u64 value, u32 type, u32 scale);

// The exact integer implementations (the model above). The functions above take
// a shortcut through host double arithmetic where its result is provably the
// same (fp.cpp, "shortcuts"), and call these otherwise. Tests compare the two.
namespace ref {
u64 add(u64 a, u64 b, Prec p, u32 fpscr);
u64 sub(u64 a, u64 b, Prec p, u32 fpscr);
u64 mul(u64 a, u64 c, Prec p, u32 fpscr);
u64 div(u64 a, u64 b, Prec p, u32 fpscr);
u64 madd(u64 a, u64 c, u64 b, bool negate_b, bool negate_result, Prec p, u32 fpscr);
u64 round_to_single(u64 x, u32 fpscr);
u64 dequantize(u32 raw, u32 type, u32 scale);
u32 quantize(u64 value, u32 type, u32 scale);
}  // namespace ref

}  // namespace mvp::ppc::fp
