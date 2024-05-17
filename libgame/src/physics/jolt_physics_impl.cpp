#include "jolt_physics_impl.hpp"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <glog/logging.h>

#include "jolt_glm_compat.hpp"

namespace glue::physics {
JoltPhysicsImpl::JoltPhysicsImpl(u32 max_rigidbodies, u32 mutex_count,
                                 u32 max_body_pairs,
                                 u32 max_contact_constraints)
    : temp_allocator_{10 * 1024 * 1024},
      job_system_{2048, 8,
                  static_cast<i32>(std::thread::hardware_concurrency()) - 1} {
  physics_system_.Init(max_rigidbodies, mutex_count, max_body_pairs,
                       max_contact_constraints, broad_phase_layer_interface_,
                       object_vs_broad_phase_layer_filter_,
                       object_layer_pair_filter_);
}

void JoltPhysicsImpl::step() {
  physics_system_.Update(kTimestep, 1, &temp_allocator_, &job_system_);
}

void JoltPhysicsImpl::read_pose(ObjectID id, Pose& pose) {
  auto it = object_id_to_body_id_.find(id);
  CHECK(it != std::end(object_id_to_body_id_));

  const auto body_id = it->second;

  auto& body_interface = physics_system_.GetBodyInterface();
  JPH::Vec3 position = body_interface.GetCenterOfMassPosition(body_id);
  JPH::Quat rotation = body_interface.GetRotation(body_id);
  pose.position = to_glm(position);
  pose.rotation = to_glm(rotation);
}

void JoltPhysicsImpl::add_static_plane(ObjectID id, const Plane& plane) {
  auto& body_interface = physics_system_.GetBodyInterface();

  JPH::BoxShapeSettings shape_settings{JPH::Vec3{plane.size, 1.0f, plane.size}};
  auto shape_result = shape_settings.Create();
  CHECK(!shape_result.HasError());

  JPH::BodyCreationSettings body_creation_settings{
      shape_result.Get(),
      from_glm(plane.pose.position - vec3{0.0f, 1.0f, 0.0f}),
      from_glm(plane.pose.rotation), JPH::EMotionType::Static, Layers::Static};
  JPH::Body* plane_body = body_interface.CreateBody(body_creation_settings);
  CHECK_NOTNULL(plane_body);

  body_interface.AddBody(plane_body->GetID(), JPH::EActivation::DontActivate);

  map_object_to_body(id, plane_body->GetID());
}

void JoltPhysicsImpl::add_dynamic_cube(ObjectID id, const Pose& pose,
                                       float radius, bool start_active) {
  auto& body_interface = physics_system_.GetBodyInterface();

  JPH::BoxShapeSettings cube_shape_settings{JPH::Vec3{radius, radius, radius}};
  auto cube_shape_result = cube_shape_settings.Create();
  CHECK(!cube_shape_result.HasError());

  JPH::BodyCreationSettings cube_settings{
      cube_shape_result.Get(), from_glm(pose.position), from_glm(pose.rotation),
      JPH::EMotionType::Dynamic, Layers::Moving};
  JPH::Body* cube = body_interface.CreateBody(cube_settings);
  CHECK_NOTNULL(cube);

  body_interface.AddBody(cube->GetID(), start_active
                                            ? JPH::EActivation::Activate
                                            : JPH::EActivation::DontActivate);

  map_object_to_body(id, cube->GetID());
}

void JoltPhysicsImpl::add_torque(ObjectID id, const vec3& axis, f32 torque) {
  auto& body_interface = physics_system_.GetBodyInterface();

  auto it = object_id_to_body_id_.find(id);
  CHECK(it != std::end(object_id_to_body_id_));
  const auto body_id = it->second;

  body_interface.AddTorque(body_id, from_glm(axis * torque));
}

void JoltPhysicsImpl::add_impulse(ObjectID id, const vec3& impulse) {
  auto& body_interface = physics_system_.GetBodyInterface();

  auto it = object_id_to_body_id_.find(id);
  CHECK(it != std::end(object_id_to_body_id_));
  const auto body_id = it->second;

  body_interface.AddImpulse(body_id, from_glm(impulse));
}

void JoltPhysicsImpl::map_object_to_body(ObjectID id, JPH::BodyID body) {
  auto it_success_pair = object_id_to_body_id_.emplace(id, body);
  CHECK(it_success_pair.second) << "couldn't insert object. already mapped.";
}
}  // namespace glue::physics