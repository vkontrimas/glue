#pragma once

#include <glue/director/igame_director.hpp>
#include <glue/director/player_director.hpp>

namespace glue::director {
class GameDirector final : public IGameDirector {
 public:
  explicit GameDirector(std::shared_ptr<physics::IPhysicsEngine> physics,
                        std::shared_ptr<PlayerDirector> player_director)
      : physics_{physics}, player_director_{player_director} {}

  void pre_physics(f64 timestep, Input& input, WorldFrame& frame) override {
    player_director_->pre_physics(input);
  }

  void post_physics(f64 timestep, Input& input, WorldFrame& frame) override {
    frame.camera.target = frame.cubes[0].position;
  }

 private:
  std::shared_ptr<physics::IPhysicsEngine> physics_;
  std::shared_ptr<PlayerDirector> player_director_;
};
}  // namespace glue::director