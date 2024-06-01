#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/network/packet.hpp>
#include <numeric>

using namespace glue::network;
using namespace glue;
using namespace testing;

TEST(PacketTests, WhenDefaultConstructed_ZeroSized) {
  Packet packet{};

  EXPECT_EQ(packet.index(), 0);
  EXPECT_EQ(packet.size_bytes(), 0);
  EXPECT_EQ(packet.begin(), packet.end());
  EXPECT_EQ(packet.cbegin(), packet.cend());
}

TEST(PacketTests, GivenConstPacket_WhenDefaultConstructed_ZeroSized) {
  const Packet packet{};

  EXPECT_EQ(packet.index(), 0);
  EXPECT_EQ(packet.size_bytes(), 0);
  EXPECT_EQ(packet.begin(), packet.end());
  EXPECT_EQ(packet.cbegin(), packet.cend());
}

TEST(PacketTests, WhenCalledWith_0_AllocSizeBytesIncludesDefaultDataArraySize) {
  EXPECT_EQ(Packet::alloc_size_bytes(0),
            sizeof(u32) + sizeof(PacketHeader) + sizeof(u32));
}

TEST(PacketTests, WhenCalledWith_4_AllocSizeBytesIncludesDefaultDataArraySize) {
  EXPECT_EQ(Packet::alloc_size_bytes(0),
            sizeof(u32) + sizeof(PacketHeader) + sizeof(u32));
}

TEST(PacketTests,
     WhenCalledWithAlignedSizes_AllocSizeBytesDoesNotChangeArraySize) {
  EXPECT_EQ(Packet::alloc_size_bytes(4),
            sizeof(u32) + sizeof(PacketHeader) + 4);
  EXPECT_EQ(Packet::alloc_size_bytes(12),
            sizeof(u32) + sizeof(PacketHeader) + 12);
  EXPECT_EQ(Packet::alloc_size_bytes(504),
            sizeof(u32) + sizeof(PacketHeader) + 504);
  EXPECT_EQ(Packet::alloc_size_bytes(1500),
            sizeof(u32) + sizeof(PacketHeader) + 1500);
  EXPECT_EQ(Packet::alloc_size_bytes(3396),
            sizeof(u32) + sizeof(PacketHeader) + 3396);
}

TEST(PacketTests, WhenCalledWithNOTAlignedSizes_AllocSizePadsSize) {
  EXPECT_EQ(Packet::alloc_size_bytes(2),
            sizeof(u32) + sizeof(PacketHeader) + 4);
  EXPECT_EQ(Packet::alloc_size_bytes(3),
            sizeof(u32) + sizeof(PacketHeader) + 4);
  EXPECT_EQ(Packet::alloc_size_bytes(10),
            sizeof(u32) + sizeof(PacketHeader) + 12);
  EXPECT_EQ(Packet::alloc_size_bytes(123),
            sizeof(u32) + sizeof(PacketHeader) + 124);
  EXPECT_EQ(Packet::alloc_size_bytes(1255),
            sizeof(u32) + sizeof(PacketHeader) + 1256);
  EXPECT_EQ(Packet::alloc_size_bytes(5123131),
            sizeof(u32) + sizeof(PacketHeader) + 5123132);
}

