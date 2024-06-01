#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/bitpack/pack_quantize.hpp>
#include <glue/types.hpp>

using namespace glue;
using namespace glue::bitpack;
using namespace testing;

TEST(BitpackQuantizeDeathTests, WhenCalledWithZeroBits_Dies) {
  std::array<u32, 10> data;

  Packer packer{data};
  EXPECT_DEATH(pack_quantize(packer, 2.0f, 0.0f, 4.0f, 0), "Assertion.*");
  EXPECT_DEATH(pack_quantize(packer, 2.0, 0.0, 4.0, 0), "Assertion.*");

  Packer unpacker{data};
  EXPECT_DEATH(pack_quantize(unpacker, 2.0f, 0.0f, 4.0f, 0), "Assertion.*");
  EXPECT_DEATH(pack_quantize(unpacker, 2.0, 0.0, 4.0, 0), "Assertion.*");
}

TEST(BitpackQuantizeDeathTests, WhenPackingValueOutsideRange_Dies) {
  std::array<u32, 10> data;

  Packer packer{data};
  EXPECT_DEATH(pack_quantize(packer, -1.0f, 0.0f, 4.0f, 0), "Assertion.*");
  EXPECT_DEATH(pack_quantize(packer, -1.0, 0.0, 4.0, 0), "Assertion.*");
  EXPECT_DEATH(pack_quantize(packer, 5.0f, 0.0f, 4.0f, 0), "Assertion.*");
  EXPECT_DEATH(pack_quantize(packer, 5.0, 0.0, 4.0, 0), "Assertion.*");
}

TEST(BitpackQuantizeTests, WhenPackingValueOutsideRangeWithClamping_Clamps) {
  std::array<u32, 2> data{{0, 0}};

  Packer packer{data};
  pack_quantize_clamp(packer, -1.0f, 2.0f, 5.0f, 6);
  EXPECT_THAT(data, ElementsAre(0b000000'00000000000000000000000000, 0));
  pack_quantize_clamp(packer, 10.0f, 2.0f, 5.0f, 6);
  EXPECT_THAT(data, ElementsAre(0b000000'111111'00000000000000000000, 0));

  pack_quantize_clamp(packer, 1.0, 2.0, 5.0, 10);
  EXPECT_THAT(data, ElementsAre(0b000000'111111'0000000000'0000000000, 0));
  pack_quantize_clamp(packer, 10.0, 2.0, 5.0, 10);
  EXPECT_THAT(data, ElementsAre(0b000000'111111'0000000000'1111111111, 0));
}

TEST(BitpackQuantizeTests, Packing) {
  std::array<u32, 3> data{{0, 0, 0}};

  Packer packer{data};
  pack_quantize(packer, 2.0f, 0.0f, 4.0f, 5);
  EXPECT_THAT(data, ElementsAre(0b01111'000000000000000000000000000, 0, 0));

  pack_quantize(packer, 12.57f, -20.0f, 20.0f, 20);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000, 0, 0));

  pack_quantize(packer, 20.0f, 20.0f, 81.0f, 10);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000,
                                0b000'00000000000000000000000000000, 0));

  pack_quantize(packer, 81.0f, 20.0f, 81.0f, 10);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000,
                                0b000'1111111111'0000000000000000000, 0));

  pack_quantize(packer, 2.0, 0.0, 4.0, 5);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000,
                                0b000'1111111111'01111'00000000000000, 0));

  pack_quantize(packer, 12.57, -20.0, 20.0, 20);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000,
                                0b000'1111111111'01111'11010000011100,
                                0b101010'00000000000000000000000000));

  pack_quantize(packer, 20.0, 20.0, 81.0, 10);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000,
                                0b000'1111111111'01111'11010000011100,
                                0b101010'0000000000'0000000000000000));

  pack_quantize(packer, 81.0, 20.0, 81.0, 10);
  EXPECT_THAT(data, ElementsAre(0b01111'11010000011100101010'0000000,
                                0b000'1111111111'01111'11010000011100,
                                0b101010'0000000000'1111111111'000000));
}

TEST(BitpackQuantizeTests, Unpacking) {
  std::array<u32, 3> data{{0b01111'11010000010000010111'0000000,
                           0b000'1111111111'01111'11010000011100,
                           0b101010'0000000000'1111111111'000000}};

  Unpacker unpacker{data};
  f32 val1 = 0.0f;
  pack_quantize(unpacker, val1, 0.0f, 4.0f, 5);
  EXPECT_NEAR(val1, 1.935483f, 0.001f);

  f32 val2 = 0.0f;
  pack_quantize(unpacker, val2, -20.0f, 20.0f, 20);
  EXPECT_NEAR(val2, 12.569f, 0.05f);

  f32 val3 = 0.0f;
  pack_quantize(unpacker, val3, 20.0f, 81.0f, 10);
  EXPECT_FLOAT_EQ(val3, 20.0f);

  f32 val4 = 0.0f;
  pack_quantize(unpacker, val4, 20.0f, 81.0f, 10);
  EXPECT_FLOAT_EQ(val4, 81.0f);

  f64 val5 = 0.0;
  pack_quantize(unpacker, val5, 0.0, 4.0, 5);
  EXPECT_NEAR(val1, 1.935483, 0.001);

  f64 val6 = 0.0;
  pack_quantize(unpacker, val6, -20.0, 20.0, 20);
  EXPECT_NEAR(val2, 12.569, 0.05);

  f64 val7 = 0.0;
  pack_quantize(unpacker, val7, 20.0, 81.0, 10);
  EXPECT_DOUBLE_EQ(val7, 20.0);

  f64 val8 = 0.0;
  pack_quantize(unpacker, val8, 20.0, 81.0, 10);
  EXPECT_DOUBLE_EQ(val8, 81.0);
}