#pragma once

#include <glog/logging.h>

#include <concepts>
#include <glue/assert.hpp>
#include <glue/bitpack/math.hpp>
#include <glue/bitpack/pack_fundamental.hpp>

namespace glue::bitpack {
template <std::integral V, std::integral B>
inline constexpr void pack_bits_wrapping(Packer& packer, V& value, B begin,
                                         u32 bits) {
  // cannot be bothered to pack larger values right now
  glue_assert(bits <= 32);
  if (bits > 0) {
    const u32 biased{static_cast<u32>(value - begin)};
    packer.write_bits(biased, bits);
  }
}

template <std::integral V, std::integral B>
inline constexpr void pack_bits_wrapping(Unpacker& packer, V& value, B begin,
                                         u32 bits) {
  // cannot be bothered to pack larger values right now
  glue_assert(bits <= 32);
  if (bits > 0) {
    value = begin + packer.read_bits(bits);
  } else {
    value = begin;
  }
}

template <std::integral V, std::integral B>
inline constexpr void pack_bits(Packer& packer, V& value, B begin, u32 bits) {
  glue_assert(value >= begin);
  glue_assert((value - begin) < (1 << bits));
  pack_bits_wrapping(packer, value, begin, bits);
}

template <std::integral V, std::integral B>
inline constexpr void pack_bits(Unpacker& packer, V& value, B begin, u32 bits) {
  pack_bits_wrapping(packer, value, begin, bits);
}
}  // namespace glue::bitpack