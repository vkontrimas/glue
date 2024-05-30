#pragma once

#include <concepts>
#include <glue/bitpack/packer.hpp>

namespace glue::bitpack {
template <std::unsigned_integral T>
inline void pack(Packer& packer, T value) {
  const Packer::value_t val{value};
  packer.write_bits(val, sizeof(T) * 8);
}
template <std::unsigned_integral T>
inline void pack(Unpacker& packer, T& val) {
  val = packer.read_bits(sizeof(T) * 8);
}

inline void pack(Packer& packer, bool val) { packer.write_bits(val, 1); }
inline void pack(Unpacker& packer, bool& val) { val = packer.read_bits(1); }
}  // namespace glue::bitpack