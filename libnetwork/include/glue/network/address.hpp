#pragma once

#include <glue/types.hpp>

namespace glue::network {
inline constexpr u32 ipv4_address(u32 a, u32 b, u32 c, u32 d) noexcept {
  return (a << 24) | (b << 16) | (c << 8) | d;
}

class IPv4Address final {
 public:
  constexpr IPv4Address() noexcept : ip_{0}, port_{0} {}
  constexpr IPv4Address(u8 a, u8 b, u8 c, u8 d, u16 port)
      : ip_{ipv4_address(a, b, c, d)}, port_{port} {}

  static constexpr IPv4Address loopback(u16 port) noexcept {
    return IPv4Address{127, 0, 0, 1, port};
  }

  constexpr u32 ip() const noexcept { return ip_; }
  constexpr u16 port() const noexcept { return port_; }

  friend constexpr bool operator==(const IPv4Address& a,
                                   const IPv4Address& b) noexcept {
    return a.ip_ == b.ip_ && a.port_ == b.port_;
  }

  friend constexpr bool operator!=(const IPv4Address& a,
                                   const IPv4Address& b) noexcept {
    return !(a == b);
  }

 private:
  u32 ip_;
  u16 port_;
};
}  // namespace glue::network