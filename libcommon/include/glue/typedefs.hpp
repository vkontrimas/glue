#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace glue {
using f32 = float;
using f64 = double;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int32_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint32_t;

using quat = glm::quat;

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec3;

using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using ivec4 = glm::ivec3;

using uvec2 = glm::uvec2;
using uvec3 = glm::uvec3;
using uvec4 = glm::uvec3;

using mat3 = glm::mat3;
using mat4 = glm::mat4;
}  // namespace glue