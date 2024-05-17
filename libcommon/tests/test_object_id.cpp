#include <gtest/gtest.h>

#include <glue/objects/id.hpp>
#include <unordered_set>
#include <vector>

using namespace glue;
using namespace glue::objects;

TEST(ObjectID, Equality) {
  const ObjectID a{10};
  const ObjectID b{10};
  ASSERT_EQ(a, b);
}

TEST(ObjectID, NoneIsZero) {
  const ObjectID none = ObjectID::None();
  const ObjectID zero{(u32)0};
  ASSERT_EQ(none, zero);
}

TEST(ObjectID, CallValueToGetID) {
  const ObjectID id{10};
  ASSERT_EQ(id.value(), 10);
}

TEST(ObjectID, WeakCollisionSanityCheck) {
  const std::vector<const char*> names{
      "player", "cube102", "ground", "earth", "light", "camera12341u895191515",
      "camera"};

  std::unordered_set<ObjectID> ids;
  for (const auto& name : names) {
    const ObjectID id{name};
    ASSERT_TRUE(ids.find(id) == std::end(ids));
    ids.insert(id);
  }
}