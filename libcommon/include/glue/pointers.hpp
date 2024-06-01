#pragma once

#include <glog/logging.h>

#include <bitset>
#include <concepts>
#include <cstdint>

namespace glue {
template <typename T, std::integral AlignT>
inline bool ptr_is_aligned(T* pointer, AlignT alignment) {
  // least significant bits must be zero
  const auto value = reinterpret_cast<intptr_t>(pointer);
  const auto mask = static_cast<intptr_t>(alignment) - 1;
  return (value & mask) == 0;
}
}  // namespace glue