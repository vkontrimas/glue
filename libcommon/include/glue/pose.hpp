#pragma once

#include <glue/typedefs.hpp>

namespace glue {
struct Pose {
  vec3 position;
  quat rotation;

  constexpr Pose() noexcept : position{0.0f}, rotation{glm::identity<quat>()} {}
  explicit constexpr Pose(vec3 position) noexcept
      : position{position}, rotation{glm::identity<quat>()} {}
  constexpr Pose(vec3 position, quat rotation) noexcept
      : position{position}, rotation{rotation} {}
};
}  // namespace glue