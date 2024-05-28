#include <gtest/gtest.h>

#include <glue/network/address.hpp>

using namespace glue::network;

TEST(AddressTests, GivenIPv4Tuple_AssembledCorrectly) {
  IPv4Address address{127, 5, 29, 39, 13373};
  EXPECT_EQ(address.port(), 13373);
  EXPECT_EQ(address.ip(), 2131041575);
}