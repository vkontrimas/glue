#pragma once

#include <SDL.h>

#include <deque>
#include <glue/types.hpp>

namespace glue::debug {
template <typename T>
class FrameLogger final {
 public:
  explicit FrameLogger(f64 seconds) : seconds_{seconds} {}

  void start_frame(f64 timestamp) {
    datapoints_.push_back({timestamp, {}});

    while (timestamp - datapoints_.front().timestamp > seconds_) {
      datapoints_.pop_front();
    }
  }

  T& current_frame() { return datapoints_.back().data; }
  const T& current_frame() const { return datapoints_.back().data; }

  const T& operator[](std::size_t index) { return datapoints_[index].data; }

  f64 time(std::size_t index) {
    return datapoints_[index].timestamp - datapoints_.back().timestamp;
  }

  std::size_t count() const {
    return datapoints_.empty() ? 0 : datapoints_.size() - 1;
  }
  f32 seconds() const noexcept { return seconds_; }

 private:
  struct Data {
    f64 timestamp;
    T data;
  };

  std::deque<Data> datapoints_;
  f64 seconds_;
};
}  // namespace glue::debug