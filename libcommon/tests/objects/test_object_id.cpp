#include <gtest/gtest.h>

#include <glue/objects/id.hpp>
#include <unordered_set>
#include <vector>

using namespace glue;
using namespace glue::objects;

TEST(ObjectID, Equality) {
  const ObjectID a{"foo"};
  const ObjectID b{"foo"};
  ASSERT_EQ(a, b);
}

TEST(ObjectID, Inequality) {
  const ObjectID a{"foo"};
  const ObjectID b{"bar"};
  ASSERT_NE(a, b);
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

TEST(ObjectID, RecoverNameFromID) {
  const std::vector<const char*> expected{
      "player", "cube102", "ground", "earth", "light", "camera12341u895191515",
      "camera"};

  for (int i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(ObjectID{expected[i]}.retrieve_name(), expected[i]);
  }
}