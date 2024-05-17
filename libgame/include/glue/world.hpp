#pragma once

#include <glog/logging.h>

#include <glue/camera.hpp>
#include <glue/types.hpp>
#include <vector>

namespace glue {
struct World {
  Plane ground{{}, 3000.0f};
  Pose player;
  float player_width = 1.0f;
  std::vector<Pose> cubes;
  float cubes_width = 0.5f;
  OrbitCamera camera;

  void place_cubes(int rows) {
    cubes.reserve(rows * rows);
    float spacing = cubes_width * 2.25f;
    vec3 start =
        -1 * spacing * 0.5f * vec3{rows, 0, rows} + vec3{0, cubes_width, 0};
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < rows; ++j) {
        vec3 position = start + spacing * vec3{i, 0, j};
        cubes.push_back({position, glm::identity<quat>()});
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