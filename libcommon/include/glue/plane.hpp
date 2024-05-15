#pragma once

#include <glue/pose.hpp>
#include <glue/typedefs.hpp>

namespace glue {
struct Plane {
  Pose pose;
  f32 size;

  constexpr Plane() noexcept : pose{}, size{1.0f} {}
  constexpr Plane(Pose pose) noexcept : pose{pose}, size{1.0f} {}
  constexpr Plane(Pose pose, f32 size) noexcept : pose{pose}, size{size} {}
};
};  // namespace glue