#include <gtest/gtest.h>

#include <glue/network/address.hpp>

using namespace glue::network;

TEST(IPv4AddressTests, GivenIPv4Tuple_AssembledCorrectly) {
  IPv4Address address{127, 5, 29, 39, 13373};
  EXPECT_EQ(address.port(), 13373);
  EXPECT_EQ(address.ip(), 2131041575);
}

TEST(IPv4AddressTests, GivenTwoIdenticalAddresses_EqualityOperatorsWork) {
  IPv4Address a{123, 123, 123, 123, 3424};
  IPv4Address b{123, 123, 123, 123, 3424};
  IPv4Address c{123, 5, 123, 6, 3424};

  EXPECT_TRUE(a == b);
  EXPECT_TRUE(b == a);
  EXPECT_FALSE(a == c);

  EXPECT_FALSE(a != b);
  EXPECT_FALSE(b != a);
  EXPECT_TRUE(a != c);
}
}