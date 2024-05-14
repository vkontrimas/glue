#pragma once

#include <glue/typedefs.hpp>

namespace glue {
struct Plane {
  vec3 normal;
  f32 distance;

  constexpr Plane() noexcept : normal{0.0f, 1.0f, 0.0f}, distance{0.0f} {}
};
};  // namespace glue