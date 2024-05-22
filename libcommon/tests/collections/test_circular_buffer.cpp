#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/collections/circular_buffer.hpp>
#include <glue/types.hpp>

using namespace glue;
using namespace testing;

TEST(CircularBufferTests, DefaultConstructs) {
  CircularBuffer<int> buffer;
  EXPECT_EQ(buffer.capacity(), 0);
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_TRUE(buffer.empty());
}

TEST(CircularBufferTests, WhenDefaultConstructed_DoesNotConstructContents) {
  static int num_ctor_calls = 0;
  struct Object final {
    Object() { num_ctor_calls++; }
  };

  CircularBuffer<Object> buffer;
  EXPECT_EQ(num_ctor_calls, 0);
}

TEST(CircularBufferDeathTests,
     GivenInitializerList_ConstructBufferContainsItems) {
  CircularBuffer<int> buffer{3, {10, 5, 9}};
  EXPECT_EQ(buffer.capacity(), 3);
  EXPECT_EQ(buffer.size(), 3);
  EXPECT_FALSE(buffer.empty());
  EXPECT_THAT(buffer, ElementsAre(10, 5, 9));
}

TEST(CircularBufferTests, GivenFullBuffer_FullIsTrue) {
  CircularBuffer<int> buffer{3};
  buffer.push_back(10);
  buffer.push_back(9);
  buffer.push_back(8);
  EXPECT_TRUE(buffer.full());
}

TEST(CircularBufferTests, GivenNonFullBuffer_FullIsFalse) {
  CircularBuffer<int> buffer{4};
  buffer.push_back(10);
  buffer.push_back(9);
  EXPECT_FALSE(buffer.full());
}

TEST(CircularBufferTests,
     GivenCountAndValue_ConstructBufferContainingCountCopies) {
  struct Object {
    int value;
  };
  CircularBuffer<Object> buffer{4, Object{42}};
  EXPECT_EQ(buffer.capacity(), 4);
  EXPECT_EQ(buffer.size(), 4);
  EXPECT_FALSE(buffer.empty());

  for (auto val : buffer) {
    EXPECT_EQ(val.value, 42);
  }
}

TEST(CircularBufferTests, IndexingWorks) {
  CircularBuffer<int> buffer{3, {10, 9, 8}};
  EXPECT_EQ(buffer[0], 10);
  EXPECT_EQ(buffer[1], 9);
  EXPECT_EQ(buffer[2], 8);
}

TEST(CircularBufferTests, BeginIsEndWhenEmpty) {
  CircularBuffer<int> buffer{10};
  EXPECT_EQ(buffer.capacity(), 10);
  EXPECT_EQ(buffer.size(), 0);
  EXPECT_EQ(std::begin(buffer), std::end(buffer));
}

TEST(CircularBufferTests, IteratorWorks) {
  struct Object {
    int value;
  };
  CircularBuffer<Object> buffer{10, {42}};
  EXPECT_EQ(buffer.capacity(), 10);
  EXPECT_EQ(buffer.size(), 10);

  std::vector<int> values;
  for (auto it = std::begin(buffer); it != std::end(buffer); ++it) {
    values.push_back(it->value);
  }

  EXPECT_THAT(values, ElementsAre(42, 42, 42, 42, 42, 42, 42, 42, 42, 42));
}

TEST(CircularBufferTests, WhenBufferNotFull_IteratorWorks) {
  struct Object {
    int value;
  };
  CircularBuffer<Object> buffer{10};
  EXPECT_EQ(buffer.capacity(), 10);
  EXPECT_EQ(buffer.size(), 0);
  for (int i = 0; i < 4; ++i) {
    buffer.emplace_back(42);
  }

  std::vector<int> values;
  for (auto it = std::begin(buffer); it != std::end(buffer); ++it) {
    values.push_back(it->value);
  }

  EXPECT_THAT(values, ElementsAre(42, 42, 42, 42));
}

