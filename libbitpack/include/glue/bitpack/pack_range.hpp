#pragma once

#include <concepts>
#include <glue/assert.hpp>
#include <glue/bitpack/math.hpp>
#include <glue/bitpack/pack_fundamental.hpp>

namespace glue::bitpack {
template <std::integral V, std::integral B>
inline constexpr void pack_range(Packer& packer, V& value, B begin, B end) {
  glue_assert(value >= begin);
  glue_assert(value < end);

  const auto bits_needed = bits_needed_for_range(begin, end);
  glue_assert(bits_needed <= 32);  // cannot be bothered to handle bigger values

  if (bits_needed > 0) {
    const u32 biased{static_cast<u32>(value - std::min(begin, end))};
    packer.write_bits(biased, bits_needed);
  }
}

template <std::integral V, std::integral B>
inline constexpr void pack_range(Unpacker& unpacker, V& value, B begin, B end) {
  const auto bits_needed = bits_needed_for_range(begin, end);
  glue_assert(bits_needed <= 32);  // cannot be bothered to handle bigger values

  if (bits_needed > 0) {
    value = begin + unpacker.read_bits(bits_needed);
  } else {
    value = begin;
  }
}
}  // namespace glue::bitpack