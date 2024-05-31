#pragma once

#include <concepts>

namespace glue::bitpack {
template <typename T>
concept CIntegralOrRef = std::integral<std::remove_reference_t<T>>;
}