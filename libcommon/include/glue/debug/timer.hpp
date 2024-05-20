#pragma once

#include <chrono>

namespace glue::debug {
class Timer final {
 public:
  Timer() : start_{std::chrono::high_resolution_clock::now()} {}

  template <typename T>
  T elapsed_sec() const {
    const auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<T>(now - start_).count();
  }

  template <typename T>
  T elapsed_ms() const {
    const auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<T, std::milli>(now - start_).count();
  }

 private:
  std::chrono::high_resolution_clock::time_point start_;
};
}  // namespace glue::debug