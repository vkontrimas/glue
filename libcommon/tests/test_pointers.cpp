#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/pointers.hpp>
#include <glue/types.hpp>

using namespace glue;

void* make_ptr(auto value) { return reinterpret_cast<void*>(0 | value); }

TEST(PointerUtilsTests, Given4ByteAlignment_ValuesCorrect) {
  EXPECT_TRUE(ptr_is_aligned(make_ptr(0b101101110111011111010100111110000), 4));
  EXPECT_TRUE(ptr_is_aligned(make_ptr(0b1101110111011111010101110110100), 4));
  EXPECT_FALSE(ptr_is_aligned(make_ptr(0b1101110111011111010100111110001), 4));
  EXPECT_FALSE(ptr_is_aligned(make_ptr(0b1101110111011111010101110110110), 4));
  EXPECT_FALSE(ptr_is_aligned(make_ptr(0b1101110111011111010100111110011), 4));
}
