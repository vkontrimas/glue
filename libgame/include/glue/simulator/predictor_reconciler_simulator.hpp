#pragma once

#include <glue/debug/idata_logger.hpp>
#include <glue/debug/timer.hpp>
#include <glue/director/igame_director.hpp>
#include <glue/physics/iphysics_engine.hpp>
#include <glue/simulator/fixed_timestep.hpp>
#include <glue/simulator/isimulator.hpp>
#include <glue/world_frame.hpp>
#include <memory>

namespace glue::simulator {
class PredictorReconcilerSimulator final : public ISimulator {
 public:
  PredictorReconcilerSimulator(
      std::shared_ptr<director::IGameDirector> director,
      std::shared_ptr<physics::IPhysicsEngine> physics,
      const WorldFrame& initial_frame, f64 timestep,
      f64 buffer_duration) noexcept
      : director_{director},
        physics_{physics},
        timestep_{timestep},
        buffer_frames_{std::max(2ul, static_cast<std::size_t>(glm::ceil(
                                         buffer_duration / timestep)))},
        past_frame_{initial_frame},
        future_frame_{initial_frame} {}

  virtual void update(f64 delta_time, const Input& input) override {
    auto logger = debug::NoOpDataLogger<f64>{};
    update_timed(delta_time, input, logger);
  };

  template <debug::CDataLogger<f64> TDataLogger = debug::NoOpDataLogger<f64>>
  void update_timed(f64 delta_time, const Input& input, TDataLogger& logger) {
    timestep_.update(delta_time, [&](f64 timestep) {
      debug::Timer timer;
      std::memcpy(&past_frame_, &future_frame_, sizeof(WorldFrame));

      director_->pre_physics(timestep, input, future_frame_, *physics_);
      physics_->step(timestep, future_frame_);
      director_->post_physics(timestep, input, future_frame_, *physics_);
      logger.log(timer.elapsed_ms<f64>());
    });
  }

  virtual void current_world_frame(WorldFrame& frame) override {
    WorldFrame::interpolate(past_frame_, future_frame_, timestep_.alpha(),
                            frame);
  }

  const std::size_t buffer_frames() const noexcept { return buffer_frames_; }

 private:
  std::shared_ptr<director::IGameDirector> director_;
  std::shared_ptr<physics::IPhysicsEngine> physics_;
  FixedTimestep timestep_;
  std::size_t buffer_frames_;

  // TODO(vkon): circular buffer
  WorldFrame past_frame_;
  WorldFrame future_frame_;
};
}  // namespace glue::simulator