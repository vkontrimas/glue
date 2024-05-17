#pragma once

#include <glog/logging.h>

#include <glue/physics/jolt_physics_engine.hpp>

namespace glue {
inline static auto create_physics_engine(f32 timestep = 1.0f / 60.0f) {
  return physics::JoltPhysicsEngine{timestep};
}
}  // namespace glue