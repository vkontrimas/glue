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

  constexpr mat4 model_matrix() const noexcept {
    return glm::translate(glm::identity<mat4>(), position) *
           glm::toMat4(rotation);
  }
};

inline constexpr static Pose lerp(const Pose& pose_a, const Pose& pose_b,
                                  float a) {
  return Pose{glm::mix(pose_a.position, pose_b.position, a),
              glm::slerp(pose_a.rotation, pose_b.rotation, a)};
}
}  // namespace glue