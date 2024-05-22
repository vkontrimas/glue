#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/collections/fixed_circular_buffer.hpp>
#include <glue/types.hpp>

using namespace glue;
using namespace testing;

TEST(FixedCircularBufferTests, DefaultConstructs) {
  FixedCircularBuffer<int, 40> buffer;
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_TRUE(buffer.empty());
}

TEST(FixedCircularBufferTests,
     WhenDefaultConstructed_DoesNotConstructContents) {
  static int num_ctor_calls = 0;
  struct Object final {
    Object() { num_ctor_calls++; }
  };

  FixedCircularBuffer<Object, 40> buffer;
  EXPECT_EQ(num_ctor_calls, 0);
}

TEST(FixedCircularBufferDeathTests,
     GivenInitializerList_ConstructBufferContainsItems) {
  FixedCircularBuffer<int, 40> buffer{10, 5, 9};
  EXPECT_EQ(buffer.size(), 3);
  EXPECT_FALSE(buffer.empty());
  EXPECT_THAT(buffer, ElementsAre(10, 5, 9));
}

TEST(FixedCircularBufferTests, GivenFullBuffer_FullIsTrue) {
  FixedCircularBuffer<int, 4> buffer{1, 2, 3, 4};
  EXPECT_TRUE(buffer.full());
}

TEST(FixedCircularBufferTests, GivenNonFullBuffer_FullIsFalse) {
  FixedCircularBuffer<int, 4> buffer{1, 2, 3};
  EXPECT_FALSE(buffer.full());
}

TEST(FixedCircularBufferDeathTests,
     GivenInitializerList_WhenListIsTooBig_Aborts) {
  auto construct = []() { FixedCircularBuffer<int, 2> buffer{10, 5, 9}; };
  EXPECT_DEATH(construct(), "Assertion.*failed");
}

TEST(FixedCircularBufferTests,
     GivenCountAndValue_ConstructBufferContainingCountCopies) {
  struct Object {
    int value;
  };
  FixedCircularBuffer<Object, 40> buffer{4, {42}};
  EXPECT_EQ(buffer.size(), 4);
  EXPECT_FALSE(buffer.empty());

  for (auto val : buffer) {
    EXPECT_EQ(val.value, 42);
  }
}

TEST(FixedCircularBufferDeathTests,
     GivenCountAndValue_WhenCountExceedsCapacity_Aborts) {
  struct Object {
    int value;
  };

  auto construct = []() {
    FixedCircularBuffer<Object, 10> buffer{20, Object{42}};
  };
  EXPECT_DEATH(construct(), "Assertion.*failed");
}

TEST(FixedCircularBufferTests, IndexingWorks) {
  FixedCircularBuffer<int, 10> buffer{10, 9, 8};
  EXPECT_EQ(buffer[0], 10);
  EXPECT_EQ(buffer[1], 9);
  EXPECT_EQ(buffer[2], 8);
}

TEST(FixedCircularBufferTests, BeginIsEndWhenEmpty) {
  FixedCircularBuffer<int, 10> buffer{};
  EXPECT_EQ(std::begin(buffer), std::end(buffer));
}

TEST(FixedCircularBufferTests, IteratorWorks) {
  struct Object {
    int value;
  };
  FixedCircularBuffer<Object, 10> buffer{10, {42}};
  EXPECT_EQ(buffer.size(), 10);

  std::vector<int> values;
  for (auto it = std::begin(buffer); it != std::end(buffer); ++it) {
    values.push_back(it->value);
  }

  EXPECT_THAT(values, ElementsAre(42, 42, 42, 42, 42, 42, 42, 42, 42, 42));
}

TEST(FixedCircularBufferTests, WhenBufferNotFull_IteratorWorks) {
  struct Object {
    int value;
  };
  FixedCircularBuffer<Object, 10> buffer{4, {42}};
  EXPECT_EQ(buffer.size(), 4);

  std::vector<int> values;
  for (auto it = std::begin(buffer); it != std::end(buffer); ++it) {
    values.push_back(it->value);
  }

  EXPECT_THAT(values, ElementsAre(42, 42, 42, 42));
}

TEST(FixedCircularBufferTests, WhenEndIsNotAtEndOfBuffer_IteratorWorks) {
  struct Object {
    int value;
  };
  FixedCircularBuffer<Object, 10> buffer{4, {42}};
  EXPECT_EQ(buffer.size(), 4);

  auto end = std::begin(buffer);
  end++;
  end++;

  std::vector<int> values;
  for (auto it = std::begin(buffer); it != end; ++it) {
    values.push_back(it->value);
  }

  EXPECT_THAT(values, ElementsAre(42, 42));
}

