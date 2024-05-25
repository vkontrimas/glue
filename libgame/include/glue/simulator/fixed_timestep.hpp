#pragma once

#include <concepts>
#include <glue/assert.hpp>
#include <glue/types.hpp>

namespace glue::simulator {
class FixedTimestep final {
 public:
  explicit FixedTimestep(f64 timestep) noexcept : timestep_{timestep} {}

  f64 timestep() const noexcept { return timestep_; }

  f64 time_to_next_step() const noexcept { return time_to_next_step_; }

  f32 alpha() const noexcept {
    return static_cast<f32>(time_to_next_step_ / timestep_);
  }

  template <std::invocable<f64> StepFn>
  void update(f64 delta_time, StepFn step) {
    glue_assert(delta_time >= 0.0);
    glue_assert(timestep_ >= 0.0);
    glue_assert(time_to_next_step_ >= 0.0);

    time_to_next_step_ += delta_time;
    while (time_to_next_step_ >= timestep_) {
      time_to_next_step_ -= timestep_;
      step(timestep_);
    }
  }

 private:
  f64 time_to_next_step_ = 0.0;
  f64 timestep_ = 1.0 / 60.0;
};
}  // namespace glue::simulator