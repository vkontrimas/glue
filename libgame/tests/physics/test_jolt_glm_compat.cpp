#include <gtest/gtest.h>

#include <physics/jolt_glm_compat.hpp>

using namespace glue;

TEST(JoltGLMCompat, GLM_vec3_to_Jolt_Vec3) {
  const JPH::Vec3 value = from_glm(vec3{10.0f, 524.0f, 3.14f});
  const JPH::Vec3 expected{10.0f, 524.0f, 3.14f};
  ASSERT_EQ(value, expected);
}

TEST(JoltGLMCompat, Jolt_Vec3_to_GLM_vec3) {
  const vec3 value = to_glm(JPH::Vec3{10.0f, 524.0f, 3.14f});
  const vec3 expected{10.0f, 524.0f, 3.14f};
  ASSERT_EQ(value, expected);
}

TEST(JoltGLMCompat, GLM_quat_to_Jolt_Quat) {
  const JPH::Quat value = from_glm(quat{1.0f, 2.0f, 3.0f, 4.0f});
  const JPH::Quat expected{2.0f, 3.0f, 4.0f, 1.0f};
  ASSERT_EQ(value, expected);
}

TEST(JoltGLMCompat, Jolt_Quat_to_GLM_quat) {
  const quat value = to_glm(JPH::Quat{2.0f, 3.0f, 4.0f, 1.0f});
  const quat expected{1.0f, 2.0f, 3.0f, 4.0f};
  ASSERT_EQ(value, expected);
}