#pragma once

#include <glue/types.hpp>

namespace glue {
struct Perspective {
  f32 fov;
  f32 near_plane;
  f32 far_plane;
  f32 aspect;
};

struct OrbitCamera {
  vec3 target;
  radial position_rel;
  Perspective params;

  constexpr OrbitCamera() noexcept : OrbitCamera{1280.0f / 720.0f} {}

  constexpr OrbitCamera(f32 aspect_ratio) noexcept
      : OrbitCamera{
            radial{glm::radians(35.0f), glm::radians(45.0f), 10.0f},
            Perspective{glm::radians(60.0f), 0.05f, 300.0f, aspect_ratio}} {}

  constexpr OrbitCamera(radial position, Perspective perspective) noexcept
      : target{0.0f}, position_rel{position}, params{perspective} {}

  constexpr vec3 position() const noexcept {
    return target + position_rel.vector();
  }

  constexpr vec3 view_direction() const noexcept {
    return -position_rel.vector_normalized();
  }

  constexpr mat4 view_matrix() const noexcept {
    return glm::lookAt(position(), target, {0.0f, 1.0f, 0.0f});
  }

  constexpr mat4 projection_matrix() const noexcept {
    return glm::perspective(params.fov, params.aspect, params.near_plane,
                            params.far_plane);
  }
};
}  // namespace glue