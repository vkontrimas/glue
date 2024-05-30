#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/bitpack/bitpack.hpp>
#include <glue/types.hpp>
#include <span>

using namespace glue;
using namespace glue::bitpack;

template <class T>
struct TestData final {
  std::vector<T> values{};
  std::vector<u32> raw{};
};

template <class T>
constexpr TestData<T> test_data();

template <>
constexpr TestData<bool> test_data() {
  return {{true, false, true, true, false, false, true, false, true},
          {0b10110010'10000000'00000000'00000000}};
}

template <>
constexpr TestData<u8> test_data() {
  return {{1, 2, 10, 128, 255}, {0x0102'0a80, 0xff00'0000}};
}

template <>
constexpr TestData<u16> test_data() {
  return {{1, 10, 255, 2048, 65535}, {0x0001'000a, 0x00ff'0800, 0xffff'0000}};
}

template <>
constexpr TestData<u32> test_data() {
  return {{1, 10, 255, 2048, 65535, 5123512, 4294967295},
          {1, 10, 255, 2048, 65535, 5123512, 4294967295}};
}

template <class T>
class BitpackTests : public ::testing::Test {};

using BitpackTypes = ::testing::Types<u8, u16, bool>;
TYPED_TEST_SUITE(BitpackTests, BitpackTypes);

TYPED_TEST(BitpackTests, Packs) {
  const auto data = test_data<TypeParam>();

  std::vector<u32> buffer;
  buffer.resize(data.raw.size(), 0);
  Packer packer{buffer};

  for (const auto& val : data.values) {
    pack(packer, val);
  }

  EXPECT_THAT(buffer, ::testing::ContainerEq(data.raw));
}

TYPED_TEST(BitpackTests, Unpacks) {
  auto data = test_data<TypeParam>();
  Unpacker unpacker{data.raw};

  std::vector<TypeParam> result;
  result.reserve(data.values.size());
  for (const auto& _ : data.values) {
    TypeParam val{};
    pack(unpacker, val);
    result.emplace_back(val);
  }

  EXPECT_THAT(result, ::testing::ContainerEq(data.values));
}
