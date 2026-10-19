// Save-state files (runtime/state_file.h): the byte stream every part of the
// machine writes its fields into and reads them back from, in one fixed order.
//
// A part lists its fields once, in a template over the archive:
//
//   template <class Ar> void Part::state_io(Ar& a) { a(x_); a(table_); ... }
//
// StateWriter appends each field, StateReader fills it from the stream, so the
// two orders cannot drift apart. The stream is portable between the native and
// the wasm32 engines: little-endian, fixed widths, no pointers. 8-, 16- and
// 32-bit integers keep their width; every other integer (std::size_t, long,
// 64-bit) is written as 64 bits.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace mvp {

// A stream that ends early, or holds a size no engine would write.
struct StateError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

namespace state_detail {

template <class T>
inline constexpr bool kNarrow = std::is_same_v<T, bool> || std::is_same_v<T, char> || std::is_same_v<T, std::int8_t> ||
                                std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::int16_t> ||
                                std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::int32_t> ||
                                std::is_same_v<T, std::uint32_t>;

template <class T>
struct Scalar {
  static constexpr bool value = std::is_arithmetic_v<T> || std::is_enum_v<T>;
};

template <class T, bool = std::is_enum_v<T>>
struct BaseOf {
  using type = T;
};
template <class T>
struct BaseOf<T, true> {
  using type = std::underlying_type_t<T>;
};
template <class T>
using Base = typename BaseOf<T>::type;

// The width a scalar takes in the stream.
template <class T>
constexpr std::size_t width() {
  using B = Base<T>;
  if constexpr (std::is_floating_point_v<B>) return sizeof(B);
  else if constexpr (kNarrow<B>) return sizeof(B);
  else return 8;
}

}  // namespace state_detail

class StateWriter {
 public:
  static constexpr bool kReading = false;

  std::vector<std::uint8_t>& out() { return out_; }

  template <class T>
  void operator()(const T& v) {
    if constexpr (state_detail::Scalar<T>::value) {
      put_scalar(v);
    } else if constexpr (requires { v.first; v.second; }) {
      (*this)(v.first);
      (*this)(v.second);
    } else if constexpr (requires { v.size(); v.data(); typename T::value_type; }) {
      using E = typename T::value_type;
      if constexpr (requires(T& x) { x.resize(std::size_t{}); }) (*this)(static_cast<std::uint64_t>(v.size()));
      if constexpr (sizeof(E) == 1 && state_detail::Scalar<E>::value) bytes(v.data(), v.size());
      else
        for (const E& e : v) (*this)(e);
    } else {
      static_assert(sizeof(T) == 0, "StateWriter: list this type's fields");
    }
  }
  template <class T, std::size_t N>
  void operator()(const T (&v)[N]) {
    for (const T& e : v) (*this)(e);
  }

  void bytes(const void* p, std::size_t n) {
    const auto* b = static_cast<const std::uint8_t*>(p);
    out_.insert(out_.end(), b, b + n);
  }

  // A vector of structs: the count, then `each` per element.
  template <class T, class F>
  void vec(const std::vector<T>& v, F each) {
    (*this)(static_cast<std::uint64_t>(v.size()));
    for (const T& e : v) each(const_cast<T&>(e));
  }

 private:
  template <class T>
  void put_scalar(T v) {
    using B = state_detail::Base<T>;
    constexpr std::size_t w = state_detail::width<T>();
    std::uint8_t b[8] = {};
    if constexpr (std::is_floating_point_v<B>) {
      std::memcpy(b, &v, w);  // both engines are little-endian IEEE
    } else {
      const auto u = static_cast<std::uint64_t>(static_cast<B>(v));
      for (std::size_t k = 0; k < w; ++k) b[k] = static_cast<std::uint8_t>(u >> (8 * k));
    }
    out_.insert(out_.end(), b, b + w);
  }

  std::vector<std::uint8_t> out_;
};

class StateReader {
 public:
  static constexpr bool kReading = true;
  // No vector in a state is longer than this (ARAM, 16 MiB, is the longest; MEM1 is read with bytes()).
  static constexpr std::uint64_t kMaxElements = 64ull << 20;

  explicit StateReader(std::span<const std::uint8_t> in) : in_(in) {}
  std::size_t left() const { return in_.size() - at_; }
  std::size_t offset() const { return at_; }

  template <class T>
  void operator()(T& v) {
    if constexpr (state_detail::Scalar<T>::value) {
      v = get_scalar<T>();
    } else if constexpr (requires { v.first; v.second; }) {
      (*this)(v.first);
      (*this)(v.second);
    } else if constexpr (requires { v.size(); v.data(); typename T::value_type; }) {
      using E = typename T::value_type;
      if constexpr (requires { v.resize(std::size_t{}); }) v.resize(count());
      if constexpr (sizeof(E) == 1 && state_detail::Scalar<E>::value) bytes(v.data(), v.size());
      else
        for (E& e : v) (*this)(e);
    } else {
      static_assert(sizeof(T) == 0, "StateReader: list this type's fields");
    }
  }
  template <class T, std::size_t N>
  void operator()(T (&v)[N]) {
    for (T& e : v) (*this)(e);
  }

  void bytes(void* p, std::size_t n) {
    need(n);
    std::memcpy(p, in_.data() + at_, n);
    at_ += n;
  }
  std::span<const std::uint8_t> take(std::size_t n) {
    need(n);
    auto s = in_.subspan(at_, n);
    at_ += n;
    return s;
  }

  template <class T, class F>
  void vec(std::vector<T>& v, F each) {
    v.resize(count());
    for (T& e : v) each(e);
  }

 private:
  void need(std::size_t n) const {
    if (n > in_.size() - at_) throw StateError("save state: the file ends early");
  }
  std::size_t count() {
    std::uint64_t n = 0;
    (*this)(n);
    if (n > kMaxElements) throw StateError("save state: a table longer than any engine writes");
    return static_cast<std::size_t>(n);
  }
  template <class T>
  T get_scalar() {
    using B = state_detail::Base<T>;
    constexpr std::size_t w = state_detail::width<T>();
    need(w);
    const std::uint8_t* b = in_.data() + at_;
    at_ += w;
    if constexpr (std::is_floating_point_v<B>) {
      B f;
      std::memcpy(&f, b, w);
      return static_cast<T>(f);
    } else {
      std::uint64_t u = 0;
      for (std::size_t k = 0; k < w; ++k) u |= static_cast<std::uint64_t>(b[k]) << (8 * k);
      if constexpr (std::is_same_v<B, bool>) return static_cast<T>(u != 0);
      else return static_cast<T>(static_cast<B>(u));
    }
  }

  std::span<const std::uint8_t> in_;
  std::size_t at_ = 0;
};

}  // namespace mvp
