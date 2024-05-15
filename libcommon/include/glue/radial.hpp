#pragma once

#include <glue/typedefs.hpp>

namespace glue {
struct radial {
  f32 pitch;
  f32 yaw;
  f32 distance;

  constexpr vec3 vector_normalized() const noexcept {
    constexpr vec3 up{0.0f, 1.0f, 0.0f};
    constexpr vec3 forward{0.0f, 0.0f, -1.0f};
    constexpr vec3 right{1.0f, 0.0f, 0.0f};

    const vec3 unpitched = glm::sin(yaw) * forward + glm::cos(yaw) * right;
    const vec3 pitched = glm::sin(pitch) * up + glm::cos(pitch) * unpitched;
    return pitched;
  }

  constexpr vec3 vector() const noexcept {
    return vector_normalized() * distance;
  }
};
}  // namespace glue