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
  EXPECT_EQ(Packet::alloc_size_bytes(0), 3 * sizeof(u32));
}

TEST(PacketTests, WhenCalledWith_4_AllocSizeBytesIncludesDefaultDataArraySize) {
  EXPECT_EQ(Packet::alloc_size_bytes(0), 3 * sizeof(u32));
}

TEST(PacketTests,
     WhenCalledWithAlignedSizes_AllocSizeBytesDoesNotChangeArraySize) {
  EXPECT_EQ(Packet::alloc_size_bytes(4), 2 * sizeof(u32) + 4);
  EXPECT_EQ(Packet::alloc_size_bytes(12), 2 * sizeof(u32) + 12);
  EXPECT_EQ(Packet::alloc_size_bytes(504), 2 * sizeof(u32) + 504);
  EXPECT_EQ(Packet::alloc_size_bytes(1500), 2 * sizeof(u32) + 1500);
  EXPECT_EQ(Packet::alloc_size_bytes(3396), 2 * sizeof(u32) + 3396);
}

TEST(PacketTests, WhenCalledWithNOTAlignedSizes_AllocSizePadsSize) {
  EXPECT_EQ(Packet::alloc_size_bytes(2), 2 * sizeof(u32) + 4);
  EXPECT_EQ(Packet::alloc_size_bytes(3), 2 * sizeof(u32) + 4);
  EXPECT_EQ(Packet::alloc_size_bytes(10), 2 * sizeof(u32) + 12);
  EXPECT_EQ(Packet::alloc_size_bytes(123), 2 * sizeof(u32) + 124);
  EXPECT_EQ(Packet::alloc_size_bytes(1255), 2 * sizeof(u32) + 1256);
  EXPECT_EQ(Packet::alloc_size_bytes(5123131), 2 * sizeof(u32) + 5123132);
}

TEST(PacketTests, GivenArrayRepresentation_InPlaceInitWorks) {
  constexpr auto kNumBytes = 10;
  constexpr auto kPadding = 2;
  alignas(Packet) u8 raw_data[2 * sizeof(u32) + kNumBytes + kPadding]{
      // size
      0,
      0,
      0,
      0,
      // index
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

  alignas(Packet)
      const u8 expected_data[2 * sizeof(u32) + kNumBytes + kPadding]{
          // WARNING: size, index are sensitive to endianness here
          // size
          10,
          0,
          0,
          0,

          // index
          12,
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

  const Packet* packet = Packet::unsafe_init(raw_data, kNumBytes, 12);
  EXPECT_THAT(raw_data, ContainerEq(expected_data));
  EXPECT_EQ(packet->size_bytes(), kNumBytes);
  EXPECT_EQ(packet->index(), 12);
  EXPECT_THAT(*packet, ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

TEST(PacketTests, GivenInitializedPacket_IteratorsWork) {
  constexpr auto kPacketSize = 6;
  std::unique_ptr<u8[]> raw_data{new u8[Packet::alloc_size_bytes(kPacketSize)]};
  Packet* packet = Packet::unsafe_init(raw_data.get(), kPacketSize, 121);

  std::fill(std::begin(*packet), std::end(*packet), 5);
  EXPECT_TRUE(std::all_of(std::begin(*packet), std::end(*packet),
                          [](auto x) { return x == 5; }));

  std::iota(std::begin(*packet), std::end(*packet), 10);
  EXPECT_THAT(*packet, ElementsAre(10, 11, 12, 13, 14, 15));
}