TEST(FixedCircularBufferTests, WhenIteratorNotAtBegin_IteratorWorks) {
  FixedCircularBuffer<int, 10> buffer{10, 12, 9, 2};
  EXPECT_EQ(buffer.size(), 4);

  auto begin = std::begin(buffer);
  ++begin;

  std::vector<int> values;
  for (auto it = begin; it != std::end(buffer); ++it) {
    values.push_back(*it);
  }

  EXPECT_THAT(values, ElementsAre(12, 9, 2));
}

TEST(FixedCircularBufferTests, ClearEmptiesBuffer) {
  FixedCircularBuffer<int, 5> buffer{4, 5, 3};
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3));
  buffer.clear();
  EXPECT_THAT(buffer, IsEmpty());
}

TEST(FixedCircularBufferTests, PushBack) {
  FixedCircularBuffer<int, 5> buffer{4, 5, 3};
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3));
  buffer.push_back(10);
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3, 10));
  buffer.push_back(5);
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3, 10, 5));
  buffer.pop_back();
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3, 10));
  buffer.pop_front();
  EXPECT_THAT(buffer, ElementsAre(5, 3, 10));
  buffer.push_back(52);
  EXPECT_THAT(buffer, ElementsAre(5, 3, 10, 52));
}

TEST(FixedCircularBufferTests, PopBack) {
  FixedCircularBuffer<int, 5> buffer{4, 5, 3};
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3));
  buffer.pop_back();
  EXPECT_THAT(buffer, ElementsAre(4, 5));
  buffer.pop_back();
  EXPECT_THAT(buffer, ElementsAre(4));
  buffer.push_back(11);
  EXPECT_THAT(buffer, ElementsAre(4, 11));
  buffer.push_back(-1);
  EXPECT_THAT(buffer, ElementsAre(4, 11, -1));
  buffer.pop_back();
  EXPECT_THAT(buffer, ElementsAre(4, 11));
  buffer.pop_back();
  EXPECT_THAT(buffer, ElementsAre(4));
  buffer.pop_back();
  EXPECT_THAT(buffer, IsEmpty());
}

TEST(FixedCircularBufferTests, PopFront) {
  FixedCircularBuffer<int, 5> buffer{4, 5, 3};
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3));
  buffer.pop_front();
  EXPECT_THAT(buffer, ElementsAre(5, 3));
  buffer.pop_front();
  EXPECT_THAT(buffer, ElementsAre(3));
  buffer.push_back(11);
  EXPECT_THAT(buffer, ElementsAre(3, 11));
  buffer.push_back(-1);
  EXPECT_THAT(buffer, ElementsAre(3, 11, -1));
  buffer.pop_front();
  EXPECT_THAT(buffer, ElementsAre(11, -1));
  buffer.pop_front();
  EXPECT_THAT(buffer, ElementsAre(-1));
  buffer.pop_front();
  EXPECT_THAT(buffer, IsEmpty());
}

TEST(FixedCircularBufferTests, PopBackCallsDestructor) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  FixedCircularBuffer<Object, 5> buffer{4, {}};
  num_dtor_calls = 0;  // remove copy calls
  buffer.pop_back();
  EXPECT_EQ(num_dtor_calls, 1);
  buffer.pop_back();
  EXPECT_EQ(num_dtor_calls, 2);
  buffer.pop_back();
  EXPECT_EQ(num_dtor_calls, 3);
  buffer.pop_back();
  EXPECT_EQ(num_dtor_calls, 4);
}

TEST(FixedCircularBufferTests, PopFrontCallsDestructor) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  FixedCircularBuffer<Object, 5> buffer{4, {}};
  num_dtor_calls = 0;  // remove copy calls
  buffer.pop_front();
  EXPECT_EQ(num_dtor_calls, 1);
  buffer.pop_front();
  EXPECT_EQ(num_dtor_calls, 2);
  buffer.pop_front();
  EXPECT_EQ(num_dtor_calls, 3);
  buffer.pop_front();
  EXPECT_EQ(num_dtor_calls, 4);
}

TEST(FixedCircularBufferTests, ClearCallsDestructors) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  FixedCircularBuffer<Object, 5> buffer{4, {}};
  num_dtor_calls = 0;  // remove copy calls
  buffer.clear();
  EXPECT_EQ(num_dtor_calls, 4);
}

TEST(FixedCircularBufferTests, DestructorCallsDestructors) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  {
    FixedCircularBuffer<Object, 5> buffer{4, {}};
    num_dtor_calls = 0;  // remove copy calls
  }
  EXPECT_EQ(num_dtor_calls, 4);
}