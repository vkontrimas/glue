#include <fcntl.h>
#include <glog/logging.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <glue/assert.hpp>
#include <glue/network/socket.hpp>

// WINDOWS: initialize socket layer once, globally

namespace glue::network {
Socket::~Socket() {
  if (handle_) {
    shutdown(handle_, SHUT_RDWR);
    close(handle_);  // WINDOWS: closesocket() instead
  }
}

std::optional<Socket> Socket::open(u16 port) {
  Socket out{socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), port};
  if (out.handle_ <= 0) {
    LOG(ERROR) << "Failed to open UDP socket for port " << port;
    return std::nullopt;
  }

  /*
   * sockaddr in socket.h is a generic base for all socket types
   *
   * sockaddr_in comes from netinet/in and is for IPv4 sockets,
   * sockaddr_in6 is IPv6 in case we ever want to support it.
   *
   * AF_INET above and below indicates precisely the type of socket we're using.
   *
   * The "_in" stands for "InterNetwork" ~= Internet Protocol
   */
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port =
      htons(port);  // htons converts from host to network endianess

  const auto bind_status =
      bind(out.handle_, reinterpret_cast<const sockaddr*>(&address),
           sizeof(sockaddr_in));
  if (bind_status < 0) {
    LOG(ERROR) << "Failed to bind to UDP port " << port;
    return std::nullopt;
  }

  /*
   * Set socket file descriptor to non-blocking mode to have non-blocking reads.
   */
  const auto non_blocking_status = fcntl(out.handle_, F_SETFL, O_NONBLOCK, 1);
  if (non_blocking_status == -1) {
    LOG(ERROR) << "Failed to set socket on UDP port " << port
               << " to non-blocking mode";
    return std::nullopt;
  }

  return {std::move(out)};
}

std::optional<Socket> Socket::open_any_port() {
  auto maybe_socket = Socket::open(0);
  if (!maybe_socket.has_value()) {
    return std::nullopt;
  }

  // fill in Socket::port
  sockaddr_in addr{};
  u32 addr_length = sizeof(addr);
  auto get_name_status = getsockname(
      maybe_socket->handle_, reinterpret_cast<sockaddr*>(&addr), &addr_length);
  if (get_name_status != 0) {
    LOG(ERROR) << "Failed to retrieve UDP port from socket";
    return std::nullopt;
  }
  glue_assert(addr_length == sizeof(addr));
  const u16 port = ntohs(addr.sin_port);
  maybe_socket->port_ = port;

  return maybe_socket;
}

void Socket::send(const IPv4Address& address, std::span<u8> data) {
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(address.ip());
  addr.sin_port = htons(address.port());

  const auto sent_bytes =
      sendto(handle_, reinterpret_cast<const char*>(data.data()),
             static_cast<int>(data.size()), 0,
             reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in));
  if (sent_bytes != data.size()) {
    // PERF: structured logging or no logging at all
    LOG(ERROR) << "failed to send packet";
  }
}

bool Socket::receive(std::span<u8> data, IPv4Address& sender) {
  sockaddr_in sender_addr{};
  u32 sender_addr_length = sizeof(sender_addr);
  const auto received_bytes =
      recvfrom(handle_, reinterpret_cast<char*>(data.data()),
               static_cast<int>(data.size()), 0,
               reinterpret_cast<sockaddr*>(&sender_addr), &sender_addr_length);

  if (received_bytes <= 0) {
    return false;
  }

  const u32 sender_ip = ntohl(sender_addr.sin_addr.s_addr);
  const u16 sender_port = ntohs(sender_addr.sin_port);
  sender = IPv4Address{sender_ip, sender_port};

  return true;
}
}  // namespace glue::network