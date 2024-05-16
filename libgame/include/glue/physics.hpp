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
  void update(float frame_delta_time, World& previous_world, World& world);

  float remaining_simulation_time() const noexcept {
    return remaining_simulation_time_;
  }

  float timestep() const noexcept { return 1.0f / 60.0f; }

 private:
  std::unique_ptr<PhysicsImpl> impl_;
  float remaining_simulation_time_ = 0.0f;
};
}  // namespace glue