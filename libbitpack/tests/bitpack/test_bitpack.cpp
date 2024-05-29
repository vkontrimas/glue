#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/types.hpp>
#include <span>

using namespace glue;
using namespace testing;

struct Packer {
  std::span<f32> values;
  int index = 0;
};

struct Unpacker {
  std::span<f32> values;
  int index = 0;
};

void pack_raw(Packer& packer, f32& value) {
  packer.values[packer.index] = value;
  packer.index++;
}

void pack_raw(Unpacker& unpacker, f32& value) {
  value = unpacker.values[unpacker.index];
  unpacker.index++;
}

struct Position {
  f32 x, y, z;
};

void pack(auto& packer, Position& position) {
  pack_raw(packer, position.x);
  pack_raw(packer, position.y);
  pack_raw(packer, position.z);
}

TEST(BitpackTests, Prototype) {
  Position position{1, 2, 3};

  std::array<f32, 4> data{{0, 0, 0, 0}};
  Position pos{1, 2, 3};

  Packer packer{data, 0};
  pack(packer, pos);
  EXPECT_EQ(pos.x, 1);
  EXPECT_EQ(pos.y, 2);
  EXPECT_EQ(pos.z, 3);
  EXPECT_THAT(data, ElementsAre(1, 2, 3, 0));

  Position pos2{0, 0, 0};
  Unpacker unpacker{data, 0};
  pack(unpacker, pos2);
  EXPECT_EQ(pos2.x, 1);
  EXPECT_EQ(pos2.y, 2);
  EXPECT_EQ(pos2.z, 3);
  EXPECT_THAT(data, ElementsAre(1, 2, 3, 0));
}