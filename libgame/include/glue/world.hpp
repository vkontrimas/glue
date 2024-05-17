#pragma once

#include <glog/logging.h>

#include <glue/camera.hpp>
#include <glue/types.hpp>
#include <vector>

namespace glue {
struct World {
  Plane ground{{}, 3000.0f};
  Pose player;
  std::vector<Pose> cubes;
  OrbitCamera camera;

  void place_cubes(int rows) {
    cubes.resize(rows * rows);

    const float start = -(rows) + 1.5f;

    int cube = 0;
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < rows; ++j) {
        vec3 position{start + 2.0f * i, 1.0f, start + 2.0f * j};
        cubes[cube] = {position, glm::identity<quat>()};
        cube++;
      }
    }
  }
};

inline World interpolated(const World& a, const World& b, float t) {
  World result = a;
  result.player.position = glm::mix(a.player.position, b.player.position, t);
  result.player.rotation = glm::slerp(a.player.rotation, b.player.rotation, t);
  result.camera.target = result.player.position;
  CHECK(a.cubes.size() == b.cubes.size())
      << "both worlds must have same number of cubes.";
  for (int i = 0; i < a.cubes.size(); ++i) {
    result.cubes[i].position =
        glm::mix(a.cubes[i].position, b.cubes[i].position, t);
    result.cubes[i].rotation =
        glm::slerp(a.cubes[i].rotation, b.cubes[i].rotation, t);
  }
  return result;
}
}  // namespace glue