#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/presence_window.hpp>
#include <numeric>

using namespace glue;
using namespace testing;

TEST(PresenceWindowTests, DefaultsZeroed) {
  PresenceWindow window;
  EXPECT_EQ(window.latest(), 0);
  EXPECT_EQ(window.oldest(), 0);
  EXPECT_EQ(window.present_flags(), 0);
}

TEST(PresenceWindowTests, CanCheck33Values) {
  PresenceWindow window{40, 0xffffffff};

  std::vector<u32> present_indices;
  for (auto i = 0; i <= 200; ++i) {
    if (window[i]) {
      present_indices.push_back(i);
    }
  }

  std::vector<u32> expected_indices;
  expected_indices.resize(33);
  std::iota(std::begin(expected_indices), std::end(expected_indices), 8);

  EXPECT_THAT(present_indices, ContainerEq(expected_indices));
}

TEST(PresenceWindowTests, OutOfRangeFalse) {
  PresenceWindow window{40, 0xffffffff};
  EXPECT_FALSE(window[41]);
  EXPECT_FALSE(window[7]);
  EXPECT_FALSE(window[0]);
}

TEST(PresenceWindowTests, Bounds) {
  PresenceWindow a{80, 0xffffffff};
  EXPECT_EQ(a.latest(), 80);
  EXPECT_EQ(a.oldest(), 48);
}

TEST(PresenceWindowTests, CombineWhenAtSameIndex) {
  PresenceWindow a{1224, 0b11100011100011100011100011100011};
  PresenceWindow b{1224, 0b00001000001000001000001000001000};

  auto combined = PresenceWindow::combine(a, b);
  EXPECT_EQ(combined.latest(), 1224);
  EXPECT_EQ(combined.oldest(), 1192);
  EXPECT_EQ(combined.present_flags(), 0b11101011101011101011101011101011);

  std::vector<bool> flags;
  for (auto i = combined.oldest(); i <= combined.latest(); ++i) {
    flags.emplace_back(combined[i]);
  }

  EXPECT_THAT(
      flags, ElementsAre(true, true, true, false, true, false, true, true, true,
                         false, true, false, true, true, true, false, true,
                         false, true, true, true, false, true, false, true,
                         true, true, false, true, false, true, true, true));
}

TEST(PresenceWindowTests, CombineNoOverlap) {
  PresenceWindow a{1224, 0b11100011100011100011100011100011};
  PresenceWindow b{2000, 0b00001000001000001000001000001000};

  auto combined_0 = PresenceWindow::combine(a, b);
  EXPECT_EQ(combined_0.latest(), 2000);
  EXPECT_EQ(combined_0.oldest(), 1968);
  EXPECT_EQ(combined_0.present_flags(), 0);

  auto combined_1 = PresenceWindow::combine(b, a);
  EXPECT_EQ(combined_0.latest(), 2000);
  EXPECT_EQ(combined_0.oldest(), 1968);
  EXPECT_EQ(combined_0.present_flags(), 0);
}

TEST(PresenceWindowTests, CombineOverlap) {
  PresenceWindow a{80, 0b11111111'11011010'10000000'00000000};
  PresenceWindow b{90, 0b00001010'00110110'00000000'00011110};

  auto combined_0 = PresenceWindow::combine(a, b);
  EXPECT_EQ(combined_0.latest(), 90);
  EXPECT_EQ(combined_0.oldest(), 58);
  EXPECT_EQ(combined_0.present_flags(), 0b01101010'00110110'00000010'00011110);

  auto combined_1 = PresenceWindow::combine(b, a);
  EXPECT_EQ(combined_0.latest(), 90);
  EXPECT_EQ(combined_0.oldest(), 58);
  EXPECT_EQ(combined_0.present_flags(), 0b01101010'00110110'00000010'00011110);
}

TEST(PresenceWindowTests, WhenMarkingIndexSmallerThanOldest_NothingHappens) {
  PresenceWindow window{100, 0};

  window.mark_present(1);
  EXPECT_EQ(window.present_flags(), 0);
  EXPECT_EQ(window.latest(), 100);

  window.mark_present(67);
  EXPECT_EQ(window.present_flags(), 0);
  EXPECT_EQ(window.latest(), 100);
}

TEST(PresenceWindowTests, WhenMarkingLatestIndex_NothingHappens) {
  PresenceWindow window{100, 0};
  window.mark_present(100);
  EXPECT_EQ(window.present_flags(), 0);
  EXPECT_EQ(window.latest(), 100);
}

TEST(PresenceWindowTests, WhenMarkingInsideCurrentWindow_SetsValues) {
  PresenceWindow window{100, 0};

  window.mark_present(99);
  EXPECT_EQ(window.present_flags(), 0b00000000'00000000'00000000'00000001);
  EXPECT_EQ(window.latest(), 100);

  window.mark_present(84);
  EXPECT_EQ(window.present_flags(), 0b00000000'00000000'10000000'00000001);
  EXPECT_EQ(window.latest(), 100);

  window.mark_present(84);
  EXPECT_EQ(window.present_flags(), 0b00000000'00000000'10000000'00000001);
  EXPECT_EQ(window.latest(), 100);
}

#include <bitset>
TEST(PresenceWindowTests, WhenMarkingPastLatestIndex_ShiftsTheEntireRangeToIt) {
  PresenceWindow window{100, 0};

  window.mark_present(99);
  EXPECT_EQ(window.present_flags(), 0b00000000'00000000'00000000'00000001);
  EXPECT_EQ(window.latest(), 100);

  window.mark_present(110);
  EXPECT_EQ(window.present_flags(), 0b00000000'00000000'00000110'00000000);
  EXPECT_EQ(window.latest(), 110);
  EXPECT_EQ(window.oldest(), 78);

  window.mark_present(107);
  window.mark_present(106);
  window.mark_present(104);
  EXPECT_EQ(window.present_flags(), 0b00000000'00000000'00000110'00101100);
  EXPECT_EQ(window.latest(), 110);

  window.mark_present(200);
  LOG(INFO) << "test: " << std::bitset<32>(window.present_flags());
  EXPECT_EQ(window.present_flags(), 0);
  EXPECT_EQ(window.latest(), 200);
}