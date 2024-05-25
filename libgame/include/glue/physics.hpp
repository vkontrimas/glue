#pragma once

#include <glog/logging.h>

#include <glue/physics/jolt_physics_engine.hpp>

namespace glue {
inline static auto create_physics_engine() {
  return physics::JoltPhysicsEngine{};
}
}  // namespace glue