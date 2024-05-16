#pragma once

#include <glue/camera.hpp>
#include <glue/types.hpp>

namespace glue {
struct World {
  Plane ground{{}, 3000.0f};
  Pose cube;
  OrbitCamera camera;
};

inline World interpolated(const World& a, const World& b, float t) {
  World result = a;
  result.cube.position = glm::mix(a.cube.position, b.cube.position, t);
  result.cube.rotation = glm::slerp(a.cube.rotation, b.cube.rotation, t);
  result.camera.target = result.cube.position;
  return result;
}
}  // namespace glue