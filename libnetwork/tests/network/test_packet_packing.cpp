#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <glue/network/packet.hpp>
#include <numeric>

using namespace glue;
using namespace glue::network;

struct Message {
  u32 id;
  f32 pos_x, pos_y, pos_z;
  bool write_value;
  u64 value;

  constexpr friend bool operator==(const Message& a, const Message& b) {
    return a.id == b.id && a.pos_x == b.pos_x && a.pos_y == b.pos_y &&
           a.pos_z == b.pos_z && a.write_value == b.write_value &&
           a.value == b.value;
  }
};

template <bitpack::CPacker T>
inline constexpr void pack(T& packer, Message& message) {
  pack(packer, message.id);
  pack(packer, message.pos_x);
  pack(packer, message.pos_y);
  pack(packer, message.pos_z);
  if (pack(packer, message.write_value)) {
    pack(packer, message.value);
  }
}

TEST(PacketPackingTests, PackAndUnpackMessageWithExtraValueEndToEnd) {
  PacketHeader header{201, 195, 0b11100011110011011100000010111111};
  constexpr auto kPacketSize = sizeof(Message) + sizeof(PacketHeader);
  constexpr auto kPacketAllocSize = Packet::alloc_size_bytes(kPacketSize);

  std::array<u8, kPacketAllocSize> packet1_data;
  std::ranges::fill(packet1_data, 0);
  Packet* packet1 =
      Packet::unsafe_init(packet1_data.data(), kPacketSize, header);

  Message message1{};
  message1.id = 12347;
  message1.pos_x = 2.0f;
  message1.pos_y = 4.5f;
  message1.pos_z = 6.125f;
  message1.write_value = true;
  message1.value = 4321;
  packet1->pack([&message1](auto& packer) { pack(packer, message1); });

  std::array<u8, kPacketAllocSize> packet2_data;
  std::copy(std::begin(packet1_data), std::end(packet1_data),
            std::begin(packet2_data));
  std::fill_n(std::begin(packet2_data), sizeof(u32) + sizeof(PacketHeader), 0);
  Packet* packet2 = Packet::unsafe_init(packet2_data.data(), kPacketSize, {});

  Message message2{};
  packet2->unpack([&message2](auto& packer) { pack(packer, message2); });
  EXPECT_EQ(packet1->index(), packet2->index());
  EXPECT_EQ(packet1->receipt_index(), packet2->receipt_index());
  EXPECT_EQ(packet1->receipt_flags(), packet2->receipt_flags());
  EXPECT_EQ(message1, message2);
}

TEST(PacketPackingTests, PackAndUnpackMessageWithoutExtraValueEndToEnd) {
  PacketHeader header{201, 195, 0b11100011110011011100000010111111};
  constexpr auto kPacketSize = sizeof(Message) + sizeof(PacketHeader);
  constexpr auto kPacketAllocSize = Packet::alloc_size_bytes(kPacketSize);

  std::array<u8, kPacketAllocSize> packet1_data;
  std::ranges::fill(packet1_data, 0);
  Packet* packet1 =
      Packet::unsafe_init(packet1_data.data(), kPacketSize, header);

  Message message1{};
  message1.id = 12347;
  message1.pos_x = 2.0f;
  message1.pos_y = 4.5f;
  message1.pos_z = 6.125f;
  message1.write_value = false;
  message1.value = 0;
  packet1->pack([&message1](auto& packer) { pack(packer, message1); });

  std::array<u8, kPacketAllocSize> packet2_data;
  std::copy(std::begin(packet1_data), std::end(packet1_data),
            std::begin(packet2_data));
  std::fill_n(std::begin(packet2_data), sizeof(u32) + sizeof(PacketHeader), 0);
  Packet* packet2 = Packet::unsafe_init(packet2_data.data(), kPacketSize, {});

  Message message2{};
  packet2->unpack([&message2](auto& packer) { pack(packer, message2); });
  EXPECT_EQ(packet1->index(), packet2->index());
  EXPECT_EQ(packet1->receipt_index(), packet2->receipt_index());
  EXPECT_EQ(packet1->receipt_flags(), packet2->receipt_flags());
  EXPECT_EQ(message1, message2);
}

