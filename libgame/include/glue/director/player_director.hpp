#pragma once

#include <glue/input.hpp>
#include <glue/physics/iphysics_engine.hpp>
#include <glue/types.hpp>

namespace glue::director {
class PlayerDirector {
 public:
  PlayerDirector(std::shared_ptr<physics::IPhysicsEngine> physics,
                 i32 max_jumps, ObjectID ground_id)
      : physics_{physics}, player_id_{"player"}, max_jumps_{max_jumps} {
    physics_->on_collision_enter(player_id_,
                                 [&, ground_id = ground_id](ObjectID other) {
                                   if (other == ground_id) {
                                     jump_count_ = 0;
                                   }
                                 });
  }

  void pre_physics(Input& input) {
    if (glm::dot(input.direction, input.direction) > 0.1f) {
      physics_->add_torque(player_id_,
                           glm::cross(vec3{0, 1, 0}, input.direction), 5000.0f);
      physics_->add_force(player_id_, input.direction * 1250.0f);
    }

    if (input.jump) {
      if (jump_count_ < max_jumps_) {
        physics_->add_impulse(player_id_, vec3{0.0f, 5000.0f, 0.0f});
        jump_count_++;
      }
      input.jump = false;
    }
  }

 private:
  std::shared_ptr<physics::IPhysicsEngine> physics_;
  ObjectID player_id_;
  i32 max_jumps_ = 3;
  i32 jump_count_ = 0;
};
}  // namespace glue::director