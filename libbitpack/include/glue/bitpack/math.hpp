#pragma once

#include <algorithm>
#include <concepts>
#include <glue/assert.hpp>
#include <glue/types.hpp>

namespace glue::bitpack {
template <std::integral V>
inline constexpr u32 bits_to_represent_n_values(V n) noexcept {
  glue_assert(n >= 0);
  if (n <= 1) {
    // n = 0 - nothing to send
    // n = 1 - one value, nothing to send
    return 0;
  }

  // no constexpr log2 :(
  n -= 1;  // handle powers of two
  u32 bits = 0;
  while (n) {
    n /= 2;
    ++bits;
  }
  return bits;
}

template <std::integral V>
inline constexpr u32 bits_needed_for_range(V begin, V end) {
  const auto n{std::max(begin, end) - std::min(begin, end)};
  return bits_to_represent_n_values(n);
}
}  // namespace glue::bitpack