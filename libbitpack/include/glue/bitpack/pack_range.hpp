#pragma once

#include <concepts>
#include <glue/assert.hpp>
#include <glue/bitpack/pack_bits.hpp>

namespace glue::bitpack {
template <std::integral V, std::integral B>
inline constexpr void pack_range(Packer& packer, V& value, B begin, B end) {
  glue_assert(value >= begin);
  glue_assert(value < end);

  const auto bits_needed = bits_needed_for_range(begin, end);
  pack_bits(packer, value, begin, bits_needed);
}

template <std::integral V, std::integral B>
inline constexpr void pack_range(Unpacker& packer, V& value, B begin, B end) {
  const auto bits_needed = bits_needed_for_range(begin, end);
  pack_bits(packer, value, begin, bits_needed);
}
}  // namespace glue::bitpack