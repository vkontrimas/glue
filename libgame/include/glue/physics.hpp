#pragma once

#include <glue/world.hpp>
#include <memory>

namespace glue {
class PhysicsImpl;

class Physics {
 public:
  Physics();
  ~Physics();

  void setup_static_objects(const World& world);
  void update(float frame_delta_time, World& world);

 private:
  std::unique_ptr<PhysicsImpl> impl_;
  float remaining_simulation_time_ = 0.0f;
};
}  // namespace glue