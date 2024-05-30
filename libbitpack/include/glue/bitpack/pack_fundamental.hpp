#pragma once

#include <concepts>
#include <glue/bitpack/packer.hpp>
#include <type_traits>

namespace glue::bitpack {
inline constexpr void pack(Packer& packer, u8& value) {
  packer.write_bits(value, 8);
}
inline constexpr void pack(Unpacker& packer, u8& value) {
  value = packer.read_bits(8);
}

inline constexpr void pack(Packer& packer, u16& value) {
  packer.write_bits(value, 16);
}
inline constexpr void pack(Unpacker& packer, u16& value) {
  value = packer.read_bits(16);
}

inline constexpr void pack(Packer& packer, u32& value) {
  packer.write_bits(value, 32);
}
inline constexpr void pack(Unpacker& packer, u32& value) {
  value = packer.read_bits(32);
}

template <CPacker TPacker>
inline constexpr void pack(TPacker& packer, u64& val) {
  u32 high_bits{static_cast<u32>((val >> 32) & 0xffffffff)};
  u32 low_bits{static_cast<u32>(val & 0xffffffff)};
  pack(packer, high_bits);
  pack(packer, low_bits);
  val = (u64{high_bits} << 32) | low_bits;
}

template <CPacker TPacker, std::signed_integral T>
inline constexpr void pack(TPacker& packer, T& val) {
  using UnsignedT = std::make_unsigned_t<T>;
  UnsignedT uval = static_cast<UnsignedT>(val);
  pack(packer, uval);
  val = static_cast<T>(uval);
}

template <CPacker TPacker>
inline constexpr void pack(TPacker& packer, f32& val) {
  // UB but how it's done in game industry afaik.
  union {
    f32 float_val;
    u32 integral_val;
  };
  float_val = val;
  pack(packer, integral_val);
  val = float_val;
}

template <CPacker TPacker>
inline constexpr void pack(TPacker& packer, f64& val) {
  // UB but how it's done in game industry afaik.
  union {
    f64 float_val;
    u64 integral_val;
  };
  float_val = val;
  pack(packer, integral_val);
  val = float_val;
}

inline bool constexpr pack(Packer& packer, bool val) {
  packer.write_bits(val, 1);
  return val;
}
inline bool constexpr pack(Unpacker& packer, bool& val) {
  val = packer.read_bits(1);
  return val;
}
}  // namespace glue::bitpack