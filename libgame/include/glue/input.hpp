#pragma once

#include <glue/types.hpp>

namespace glue {
struct Input {
  vec3 direction{0.0f};
  bool jump{false};
};
}  // namespace glue