#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <glue/bitpack/packer.hpp>

using namespace glue;
using namespace glue::bitpack;
using namespace ::testing;

template <std::derived_from<glue::bitpack::detail::BasePacker> T>
class BasePackerTests : public ::testing::Test {};

using PackerTypes = ::testing::Types<Packer, Unpacker>;
TYPED_TEST_SUITE(BasePackerTests, PackerTypes);

TYPED_TEST(BasePackerTests, WhenDefaultConstructed_ZeroCapacityAndSizes) {
  TypeParam packer;
  EXPECT_EQ(packer.capacity(), 0);
  EXPECT_EQ(packer.capacity_bits(), 0);
  EXPECT_EQ(packer.current(), 0);
  EXPECT_EQ(packer.current_bit(), 0);
}

TYPED_TEST(BasePackerTests, WhenBackedByArray_CapacityDerivedFromArray) {
  std::array<u32, 200> data;

  TypeParam packer{data};
  EXPECT_EQ(packer.capacity(), 200);
  EXPECT_EQ(packer.capacity_bits(), 6400);
  EXPECT_EQ(packer.current(), 0);
  EXPECT_EQ(packer.current_bit(), 0);
}

TEST(PackerTests, WriteBitsCorrectly) {
  std::array<u32, 2> data{{0, 0}};
  Packer packer{data};

  packer.write_bits(0b0101'0110'1111'0101, 16);
  EXPECT_EQ(packer.current(), 0);
  EXPECT_EQ(packer.current_bit(), 16);

  packer.write_bits(0b1111'0101'11, 10);
  EXPECT_EQ(packer.current(), 0);
  EXPECT_EQ(packer.current_bit(), 26);

  packer.write_bits(0x0, 32);
  EXPECT_EQ(packer.current(), 1);
  EXPECT_EQ(packer.current_bit(), 58);

  packer.write_bits(0b11, 2);
  EXPECT_EQ(packer.current(), 1);
  EXPECT_EQ(packer.current_bit(), 60);

  EXPECT_THAT(data, ElementsAre(0b0101'0110'1111'0101'1111'0101'1100'0000,
                                0b0000'0000'0000'0000'0000'0000'0011'0000));
}

TEST(PackerTests, WriteExactly32BitsAligned) {
  std::array<u32, 1> data{{0}};
  Packer packer{data};
  EXPECT_EQ(packer.capacity(), 1);
  EXPECT_EQ(packer.capacity_bits(), 32);
  packer.write_bits(0xabcdef12, 32);
  EXPECT_THAT(data, ElementsAre(0xabcdef12));
}

TEST(PackerTests, Write16BitsThenWrite32Bits) {
  std::array<u32, 2> data{{0, 0}};
  Packer packer{data};
  EXPECT_EQ(packer.capacity(), 2);
  EXPECT_EQ(packer.capacity_bits(), 64);
  packer.write_bits(0xabab, 16);
  packer.write_bits(0xf5f5f5f5, 32);
  EXPECT_THAT(data, ElementsAre(0xababf5f5, 0xf5f50000));
}

TEST(UnpackerTests, ReadBitsCorrectly) {
  std::array<u32, 2> data{{0b0101'0110'1111'0101'1111'0101'1100'0000,
                           0b0000'0000'0000'0000'0000'0000'0011'0000}};
  Unpacker packer{data};

  EXPECT_EQ(packer.read_bits(16), 0b0101'0110'1111'0101);
  EXPECT_EQ(packer.current(), 0);
  EXPECT_EQ(packer.current_bit(), 16);

  EXPECT_EQ(packer.read_bits(10), 0b1111'0101'11);
  EXPECT_EQ(packer.current(), 0);
  EXPECT_EQ(packer.current_bit(), 26);

  EXPECT_EQ(packer.read_bits(32), 0);
  EXPECT_EQ(packer.current(), 1);
  EXPECT_EQ(packer.current_bit(), 58);

  EXPECT_EQ(packer.read_bits(2), 0b11);
  EXPECT_EQ(packer.current(), 1);
  EXPECT_EQ(packer.current_bit(), 60);
}

TEST(UnpackerTests, ReadExactly32BitsAligned) {
  std::array<u32, 1> data{{0xabab124f}};
  Unpacker packer{data};
  EXPECT_EQ(packer.capacity(), 1);
  EXPECT_EQ(packer.capacity_bits(), 32);
  u32 value = packer.read_bits(32);
  EXPECT_EQ(value, 0xabab124f);
}

TEST(UnpackerTests, Read16BitsThenRead32Bits) {
  std::array<u32, 2> data{{0xababf5f5, 0xf5f50000}};
  Unpacker packer{data};
  EXPECT_EQ(packer.capacity(), 2);
  EXPECT_EQ(packer.capacity_bits(), 64);
  u32 a = packer.read_bits(16);
  u32 b = packer.read_bits(32);
  EXPECT_EQ(a, 0xabab);
  EXPECT_EQ(b, 0xf5f5f5f5);
}

TEST(PackerDeathTests, WhenWritingMoreThan32Bits_Die) {
  std::array<u32, 24> data;
  Packer packer{data};
  EXPECT_DEATH(packer.write_bits(0b000, 33), "Assertion.*");
}

TEST(PackerDeathTests, WhenWritingOutOfBounds_Die) {
  std::array<u32, 2> data;
  Packer packer{data};
  packer.write_bits(0x0, 32);
  packer.write_bits(0x0, 16);
  EXPECT_DEATH(packer.write_bits(0x0, 32), "Assertion.*");
}

TEST(UnpackerDeathTests, WhenReadingMoreThan32Bits_Die) {
  std::array<u32, 24> data;
  Unpacker packer{data};
  EXPECT_DEATH(packer.read_bits(33), "Assertion.*");
}

TEST(UnpackerDeathTests, WhenReadingOutOfBounds_Die) {
  std::array<u32, 2> data;
  Unpacker packer{data};
  packer.read_bits(32);
  packer.read_bits(16);
  EXPECT_DEATH(packer.read_bits(32), "Assertion.*");
}