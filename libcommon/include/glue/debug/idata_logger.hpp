#pragma once

namespace glue::debug {
template <typename T>
struct IDataLogger {
  virtual ~IDataLogger() = default;
  virtual void log(T data) = 0;
};
}  // namespace glue::debug