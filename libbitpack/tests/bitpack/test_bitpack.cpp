#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/types.hpp>
#include <span>

using namespace glue;
using namespace testing;

struct Packer {
  std::span<u8> values;
  int index = 0;
};

struct Unpacker {
  std::span<u8> values;
  int index = 0;
};

void pack(Packer& packer, const f32& value) {
  // dark magic, not robust or smart. prototype shit
  const u8* bytes = reinterpret_cast<const u8*>(&value);
  std::copy(bytes, bytes + 4, std::begin(packer.values) + packer.index);
  packer.index += 4;
}

void pack(Unpacker& unpacker, f32& value) {
  // dark magic, not robust or smart. prototype shit
  std::copy(std::begin(unpacker.values) + unpacker.index,
            std::begin(unpacker.values) + unpacker.index + 4,
            reinterpret_cast<u8*>(&value));
  unpacker.index += 4;
}

void pack_quantize(Packer& packer, const f32& value, f32 min, f32 max) {
  // probably sucks stability and precision wise
  // again, prototype shit
  const f32 length = max - min;
  const f32 val = 255.0f * ((value - min) / length);
  const u8 byte = static_cast<u8>(val);
  packer.values[packer.index] = byte;
  ++packer.index;
}

void pack_quantize(Unpacker& packer, f32& value, f32 min, f32 max) {
  // probably sucks stability and precision wise
  // again, prototype shit
  const u8 byte = packer.values[packer.index];
  const f32 length = max - min;
  value = min + ((byte * length) / 255.0f);
  ++packer.index;
}

struct Position {
  f32 x, y, z;
};

void pack(auto& packer, Position& position) {
  pack(packer, position.x);
  pack_quantize(packer, position.y, 1.0f, 4.0f);  // switching is easy!!!!
  pack(packer, position.z);
}

TEST(BitpackTests, Prototype) {
  Position position{1, 2, 3};

  std::array<u8, 200> data;
  std::ranges::fill(data, 0);

  Position pos{1, 2, 3};

  Packer packer{data, 0};
  pack(packer, pos);

  Position pos2{0, 0, 0};
  Unpacker unpacker{data, 0};
  pack(unpacker, pos2);

  LOG(INFO) << "Pre: (" << pos.x << ", " << pos.y << ", " << pos.z << ")"
            << " Post: (" << pos2.x << ", " << pos2.y << ", " << pos2.z << ")";
}