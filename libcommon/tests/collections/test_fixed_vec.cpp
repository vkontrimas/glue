#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <concepts>
#include <glue/collections/fixed_vec.hpp>
#include <glue/types.hpp>

using namespace glue;
using namespace testing;

TEST(FixedVecTests, CapacityAccurate) {
  FixedVec<int, 10> vector;
  EXPECT_EQ(vector.capacity(), 10);
}

TEST(FixedVecTests, WhenDefaultConstructed_SizeIsZero) {
  FixedVec<int, 200> vector;
  EXPECT_EQ(vector.size(), 0);
}

TEST(FixedVecTests, WhenDefaultConstructed_DoesNotConstructContents) {
  static int ctor_calls = 0;
  struct Object final {
    Object() { ctor_calls++; }
  };

  FixedVec<Object, 5> vector;
  EXPECT_EQ(ctor_calls, 0);
}

TEST(FixedVecTests, WhenDestroyed_DestructorsAreCalled) {
  static int num_dtor_calls = 0;
  struct Object final {
    ~Object() { num_dtor_calls++; }
  };

  {
    FixedVec<Object, 5> vec{{}, {}, {}};
    EXPECT_EQ(vec.size(), 3);
    num_dtor_calls = 0;  // remove copy init
  }
  EXPECT_EQ(num_dtor_calls, 3);
}

TEST(FixedVecTests, WhenPushBackCalled_PushesItemToTheBackOfTheArray) {
  FixedVec<int, 5> vector;
  EXPECT_EQ(vector.size(), 0);

  vector.push_back(10);
  EXPECT_EQ(vector.size(), 1);
  EXPECT_EQ(vector[0], 10);

  vector.push_back(321);
  EXPECT_EQ(vector.size(), 2);
  EXPECT_EQ(vector[1], 321);
}

TEST(FixedVecDeathTests, GivenFullVector_WhenPushBackCalled_Aborts) {
  FixedVec<int, 2> vector;
  vector.push_back(10);
  vector.push_back(20);
  EXPECT_EQ(vector.size(), 2);

  EXPECT_DEATH(vector.push_back(11), "Assertion.*failed");
}

TEST(FixedVecTests,
     WhenConstructedWithInitializerList_ContainsInitializerListItems) {
  FixedVec<int, 5> vector{{10, 9, 8, 7}};
  EXPECT_THAT(vector, ElementsAre(10, 9, 8, 7));
}

TEST(FixedVecTests, GivenEmptyVector_EmptyReturnsTrue) {
  FixedVec<int, 5> vector{};
  EXPECT_TRUE(vector.empty());
}

TEST(FixedVecTests, GivenFullVector_FullReturnsTrue) {
  FixedVec<int, 4> vector{3, 4, 5, 6};
  EXPECT_TRUE(vector.full());
}

TEST(FixedVecTests, GivenNonFullVector_FullReturnsFalse) {
  FixedVec<int, 4> vector{3, 5, 6};
  EXPECT_FALSE(vector.full());
}

TEST(FixedVecTests, GivenVectorWithElements_EmptyReturnsFalse) {
  FixedVec<int, 5> vector{{10, 9, 8}};
  EXPECT_FALSE(vector.empty());
}

TEST(FixedVecTests, IndexOperatorWorks) {
  FixedVec<int, 5> vector{{10, 9, 8, 7}};
  EXPECT_EQ(vector[0], 10);
  EXPECT_EQ(vector[3], 7);
  EXPECT_EQ(vector[2], 8);
  EXPECT_EQ(vector[1], 9);
}

TEST(FixedVecTests, WhenPopBackCalled_RemovesLastItemInList) {
  FixedVec<int, 5> vector{{10, 9, 8, 7}};
  EXPECT_THAT(vector, ElementsAre(10, 9, 8, 7));
  vector.pop_back();
  EXPECT_THAT(vector, ElementsAre(10, 9, 8));
  vector.pop_back();
  EXPECT_THAT(vector, ElementsAre(10, 9));
}

TEST(FixedVecTests, WhenPopBackCalled_CallsDestructorOnItem) {
  static int num_dtor_calls = 0;
  struct Object final {
    ~Object() { num_dtor_calls++; }
  };

  FixedVec<Object, 5> vector;
  vector.emplace_back(Object{});
  EXPECT_EQ(num_dtor_calls, 1);  // from copy
  vector.pop_back();
  EXPECT_EQ(num_dtor_calls, 2);
}

TEST(FixedVecDeathTests, GivenEmptyVector_WhenPopBackCalled_Aborts) {
  FixedVec<int, 5> vector;
  EXPECT_DEATH(vector.pop_back(), "Assertion.*failed");
}

TEST(FixedVecTests, GivenEmptyVector_WhenClearCalled_NothingHappens) {
  FixedVec<int, 5> vector;
  vector.clear();
}

TEST(FixedVecTests, GivenEmptyVector_BeginIsEnd) {
  FixedVec<int, 10> vector;
  EXPECT_EQ(std::begin(vector), std::end(vector));
}

TEST(FixedVecTests, IteratorWorks) {
  FixedVec<int, 10> vector{{10, 9, 8, 7, 6, 5, 4}};

  std::vector<int> out;
  for (auto it = std::begin(vector); it != std::end(vector); ++it) {
    out.push_back(*it);
  }

  EXPECT_THAT(out, ElementsAre(10, 9, 8, 7, 6, 5, 4));
}

TEST(FixedVecTests, WhenClearCalled_VectorIsEmpty) {
  FixedVec<int, 5> vector{10, 5, 1, 2};
  EXPECT_THAT(vector, ElementsAre(10, 5, 1, 2));
  EXPECT_EQ(vector.size(), 4);
  vector.clear();
  EXPECT_THAT(vector, IsEmpty());
  EXPECT_EQ(vector.size(), 0);
}

TEST(FixedVecTests, WhenClearCalled_DestructorsAreCalled) {
  static int num_dtor_calls = 0;
  struct Object final {
    ~Object() { num_dtor_calls++; }
  };

  FixedVec<Object, 5> vector{{}, {}, {}};
  num_dtor_calls = 0;  // remove dtor calls caused by copy

  vector.clear();
  EXPECT_EQ(num_dtor_calls, 3);
}

TEST(FixedVecTests, WhenConstructedWithCountAndValue_InitializesCountElements) {
  struct Item {
    int value;

    bool operator!=(const Item& other) { return !(*this == other); }
    bool operator==(const Item& other) { return value == other.value; }
  };

  FixedVec<Item, 10> vector{5, {42}};
  EXPECT_EQ(vector.size(), 5);
  int count = 0;
  for (auto val : vector) {
    EXPECT_EQ(val.value, 42);
    ++count;
  }
  EXPECT_EQ(count, 5);
}