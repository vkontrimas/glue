#pragma once

#include <SDL.h>

#include <deque>
#include <glue/debug/idata_logger.hpp>

namespace glue::debug {
template <typename T>
class DataLogger final : public IDataLogger<T> {
 public:
  struct Entry {
    double time;
    T data;
  };

  explicit DataLogger(double duration)
      : timestamp_{SDL_GetPerformanceCounter()}, duration_{duration} {}

  void log(T data) override {
    const auto time = get_time();
    data_.push_back({time, data});
    while (time - data_.front().time >= duration_) {
      data_.pop_front();
    }
  }

  double duration() const { return duration_; }
  std::size_t count() const { return data_.size(); }
  const Entry& operator[](std::size_t index) const { return data_[index]; }

 private:
  u64 timestamp_;
  double duration_;

  std::deque<Entry> data_;

  double get_time() {
    const u64 now = SDL_GetPerformanceCounter();
    return static_cast<double>(now - timestamp_) /
           static_cast<double>(SDL_GetPerformanceFrequency());
  }
};
}  // namespace glue::debug