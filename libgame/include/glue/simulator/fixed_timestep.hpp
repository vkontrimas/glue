#pragma once

#include <concepts>
#include <glue/assert.hpp>
#include <glue/types.hpp>

namespace glue::simulator {
template <typename T>
concept CStepper = requires(T a, f64 timestep) {
  { a.step(timestep) } -> std::same_as<void>;
};

class FixedTimestep final {
 public:
  explicit FixedTimestep(f64 timestep) noexcept : timestep_{timestep} {}

  f64 timestep() const noexcept { return timestep_; }

  f64 time_to_next_step() const noexcept { return time_to_next_step_; }

  template <CStepper Stepper>
  void update(Stepper& stepper, f64 delta_time) {
    glue_assert(delta_time >= 0.0);
    glue_assert(timestep_ >= 0.0);
    glue_assert(time_to_next_step_ >= 0.0);

    time_to_next_step_ += delta_time;
    while (time_to_next_step_ >= timestep_) {
      time_to_next_step_ -= timestep_;
      stepper.step(timestep_);
    }
  }

 private:
  f64 time_to_next_step_ = 0.0;
  f64 timestep_ = 1.0 / 60.0;
};
}  // namespace glue::simulator