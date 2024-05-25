#pragma once

#include <glue/input.hpp>
#include <glue/physics/iphysics_engine.hpp>
#include <glue/types.hpp>
#include <glue/world_frame.hpp>

namespace glue::director {
struct IGameDirector {
  virtual ~IGameDirector() = default;
  virtual void pre_physics(f64 timestep, const Input& input, WorldFrame& frame,
                           physics::IPhysicsEngine&) = 0;

  virtual void post_physics(f64 timestep, const Input& input, WorldFrame& frame,
                            physics::IPhysicsEngine&) = 0;
};
}  // namespace glue::director