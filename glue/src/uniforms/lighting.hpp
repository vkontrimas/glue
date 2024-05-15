#pragma once

#include <glue/types.hpp>

namespace glue::uniforms {
struct Lighting {
  vec4 ambient_light;
  vec4 sun_direction;  // this is a vec4 for alignment
  vec4 sun_light;

  constexpr Lighting() noexcept
      : Lighting{vec4{0.05f, 0.08f, 0.1f, 1.0f},
                 radial{glm::radians(65.0f), glm::radians(65.0f), 1.0f}
                     .vector_normalized(),
                 vec4{0.95f, 0.95f, 0.95f, 1.0f}} {}

  constexpr Lighting(vec4 ambient_light, vec3 sun_direction,
                     vec4 sun_light) noexcept
      : ambient_light{ambient_light},
        sun_direction{sun_direction, 1.0f},
        sun_light{sun_light} {}
};
}  // namespace glue::uniforms