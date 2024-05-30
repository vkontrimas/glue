#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/math.hpp>
#include <glue/types.hpp>

using namespace glue;

template <class T>
class SignedMathTests : public ::testing::Test {};

using SignedTypes = ::testing::Types<i8, i16, i32, i64, f32, f64>;
TYPED_TEST_SUITE(SignedMathTests, SignedTypes);

TYPED_TEST(SignedMathTests, Abs) {
  TypeParam a{-20};
  TypeParam b{12};
  EXPECT_EQ(math::abs(a), 20);
  EXPECT_EQ(math::abs(b), 12);
}
