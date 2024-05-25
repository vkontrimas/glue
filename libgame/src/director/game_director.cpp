#include <glue/director/game_director.hpp>

namespace glue::director {
void GameDirector::pre_physics(f64 timestep, const Input& input,
                               WorldFrame& frame,
                               physics::IPhysicsEngine& physics) {
  static auto player_id = ObjectID{"player"};
  if (glm::dot(input.direction, input.direction) > 0.1f) {
    physics.add_torque(player_id, glm::cross(vec3{0, 1, 0}, input.direction),
                       5000.0f);
    physics.add_force(player_id, input.direction * 1250.0f);
  }

  if (input.jump) {
    physics.add_impulse(player_id, vec3{0.0f, 5000.0f, 0.0f});
  }
}

void GameDirector::post_physics(f64 timestep, const Input& input,
                                WorldFrame& frame, physics::IPhysicsEngine&) {
  frame.camera.target = frame.cubes[0].position;
}
}  // namespace glue::director