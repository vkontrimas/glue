#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glue/bitpack/bitpack.hpp>
#include <glue/types.hpp>
#include <vector>

using namespace glue;
using namespace glue::bitpack;

/*
 * Let's pack a simple skeleton.
 */
struct Skeleton {
  f32 pos_x, pos_y, pos_z;
  u32 hitpoints;
  bool is_alive;

  // for EXPECT_EQ
  friend constexpr bool operator==(const Skeleton& a, const Skeleton& b) {
    return a.pos_x == b.pos_x && a.pos_y == b.pos_y && a.pos_z == b.pos_z &&
           a.hitpoints == b.hitpoints && a.is_alive == b.is_alive;
  }

  /*
   * We only have to define a single pack function to handle both,
   * serialization and deserialization.
   *
   * On serialize, we take the reference and write it.
   * On deserialize, we update the reference.
   *
   * As a convenience for conditional expressions, we may return
   * a value. pack<bool> does this.
   */
  template <CPacker TPacker>
  friend constexpr void pack(TPacker& packer, Skeleton& skeleton) {
    pack(packer, skeleton.pos_x);
    pack(packer, skeleton.pos_y);
    pack(packer, skeleton.pos_z);
    pack(packer, skeleton.hitpoints);
    pack(packer, skeleton.is_alive);
  }
};

TEST(BitpackExampleTests, SerializeAndDeserializeObject) {
  std::vector<u32> network;
  network.resize(sizeof(Skeleton), 0);

  Packer packer{network};
  Skeleton skeleton_before{12.0f, 60.0f, -11.0f, 20, true};
  pack(packer, skeleton_before);

  Unpacker unpacker{network};
  Skeleton skeleton_after{};
  pack(unpacker, skeleton_after);

  EXPECT_EQ(skeleton_before, skeleton_after);
}

/*
 * Let's write some poses that are packed only if changed.
 */
struct TestVec3 {
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;

  friend constexpr bool operator==(const TestVec3& a, const TestVec3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
  }

  template <CPacker T>
  friend constexpr void pack(T& packer, TestVec3& vec) {
    pack(packer, vec.x);
    pack(packer, vec.y);
    pack(packer, vec.z);
  }
};

struct TestQuat {
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
  f32 w = 0;

  friend constexpr bool operator==(const TestQuat& a, const TestQuat& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
  }

  template <CPacker T>
  friend constexpr void pack(T& packer, TestQuat& quat) {
    pack(packer, quat.x);
    pack(packer, quat.y);
    pack(packer, quat.z);
    pack(packer, quat.w);
  }
};

struct TestPose {
  bool updated;
  TestVec3 pos;
  TestQuat rot;

  friend constexpr bool operator==(const TestPose& a, const TestPose& b) {
    return a.updated == b.updated && a.pos == b.pos && a.rot == b.rot;
  }

  template <CPacker T>
  friend constexpr void pack(T& packer, TestPose& pose) {
    // here, we're only sending the position if it has been updated
    // otherwise we only send the one bit
    if (pack(packer, pose.updated)) {
      pack(packer, pose.pos);
      pack(packer, pose.rot);
    }
  }
};

TEST(BitpackExampleTests, SerializeAndDeserializePoses) {
  std::vector<u32> network;
  network.resize(sizeof(TestPose) * 3, 0);

  {
    Packer packer{network};
    TestPose full{true, {1, 2, 3}, {0.25, 0.5, 0.25, 0.125}};

    // Here, active is true so we end up writing the entire pose.
    ASSERT_EQ(packer.current_bit(), 0);
    pack(packer, full);
    EXPECT_EQ(packer.current_bit(), 225);

    Unpacker unpacker{network};
    TestPose full_unpacked{};
    pack(unpacker, full_unpacked);
    EXPECT_EQ(full, full_unpacked);
  }

  {
    Packer packer{network};
    TestPose only_updated_bit{false, {1, 2, 3}, {0.25, 0.5, 0.25, 0.125}};

    // Here, active is false, so we only end up writing the single zero bit
    ASSERT_EQ(packer.current_bit(), 0);
    pack(packer, only_updated_bit);
    EXPECT_EQ(packer.current_bit(), 1);

    Unpacker unpacker{network};
    TestPose unpacked_pose{false, {0, 0, 0}, {0, 0, 0, 0}};
    pack(unpacker, unpacked_pose);

    // Because active was false and only one bit sent, the rest of the values
    // are left as they are.
    // THEY ARE NOT ZERO-d by pack(), instead they're left as they were
    // initialized before.
    const TestVec3 zero_vec{0, 0, 0};
    const TestQuat zero_quat{0, 0, 0, 0};
    EXPECT_FALSE(unpacked_pose.updated);
    EXPECT_EQ(unpacked_pose.pos, zero_vec);
    EXPECT_EQ(unpacked_pose.rot, zero_quat);
  }
}