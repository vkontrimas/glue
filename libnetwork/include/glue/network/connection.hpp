#pragma once

namespace glue::network {
struct IConnection {
  virtual ~IConnection() = default;
};

class Connection final {};
}  // namespace glue::network