TEST(PacketPackingTests, PackMessageWithExtraValue) {
  PacketHeader header{201, 195, 0b11100011110011011100000010111111};
  constexpr auto kPacketSize = sizeof(Message) + sizeof(PacketHeader);
  constexpr auto kPacketAllocSize = Packet::alloc_size_bytes(kPacketSize);

  std::array<u8, kPacketAllocSize> packet_data;
  std::ranges::fill(packet_data, 0);
  Packet* packet = Packet::unsafe_init(packet_data.data(), kPacketSize, header);

  Message message{};
  message.id = 12347;
  message.pos_x = 2.0f;
  message.pos_y = 4.5f;
  message.pos_z = 6.125f;
  message.write_value = true;
  message.value = 432;

  packet->pack([&message](auto& packer) { pack(packer, message); });

  std::array<u8, kPacketAllocSize> expected_data{{
      // WARNING: size, index are sensitive to endianness here
      // size
      kPacketSize,
      0,
      0,
      0,

      // HEADER
      // index
      201,
      0,
      0,
      0,
      // receipt index
      195,
      0,
      0,
      0,

      // receipt flags
      0b10111111,
      0b11000000,
      0b11001101,
      0b11100011,

      // DATA
      // DATA | HEADER
      // index
      201,
      0,
      0,
      0,
      // receipt index
      195,
      0,
      0,
      0,
      // receipt flags
      0b10111111,
      0b11000000,
      0b11001101,
      0b11100011,

      // DATA | MESSAGE
      // id
      0b00111011,
      0b00110000,
      0,
      0,

      // pos_x
      0,
      0,
      0,
      0b01000000,

      // pos_y
      0,
      0,
      0b10010000,
      0b01000000,

      // pos_z
      0,
      0,
      0b11000100,
      0b01000000,

      // write_value (packs to 1 bit) + value (64 bits)
      // we're little endian rn so this bit goes in the very end
      0,
      0,
      0,
      0b1'0000000,
      0b11011000,
      0,
      0,
      0,

      0,
      0,
      0,
      0,
  }};

  EXPECT_THAT(packet_data, testing::ContainerEq(expected_data));
}

TEST(PacketPackingTests, UnpackMessageWithExtraValue) {
  constexpr auto kPacketSize = sizeof(Message) + sizeof(PacketHeader);
  constexpr auto kPacketAllocSize = Packet::alloc_size_bytes(kPacketSize);

  std::array<u8, kPacketAllocSize> packet_data{{
      // WARNING: size, index are sensitive to endianness here
      // size
      kPacketSize,
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

      // DATA
      // DATA | HEADER
      // index
      201,
      0,
      0,
      0,
      // receipt index
      195,
      0,
      0,
      0,
      // receipt flags
      0b10111111,
      0b11000000,
      0b11001101,
      0b11100011,

      // DATA | MESSAGE
      // id
      0b00111011,
      0b00110000,
      0,
      0,

      // pos_x
      0,
      0,
      0,
      0b01000000,

      // pos_y
      0,
      0,
      0b10010000,
      0b01000000,

      // pos_z
      0,
      0,
      0b11000100,
      0b01000000,

      // write_value (packs to 1 bit) + value (64 bits)
      // we're little endian rn so this bit goes in the very end
      0,
      0,
      0,
      0b1'0000000,
      0b11011000,
      0,
      0,
      0,

      0,
      0,
      0,
      0,
  }};
  Packet* packet = Packet::unsafe_init(packet_data.data(), kPacketSize, {});

  Message message{};
  message.id = 0;
  message.pos_x = 0;
  message.pos_y = 0;
  message.pos_z = 0;
  message.write_value = 0;
  message.value = 0;

  packet->unpack([&message](auto& packer) { pack(packer, message); });

  EXPECT_EQ(packet->index(), 201);
  EXPECT_EQ(packet->receipt_index(), 195);
  EXPECT_EQ(packet->receipt_flags(), 0b11100011110011011100000010111111);
  EXPECT_EQ(message.id, 12347);
  EXPECT_EQ(message.pos_x, 2.0f);
  EXPECT_EQ(message.pos_y, 4.5f);
  EXPECT_EQ(message.pos_z, 6.125f);
  EXPECT_EQ(message.write_value, true);
  EXPECT_EQ(message.value, 432);
}