TEST(PacketTests, GivenArrayRepresentation_InPlaceInitWorks) {
  constexpr auto kNumBytes = 10;
  constexpr auto kPadding = 2;
  alignas(Packet)
      u8 raw_data[sizeof(u32) + sizeof(PacketHeader) + kNumBytes + kPadding]{
          // size
          0,
          0,
          0,
          0,

          // HEADER
          // index
          0,
          0,
          0,
          0,
          // receipt index
          0,
          0,
          0,
          0,
          // receipt flags
          0,
          0,
          0,
          0,

          // data
          1,
          2,
          3,
          4,
          5,
          6,
          7,
          8,
          9,
          10,

          // padding
          0,
          0,
      };

  ASSERT_EQ(sizeof(raw_data), Packet::alloc_size_bytes(kNumBytes));
  ASSERT_TRUE(ptr_is_aligned(raw_data, Packet::kAlignment));
  ASSERT_TRUE(ptr_is_aligned(raw_data, alignof(Packet)));

  alignas(Packet) const u8
      expected_data[sizeof(u32) + sizeof(PacketHeader) + kNumBytes + kPadding]{
          // WARNING: size, index are sensitive to endianness here
          // size
          10,
          0,
          0,
          0,

          // HEADER
          // index
          12,
          0,
          0,
          0,
          // receipt index
          5,
          0,
          0,
          0,
          // receipt flags
          0b10111111,
          0b11111111,
          0b11010101,
          0b11110011,

          // data
          1,
          2,
          3,
          4,
          5,
          6,
          7,
          8,
          9,
          10,

          // padding
          0,
          0,
      };

  const Packet* packet = Packet::unsafe_init(
      raw_data, kNumBytes, {12, 5, 0b11110011110101011111111110111111});
  EXPECT_THAT(raw_data, ContainerEq(expected_data));
  EXPECT_EQ(packet->size_bytes(), kNumBytes);
  EXPECT_EQ(packet->index(), 12);
  EXPECT_EQ(packet->receipt_index(), 5);
  EXPECT_EQ(packet->receipt_flags(), 0b11110011110101011111111110111111);
  EXPECT_THAT(*packet, ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

TEST(PacketTests, GivenInitializedPacket_IteratorsWork) {
  constexpr auto kPacketSize = 6;
  std::unique_ptr<u8[]> raw_data{new u8[Packet::alloc_size_bytes(kPacketSize)]};
  Packet* packet = Packet::unsafe_init(
      raw_data.get(), kPacketSize, {12, 5, 0b11110011110101011111111110111111});

  std::fill(std::begin(*packet), std::end(*packet), 5);
  EXPECT_TRUE(std::all_of(std::begin(*packet), std::end(*packet),
                          [](auto x) { return x == 5; }));

  std::iota(std::begin(*packet), std::end(*packet), 10);
  EXPECT_THAT(*packet, ElementsAre(10, 11, 12, 13, 14, 15));
}

TEST(PacketTests, GivenPacket_AsSpanGivesSpanOverData) {
  constexpr auto kPacketSize = 6;
  std::unique_ptr<u8[]> raw_data{new u8[Packet::alloc_size_bytes(kPacketSize)]};
  Packet* packet = Packet::unsafe_init(
      raw_data.get(), kPacketSize, {12, 5, 0b11110011110101011111111110111111});

  std::iota(std::begin(*packet), std::end(*packet), 21);
  ASSERT_THAT(*packet, ElementsAre(21, 22, 23, 24, 25, 26));

  std::span<u8> packet_span = packet->as_span();
  EXPECT_THAT(packet_span, ElementsAre(21, 22, 23, 24, 25, 26));
}

TEST(PacketTests, GivenPacket_SpanCanBeUsedToWrite) {
  constexpr auto kPacketSize = 6;
  std::unique_ptr<u8[]> raw_data{new u8[Packet::alloc_size_bytes(kPacketSize)]};
  Packet* packet = Packet::unsafe_init(
      raw_data.get(), kPacketSize, {12, 5, 0b11110011110101011111111110111111});

  std::fill(std::begin(*packet), std::end(*packet), 5);
  ASSERT_TRUE(std::all_of(std::begin(*packet), std::end(*packet),
                          [](auto x) { return x == 5; }));

  std::span<u8> packet_span = packet->as_span();
  std::iota(std::begin(packet_span), std::end(packet_span), 21);
  EXPECT_THAT(*packet, ElementsAre(21, 22, 23, 24, 25, 26));
  EXPECT_THAT(packet_span, ElementsAre(21, 22, 23, 24, 25, 26));
}