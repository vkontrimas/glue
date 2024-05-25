#pragma once

#include <glue/director/igame_director.hpp>

namespace glue::director {
class GameDirector final : public IGameDirector {
public:
  void pre_physics(f64 timestep, const Input& input, WorldFrame& frame,
                   physics::IPhysicsEngine&) override;

  void post_physics(f64 timestep, const Input& input, WorldFrame& frame,
                    physics::IPhysicsEngine&) override;
};
}  // namespace glue::director