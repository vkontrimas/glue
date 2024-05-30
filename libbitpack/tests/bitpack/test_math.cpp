
#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/bitpack/math.hpp>
#include <glue/types.hpp>
#include <span>

using namespace glue;
using namespace glue::bitpack;

TEST(MathTests, CorrectNumberOfBitsZeroToN) {
  EXPECT_EQ(bits_to_represent_n_values(0), 0);  // nothing to send...
  EXPECT_EQ(bits_to_represent_n_values(1), 0);  // value is always the same...
  EXPECT_EQ(bits_to_represent_n_values(2), 1);
  EXPECT_EQ(bits_to_represent_n_values(4), 2);
  EXPECT_EQ(bits_to_represent_n_values(8), 3);
  EXPECT_EQ(bits_to_represent_n_values(9), 4);
  EXPECT_EQ(bits_to_represent_n_values(16), 4);
  EXPECT_EQ(bits_to_represent_n_values(231), 8);
  EXPECT_EQ(bits_to_represent_n_values(1241), 11);
  EXPECT_EQ(bits_to_represent_n_values(2351), 12);
  EXPECT_EQ(bits_to_represent_n_values(235141243), 28);
}

TEST(MathTests, CorrectNumberOfBitsForRange) {
  EXPECT_EQ(bits_needed_for_range(0, 0), 0);  // nothing to send
  EXPECT_EQ(bits_needed_for_range(0, 1),
            0);  // single unique value, always same
  EXPECT_EQ(bits_needed_for_range(-252, 121), 9);
  EXPECT_EQ(bits_needed_for_range(-12512512572ll, 2512ll), 34);
  EXPECT_EQ(bits_needed_for_range(-53125123512, 12512512572), 36);
}

TEST(MathTests, BackwardsRangeWorks) {
  EXPECT_EQ(bits_needed_for_range(2512ll, -12512512572ll), 34);
  EXPECT_EQ(bits_needed_for_range(12512512572, -53125123512), 36);
}