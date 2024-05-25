#pragma once

#include <concepts>

namespace glue::debug {
template <typename T>
struct IDataLogger {
  virtual ~IDataLogger() = default;
  virtual void log(T data) = 0;
};

template <typename T, typename TData>
concept CDataLogger = requires(T t, TData data) {
  { t.log(data) } -> std::same_as<void>;
};

template <typename TData>
struct NoOpDataLogger : public IDataLogger<TData> {
  virtual void log(TData) override {}
};
}  // namespace glue::debug