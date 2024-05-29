#pragma once

#include <concepts>
#include <cstdint>

namespace glue {
template <typename T, std::integral AlignT>
inline bool ptr_is_aligned(T* pointer, AlignT alignment) {
  const auto value = reinterpret_cast<intptr_t>(pointer);
  // least significant bits must be zero
  return (value & static_cast<intptr_t>(alignment)) == 0;
}
}  // namespace glue