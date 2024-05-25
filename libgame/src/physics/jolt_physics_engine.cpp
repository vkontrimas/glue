// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <glog/logging.h>

#include <glue/physics/jolt_physics_engine.hpp>
#include <glue/world_frame.hpp>

#include "jolt_glm_compat.hpp"
#include "jolt_physics_backend.hpp"
#include "jolt_setup_globals.hpp"

namespace glue::physics {
JoltPhysicsEngine::JoltPhysicsEngine() {
  setup_jolt_allocator();
  setup_jolt_logging();
  backend_.reset(new JoltPhysicsBackend{65536, 0, 65536, 10240});
}

JoltPhysicsEngine::~JoltPhysicsEngine() = default;

void JoltPhysicsEngine::step(f64 timestep, WorldFrame& frame) {
  backend_->update(static_cast<f32>(timestep), 1);
  process_on_collision_enter_subscriptions();
  process_on_become_active_subscriptions();
  process_on_become_inactive_subscriptions();
  read_back_poses(frame);
}

void JoltPhysicsEngine::read_back_poses(WorldFrame& frame) {
  for (auto body_id : backend_->activation_listener().active_bodies()) {
    const auto index = backend_->get_object_index(body_id);

    auto& body_interface = backend_->physics_system().GetBodyInterface();
    auto position = body_interface.GetCenterOfMassPosition(body_id);
    auto rotation = body_interface.GetRotation(body_id);

    frame.cubes[index].position = to_glm(position);
    frame.cubes[index].rotation = to_glm(rotation);
  }
}

void JoltPhysicsEngine::process_on_collision_enter_subscriptions() {
  for (auto& pair : subscriptions_) {
    const auto body_id = backend_->get_body_id(pair.first);
    backend_->contact_listener().process_on_contact_added_queue(
        body_id, [&](JPH::BodyID other) {
          const auto other_object_id = backend_->get_object_id(other);
          for (auto& f : pair.second.on_collision_enter) {
            f(other_object_id);
          }
        });
    backend_->contact_listener().clear_on_contact_added_queue(body_id);
  }
}

void JoltPhysicsEngine::process_on_become_active_subscriptions() {
  backend_->activation_listener().process_active_queue(
      [&](JPH::BodyID body_id) {
        const auto object_id = backend_->get_object_id(body_id);
        auto it = subscriptions_.find(object_id);
        if (it == std::end(subscriptions_)) {
          return;
        }
        for (auto& f : it->second.on_become_active) {
          f();
        }
      });
  backend_->activation_listener().clear_active_queue();
}

void JoltPhysicsEngine::process_on_become_inactive_subscriptions() {
  backend_->activation_listener().process_inactive_queue(
      [&](JPH::BodyID body_id) {
        const auto object_id = backend_->get_object_id(body_id);
        auto it = subscriptions_.find(object_id);
        if (it == std::end(subscriptions_)) {
          return;
        }
        for (auto& f : it->second.on_become_inactive) {
          f();
        }
      });
  backend_->activation_listener().clear_inactive_queue();
}

void JoltPhysicsEngine::subscribe_on_collision_enter(ObjectID id) {
  const auto body_id = backend_->get_body_id(id);
  backend_->contact_listener().subscribe_on_contact_added(body_id);
}

void JoltPhysicsEngine::add_dynamic_cube(ObjectID id, std::size_t stupid_index,
                                         const Pose& pose, float radius,
                                         bool start_active) {
  auto& body_interface = backend_->physics_system().GetBodyInterface();

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

  backend_->map_object_to_body(id, stupid_index, cube->GetID());
}

void JoltPhysicsEngine::add_static_plane(ObjectID id, std::size_t stupid_index,
                                         const Plane& plane) {
  auto& body_interface = backend_->physics_system().GetBodyInterface();

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

  backend_->map_object_to_body(id, stupid_index, plane_body->GetID());
}

void JoltPhysicsEngine::add_torque(ObjectID id, const vec3& axis, f32 torque) {
  auto& body_interface = backend_->physics_system().GetBodyInterface();
  body_interface.AddTorque(backend_->get_body_id(id), from_glm(axis * torque));
}

void JoltPhysicsEngine::add_impulse(ObjectID id, const vec3& impulse) {
  auto& body_interface = backend_->physics_system().GetBodyInterface();
  body_interface.AddImpulse(backend_->get_body_id(id), from_glm(impulse));
}

void JoltPhysicsEngine::add_force(ObjectID id, const vec3& force) {
  auto& body_interface = backend_->physics_system().GetBodyInterface();
  body_interface.AddForce(backend_->get_body_id(id), from_glm(force));
}

}  // namespace glue::physics