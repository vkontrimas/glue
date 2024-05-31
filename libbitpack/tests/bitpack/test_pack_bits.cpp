

#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/bitpack/pack_bits.hpp>
#include <glue/types.hpp>
#include <span>

using namespace glue;
using namespace glue::bitpack;
using namespace ::testing;

TEST(BitpackBitsDeathTests, WhenPackBitsCalledWithMoreThan32Bits_Dies) {
  std::array<u32, 1> data{0};
  Packer packer{data};
  EXPECT_DEATH(pack_bits(packer, 0, 0, 33), "Assertion.*");
  EXPECT_DEATH(pack_bits_wrap(packer, 0, 0, 33), "Assertion.*");

  Unpacker unpacker{data};
  EXPECT_DEATH(pack_bits(unpacker, 0, 0, 33), "Assertion.*");
  EXPECT_DEATH(pack_bits_wrap(unpacker, 0, 0, 33), "Assertion.*");
}

TEST(BitpackBitsDeathTests, WhenPackBitsCalledWithValueBeforeBegin_Dies) {
  std::array<u32, 1> data{0};
  Packer packer{data};
  EXPECT_DEATH(pack_bits(packer, 0, 10, 5), "Assertion.*");
}

TEST(BitpackBitsDeathTests, WhenPackBitsCalledWithValueOutsideBitRange_Dies) {
  std::array<u32, 1> data{0};
  Packer packer{data};
  EXPECT_DEATH(pack_bits(packer, 33, 0, 5), "Assertion.*");
}

TEST(BitpackBitsTests, WhenCalledWithZeroBits_WritesNothing) {
  std::array<u32, 1> data{{0}};

  Packer packer{data};
  pack_bits(packer, 10, 10, 0);
  EXPECT_EQ(packer.current_bit(), 0);
  pack_bits_wrap(packer, 10, 10, 0);
  EXPECT_EQ(packer.current_bit(), 0);

  EXPECT_THAT(data, ElementsAre(0));
}

TEST(BitpackBitsTests, WhenCalledWithZeroBits_ReadsBeginValue) {
  std::array<u32, 1> data{{0}};

  Unpacker unpacker{data};
  int value_a = 0;
  pack_bits(unpacker, value_a, 10, 0);
  EXPECT_EQ(unpacker.current_bit(), 0);
  EXPECT_EQ(value_a, 10);

  int value_b = 0;
  pack_bits_wrap(unpacker, value_b, 45, 0);
  EXPECT_EQ(unpacker.current_bit(), 0);
  EXPECT_EQ(value_b, 45);
}

TEST(BitpackBitsTests, PacksValues) {
  std::array<u32, 1> data{{0}};
  Packer packer{data};

  pack_bits(packer, 5, 0, 3);
  EXPECT_THAT(data, ElementsAre(0b101'00000000000000000000000000000));

  pack_bits(packer, 15, 10, 5);
  EXPECT_THAT(data, ElementsAre(0b101'00101'000000000000000000000000));

  pack_bits_wrap(packer, 2522, 10, 5);
  EXPECT_THAT(data, ElementsAre(0b101'00101'10000'0000000000000000000));

  pack_bits_wrap(packer, 2532, 10, 12);
  EXPECT_THAT(data, ElementsAre(0b101'00101'10000'100111011010'0000000));
}

TEST(BitpackBitsTests, UnpacksValues) {
  std::array<u32, 1> data{{0b101'00101'10000'100111011010'0000000}};
  Unpacker unpacker{data};

  u8 value_ubyte = 0;
  pack_bits(unpacker, value_ubyte, 0, 3);
  EXPECT_EQ(value_ubyte, 5);

  i8 value_byte = 0;
  pack_bits(unpacker, value_byte, 10, 5);
  EXPECT_EQ(value_byte, 15);

  u32 value_uint = 0;
  pack_bits_wrap(unpacker, value_uint, 10, 5);
  EXPECT_EQ(value_uint, 26);

  i64 value_int = 0;
  pack_bits_wrap(unpacker, value_int, 10, 12);
  EXPECT_EQ(value_int, 2532);
}