TEST(CircularBufferTests, WhenEndIsNotAtEndOfBuffer_IteratorWorks) {
  struct Object {
    int value;
  };
  CircularBuffer<Object> buffer{4, {42}};
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

TEST(CircularBufferTests, WhenIteratorNotAtBegin_IteratorWorks) {
  CircularBuffer<int> buffer{4, {10, 12, 9, 2}};
  EXPECT_EQ(buffer.size(), 4);

  auto begin = std::begin(buffer);
  ++begin;

  std::vector<int> values;
  for (auto it = begin; it != std::end(buffer); ++it) {
    values.push_back(*it);
  }

  EXPECT_THAT(values, ElementsAre(12, 9, 2));
}

TEST(CircularBufferTests, ClearEmptiesBuffer) {
  CircularBuffer<int> buffer{3, {4, 5, 3}};
  EXPECT_THAT(buffer, ElementsAre(4, 5, 3));
  buffer.clear();
  EXPECT_THAT(buffer, IsEmpty());
  EXPECT_EQ(buffer.capacity(), 3);
}

TEST(CircularBufferTests, PushBack) {
  CircularBuffer<int> buffer{10};
  buffer.push_back(4);
  buffer.push_back(5);
  buffer.push_back(3);

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

TEST(CircularBufferTests, PopBack) {
  CircularBuffer<int> buffer{5};
  buffer.push_back(4);
  buffer.push_back(5);
  buffer.push_back(3);

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

TEST(CircularBufferTests, PopFront) {
  CircularBuffer<int> buffer{5};
  buffer.push_back(4);
  buffer.push_back(5);
  buffer.push_back(3);

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

TEST(CircularBufferTests, PopBackCallsDestructor) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  CircularBuffer<Object> buffer{4, Object{}};
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

TEST(CircularBufferTests, PopFrontCallsDestructor) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  CircularBuffer<Object> buffer{4, Object{}};
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

TEST(CircularBufferTests, ClearCallsDestructors) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  CircularBuffer<Object> buffer{4, Object{}};
  num_dtor_calls = 0;  // remove copy calls
  buffer.clear();
  EXPECT_EQ(num_dtor_calls, 4);
}

TEST(CircularBufferTests, DestructorCallsDestructors) {
  static int num_dtor_calls = 0;
  struct Object {
    ~Object() { ++num_dtor_calls; }
  };

  {
    CircularBuffer<Object> buffer{4, Object{}};
    num_dtor_calls = 0;  // remove copy calls
  }
  EXPECT_EQ(num_dtor_calls, 4);
}

TEST(CircularBufferTests, ResizeChangesCapacity) {
  CircularBuffer<int> buffer{4};
  EXPECT_EQ(buffer.capacity(), 4);
  buffer.resize(10);
  EXPECT_EQ(buffer.capacity(), 10);
  buffer.resize(2);
  EXPECT_EQ(buffer.capacity(), 2);
}

TEST(CircularBufferTests, ResizeTruncatesSize) {
  CircularBuffer<int> buffer{5, {4, 5, 6, 7}};
  EXPECT_EQ(buffer.capacity(), 5);
  EXPECT_EQ(buffer.size(), 4);
  EXPECT_THAT(buffer, ElementsAre(4, 5, 6, 7));

  buffer.resize(2);
  EXPECT_EQ(buffer.capacity(), 2);
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_THAT(buffer, ElementsAre(4, 5));

  buffer.resize(10);
  EXPECT_EQ(buffer.capacity(), 10);
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_THAT(buffer, ElementsAre(4, 5));
}

TEST(CircularBufferTests, WhenResized_TruncationCallsDestructors) {
  static int num_dtor_calls = 0;
  struct Object {
    int value;
    ~Object() { num_dtor_calls++; }
  };

  CircularBuffer<Object> buffer{4, Object{42}};
  num_dtor_calls = 0;  // erase copy calls

  EXPECT_EQ(buffer.capacity(), 4);
  EXPECT_EQ(buffer.size(), 4);

  buffer.resize(2);
  EXPECT_EQ(buffer.capacity(), 2);
  EXPECT_EQ(buffer.size(), 2);
  // EXPECT_EQ(num_dtor_calls, 6); // TODO(vkon): we only copy rn so the dtor
  // calls are high, fix
}