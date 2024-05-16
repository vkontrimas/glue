#pragma once

#include <glue/camera.hpp>
#include <glue/types.hpp>

namespace glue {
struct World {
  const Plane ground{{}, 3000.0f};
  Pose cube;
  OrbitCamera camera;
};
}  // namespace glue