TEST(PacketPackingTests, PackMessageWithoutExtraValue) {
  PacketHeader header{201, 195, 0b11100011110011011100000010111111};
  constexpr auto kPacketSize = sizeof(Message) + sizeof(PacketHeader);
  constexpr auto kPacketAllocSize = Packet::alloc_size_bytes(kPacketSize);

  std::array<u8, kPacketAllocSize> packet_data;
  std::ranges::fill(packet_data, 0);
  Packet* packet = Packet::unsafe_init(packet_data.data(), kPacketSize, header);

  Message message{};
  message.id = 12347;
  message.pos_x = 2.0f;
  message.pos_y = 4.5f;
  message.pos_z = 6.125f;
  message.write_value = false;
  message.value = 432;

  packet->pack([&message](auto& packer) { pack(packer, message); });

  std::array<u8, kPacketAllocSize> expected_data{{
      // WARNING: size, index are sensitive to endianness here
      // size
      kPacketSize,
      0,
      0,
      0,

      // HEADER
      // index
      201,
      0,
      0,
      0,
      // receipt index
      195,
      0,
      0,
      0,

      // receipt flags
      0b10111111,
      0b11000000,
      0b11001101,
      0b11100011,

      // DATA
      // DATA | HEADER
      // index
      201,
      0,
      0,
      0,
      // receipt index
      195,
      0,
      0,
      0,
      // receipt flags
      0b10111111,
      0b11000000,
      0b11001101,
      0b11100011,

      // DATA | MESSAGE
      // id
      0b00111011,
      0b00110000,
      0,
      0,

      // pos_x
      0,
      0,
      0,
      0b01000000,

      // pos_y
      0,
      0,
      0b10010000,
      0b01000000,

      // pos_z
      0,
      0,
      0b11000100,
      0b01000000,

      // write_value (packs to 1 bit)
      // we're little endian rn so this bit goes in the very end
      0,
      0,
      0,
      0b0'0000000,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
  }};

  EXPECT_THAT(packet_data, testing::ContainerEq(expected_data));
}

TEST(PacketPackingTests, UnpackMessageWithoutExtraValue) {
  constexpr auto kPacketSize = sizeof(Message) + sizeof(PacketHeader);
  constexpr auto kPacketAllocSize = Packet::alloc_size_bytes(kPacketSize);

  std::array<u8, kPacketAllocSize> packet_data{{
      // WARNING: size, index are sensitive to endianness here
      // size
      kPacketSize,
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

      // DATA
      // DATA | HEADER
      // index
      201,
      0,
      0,
      0,
      // receipt index
      195,
      0,
      0,
      0,
      // receipt flags
      0b10111111,
      0b11000000,
      0b11001101,
      0b11100011,

      // DATA | MESSAGE
      // id
      0b00111011,
      0b00110000,
      0,
      0,

      // pos_x
      0,
      0,
      0,
      0b01000000,

      // pos_y
      0,
      0,
      0b10010000,
      0b01000000,

      // pos_z
      0,
      0,
      0b11000100,
      0b01000000,

      // write_value (packs to 1 bit)
      // we're little endian rn so this bit goes in the very end
      0,
      0,
      0,
      0b0'0000000,
      0,
      0,
      0,
      0,

      0,
      0,
      0,
      0,
  }};
  Packet* packet = Packet::unsafe_init(packet_data.data(), kPacketSize, {});

  Message message{};
  message.id = 0;
  message.pos_x = 0;
  message.pos_y = 0;
  message.pos_z = 0;
  message.write_value = 0;
  message.value = 0;

  packet->unpack([&message](auto& packer) { pack(packer, message); });

  EXPECT_EQ(packet->index(), 201);
  EXPECT_EQ(packet->receipt_index(), 195);
  EXPECT_EQ(packet->receipt_flags(), 0b11100011110011011100000010111111);
  EXPECT_EQ(message.id, 12347);
  EXPECT_EQ(message.pos_x, 2.0f);
  EXPECT_EQ(message.pos_y, 4.5f);
  EXPECT_EQ(message.pos_z, 6.125f);
  EXPECT_EQ(message.write_value, false);
  EXPECT_EQ(message.value, 0);
}