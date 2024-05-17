#pragma once

#include <Jolt/Jolt.h>

#include <glue/types.hpp>

namespace glue {
inline JPH::Vec3 from_glm(vec3 pos) noexcept { return {pos.x, pos.y, pos.z}; }

inline JPH::Quat from_glm(quat o) noexcept {
  return JPH::Quat{o.x, o.y, o.z, o.w};
}

inline vec3 to_glm(JPH::Vec3 pos) noexcept {
  return {pos.GetX(), pos.GetY(), pos.GetZ()};
}
inline quat to_glm(JPH::Quat o) noexcept {
  return {o.GetW(), o.GetX(), o.GetY(), o.GetZ()};
}
}  // namespace glue