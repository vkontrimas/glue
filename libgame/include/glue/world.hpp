#pragma once

#include <glue/camera.hpp>
#include <glue/types.hpp>

namespace glue {
struct World {
  Plane ground;
  Pose cube;
  OrbitCamera camera;
};
}  // namespace glue