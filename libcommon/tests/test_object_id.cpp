#include <gtest/gtest.h>

#include <glue/objects/id.hpp>

using namespace glue;

TEST(ObjectID, Equality) {
  const ObjectID a{10};
  const ObjectID b{10};
  ASSERT_EQ(a, b);
}

TEST(ObjectID, NoneIsZero) {
  const ObjectID none = ObjectID::None();
  const ObjectID zero{0};
  ASSERT_EQ(none, zero);
}

TEST(ObjectID, CallValueToGetID) {
  const ObjectID id{10};
  ASSERT_EQ(id.value(), 10);
}