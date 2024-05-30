#pragma once

namespace glue::math {
template <typename T>
inline constexpr T abs(const T& val) noexcept {
  if (val < T{}) {
    return -val;
  }
  return val;
}
}  // namespace glue::math