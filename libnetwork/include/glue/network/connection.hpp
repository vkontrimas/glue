#pragma once

namespace glue::network {
struct IConnection {
  virtual ~IConnection() = default;
};

/*
 * A UDP connection with another party.
 *
 * Sends packets to the destination IP and receives packets filtered by same IP.
 *
 * Implements a lightweight packet receipt tracker.
 */
class Connection final {
 public:
 private:
};
}  // namespace glue::network