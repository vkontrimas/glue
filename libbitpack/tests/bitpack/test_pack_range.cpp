
#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/bitpack/pack_range.hpp>
#include <glue/types.hpp>
#include <span>

using namespace glue;
using namespace glue::bitpack;
using namespace ::testing;

TEST(BitpackRangeDeathTests, WhenMoreThan32BitsNeeded_Dies) {
  u64 value = 0;
  std::array<u32, 10> data;
  Packer packer{data};
  Packer unpacker{data};

  EXPECT_DEATH(pack_range(packer, value, 0ull, 2131241257812681275ull),
               "Assertion.*");
  EXPECT_DEATH(
      pack_range(unpacker, value, 0ull, 11241247865178651237ull),
      "Assertion.*");

  i64 signed_value;
  EXPECT_DEATH(pack_range(packer, signed_value, -189751571892375ll,
                                  123512512351235ll),
               "Assertion.*");
  EXPECT_DEATH(pack_range(unpacker, signed_value, -1247829752892372ll,
                                  1512356782627823ll),
               "Assertion.*");
}

TEST(BitpackRangeDeathTests, WhenBeforeBegin_Dies) {
  i64 value = -10;
  std::array<u32, 10> data;
  Packer packer{data};
  Packer unpacker{data};

  EXPECT_DEATH(pack_range(packer, value, 0, 10), "Assertion.*");
}

TEST(BitpackRangeDeathTests, WhenAtEnd_Dies) {
  i64 value = 10;
  std::array<u32, 10> data;
  Packer packer{data};
  Packer unpacker{data};

  EXPECT_DEATH(pack_range(packer, value, 0, 10), "Assertion.*");
}

TEST(BitpackRangeDeathTests, WhenAfterEnd_Dies) {
  i64 value = 2352;
  std::array<u32, 10> data;
  Packer packer{data};
  Packer unpacker{data};

  EXPECT_DEATH(pack_range(packer, value, 0, 10), "Assertion.*");
}

TEST(BitpackRangeTests, PackValues) {
  std::array<u32, 2> data{{0, 0}};
  Packer packer{data};
  u8 value_byte = 211;
  pack_range(packer, value_byte, 200, 240);
  EXPECT_THAT(data, ElementsAre(0b001011'00000000000000000000000000, 0));

  u16 value_short = 5187;
  pack_range(packer, value_short, 5000, 6024);
  EXPECT_THAT(data, ElementsAre(0b001011'0010111011'0000000000000000, 0));

  i32 value_int = -1519;
  pack_range(packer, value_int, -2000, 2000);
  EXPECT_THAT(data, ElementsAre(0b001011'0010111011'000111100001'0000, 0));

  i64 value_long = -1000000000000000004;
  pack_range(packer, value_long, -1000000000000000015,
                     -1000000000000000000);
  EXPECT_THAT(data, ElementsAre(0b001011'0010111011'000111100001'1011, 0));
}

TEST(BitpackRangeTests, UnpackValues) {
  std::array<u32, 2> data{{0b001011'0010111011'000111100001'1011, 0}};
  Unpacker unpacker{data};
  u8 value_byte{0};
  pack_range(unpacker, value_byte, 200, 240);
  EXPECT_EQ(value_byte, 211);

  u16 value_short{0};
  pack_range(unpacker, value_short, 5000, 6024);
  EXPECT_EQ(value_short, 5187);

  i32 value_int{0};
  pack_range(unpacker, value_int, -2000, 2000);
  EXPECT_EQ(value_int, -1519);

  i64 value_long{0};
  pack_range(unpacker, value_long, -1000000000000000015,
                     -1000000000000000000);
  EXPECT_EQ(value_long, -1000000000000000004);
}