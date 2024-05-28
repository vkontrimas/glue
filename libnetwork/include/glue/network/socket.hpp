#pragma once

#include <glue/network/address.hpp>
#include <memory>
#include <optional>
#include <span>

#include "detail/socket_handle.inl"

namespace glue::network {
class Socket final {
 public:
  constexpr Socket() noexcept : handle_{0}, port_{0} {}

  constexpr Socket(const Socket&) = delete;
  constexpr Socket& operator=(const Socket&) = delete;

  constexpr Socket(Socket&& other) noexcept { swap(*this, other); }
  constexpr Socket& operator=(Socket&& other) noexcept {
    swap(*this, other);
    return *this;
  }

  ~Socket();

  static std::optional<Socket> open(u16 port);

  void send(const IPv4Address& address, std::span<u8> data);
  bool receive(std::span<u8> data, IPv4Address& sender);

  friend constexpr void swap(Socket& a, Socket& b) noexcept {
    using std::swap;
    swap(a.handle_, b.handle_);
    swap(a.port_, b.port_);
  }

  constexpr u16 port() const noexcept { return port_; }

 private:
  constexpr Socket(detail::SocketHandle handle, u16 port) noexcept
      : handle_{handle}, port_{port} {}

 private:
  detail::SocketHandle handle_;
  u16 port_;
};
}  // namespace glue::network