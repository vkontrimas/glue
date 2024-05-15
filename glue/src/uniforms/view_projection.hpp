#pragma once

#include <glue/camera.hpp>
#include <glue/types.hpp>

namespace glue::uniforms {
// TODO(vkon): double check alignment compat with std140
struct ViewProjection {
  mat4 view;
  mat4 projection;

  // TODO(vkon): better default - position some offset from origin and a
  // perspective with some sensible values.
  constexpr ViewProjection() noexcept
      : view{glm::identity<mat4>()}, projection{glm::identity<mat4>()} {}

  constexpr ViewProjection(const OrbitCamera& camera) noexcept
      : view{camera.view_matrix()}, projection{camera.projection_matrix()} {}
};
}  // namespace glue::uniforms