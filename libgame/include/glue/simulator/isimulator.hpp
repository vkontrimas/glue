#pragma once

#include <glue/input.hpp>
#include <glue/types.hpp>
#include <glue/world_frame.hpp>

namespace glue::simulator {
struct ISimulator {
  virtual ~ISimulator() = default;

  virtual void update(f64 delta_time, const Input& input) = 0;
  virtual void current_world_frame(WorldFrame& frame) = 0;
};
}  // namespace glue::simulator