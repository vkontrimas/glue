#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/debug/timer.hpp>
#include <glue/network/socket.hpp>
#include <glue/types.hpp>
#include <thread>
#include <tuple>
#include <vector>

using namespace glue;
using namespace glue::network;

class SocketTests : public ::testing::Test {
 public:
  std::pair<IPv4Address, Socket> open_test_socket() {
    u16 port = 15000;
    for (; port < std::numeric_limits<u16>::max(); ++port) {
      auto maybe_socket = Socket::open(port);
      if (maybe_socket) {
        return {IPv4Address::loopback(port), std::move(maybe_socket.value())};
      }
    }
    return {};
  }

 private:
};

TEST_F(SocketTests, GivenTwoSockets_WhenPacketSent_PacketReceivedByOther) {
  auto a = open_test_socket();
  auto b = open_test_socket();
  auto& [ip_a, socket_a] = a;
  auto& [ip_b, socket_b] = b;

  std::vector<u8> sent_data{10, 2, 3, 54, 20, 80, 92, 42, 50, 12};

  std::vector<u8> received_data;
  received_data.resize(sent_data.size());

  IPv4Address sender_ip;
  bool timed_out = false;

  std::thread poll_thread{[&]() {
    auto& [ip_a, socket_a] = a;
    auto& [ip_b, socket_b] = b;

    // we'll poll until we receive a packet or reach some max time.
    constexpr f64 kMaxTimeSec = 0.250;
    debug::Timer timer;

    for (;;) {
      // important: check sender_ip matches ip_a to ignore packets from other
      // senders
      if (socket_b.receive(received_data, sender_ip) && sender_ip == ip_a) {
        return;
      }

      if (timer.elapsed_sec<f64>() >= kMaxTimeSec) {
        timed_out = true;
        break;
      }
    }
  }};

  // to permit some packet loss
  // though unlikely with loopback
  constexpr i32 kSentPackets = 20;
  for (i32 i = 0; i < kSentPackets; ++i) {
    socket_a.send(ip_b, sent_data);
  }

  poll_thread.join();

  EXPECT_FALSE(timed_out);
  EXPECT_THAT(received_data, ::testing::ContainerEq(sent_data));
}

TEST_F(SocketTests, GivenOpenSocket_CannotOpenSocketOnSamePort) {
  auto [ip_a, socket_a] = open_test_socket();
  auto maybe_new_socket = Socket::open(ip_a.port());
  EXPECT_FALSE(maybe_new_socket.has_value());
}

TEST_F(SocketTests,
       GivenSocketsWithAnyPort_SocketsAreOpenAndPortsAreDifferent) {
  auto maybe_a = Socket::open_any_port();
  auto maybe_b = Socket::open_any_port();

  ASSERT_TRUE(maybe_a.has_value());
  ASSERT_TRUE(maybe_b.has_value());
  EXPECT_NE(maybe_a->port(), maybe_b->port());
}

TEST_F(SocketTests, GivenSocketWithAnyPort_PortIsNonZero) {
  auto maybe_socket = Socket::open_any_port();
  ASSERT_TRUE(maybe_socket.has_value());
  EXPECT_NE(maybe_socket->port(), 0);
}