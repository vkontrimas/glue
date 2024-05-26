#pragma once

#include <glue/collections/circular_buffer.hpp>
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
        frame_buffer_{buffer_frames(), {initial_frame}},
        input_buffer_{buffer_frames(), {Input{}}} {}

  virtual void update(f64 delta_time, Input& input) override {
    auto logger = debug::NoOpDataLogger<f64>{};
    update_timed(delta_time, input, logger);
  };

  template <debug::CDataLogger<f64> TDataLogger = debug::NoOpDataLogger<f64>>
  void update_timed(f64 delta_time, Input& input, TDataLogger& logger) {
    timestep_.update(delta_time, [&](f64 timestep) {
      debug::Timer timer;

      if (input_buffer_.full()) {
        input_buffer_.pop_front();
      }
      input_buffer_.push_back(input);

      if (frame_buffer_.full()) {
        frame_buffer_.pop_front();
      }
      frame_buffer_.emplace_back();
      // copy prev frame into future one as we're building on top of it
      std::memcpy(&frame_buffer_[frame_buffer_.size() - 1],
                  &frame_buffer_[frame_buffer_.size() - 2], sizeof(WorldFrame));

      auto& future_frame = frame_buffer_[frame_buffer_.size() - 1];
      future_frame.active_cubes.clear();

      director_->pre_physics(timestep, input, future_frame);
      physics_->step(timestep, future_frame);
      director_->post_physics(timestep, input, future_frame);
      logger.log(timer.elapsed_ms<f64>());

      ++current_frame_;
      future_frame.index = static_cast<u32>(current_frame_);
    });
  }

  virtual void current_world_frame(WorldFrame& frame) override {
    WorldFrame::interpolate(frame_buffer_[frame_buffer_.size() - 2],
                            frame_buffer_[frame_buffer_.size() - 1],
                            timestep_.alpha(), frame);
  }

  const std::size_t buffer_frames() const noexcept { return buffer_frames_; }
  const std::size_t current_frame() const noexcept { return current_frame_; }

 private:
  std::shared_ptr<director::IGameDirector> director_;
  std::shared_ptr<physics::IPhysicsEngine> physics_;
  FixedTimestep timestep_;
  std::size_t buffer_frames_;
  std::size_t current_frame_ = 0;

  // TODO(vkon): circular buffer
  CircularBuffer<WorldFrame> frame_buffer_;
  CircularBuffer<Input> input_buffer_;
};
}  // namespace glue::simulator