#pragma once

#include <glue/camera.hpp>
#include <glue/types.hpp>

namespace glue {
struct World {
  Plane ground{{}, 3000.0f};
  Pose player;
  OrbitCamera camera;
};

inline World interpolated(const World& a, const World& b, float t) {
  World result = a;
  result.player.position = glm::mix(a.player.position, b.player.position, t);
  result.player.rotation = glm::slerp(a.player.rotation, b.player.rotation, t);
  result.camera.target = result.player.position;
  return result;
}
}  // namespace glue