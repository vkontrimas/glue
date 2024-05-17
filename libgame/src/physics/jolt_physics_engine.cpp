#include <glue/physics/jolt_physics_engine.hpp>

#include "jolt_physics_impl.hpp"
#include "jolt_setup_globals.hpp"

namespace glue::physics {
JoltPhysicsEngine::JoltPhysicsEngine() {
  setup_jolt_allocator();
  setup_jolt_logging();
  impl_.reset(new JoltPhysicsImpl{65536, 0, 65536, 10240});
}

JoltPhysicsEngine::~JoltPhysicsEngine() = default;

void JoltPhysicsEngine::step() { impl_->step(); }

void JoltPhysicsEngine::read_pose(ObjectID object, Pose& pose) {
  impl_->read_pose(object, pose);
}

void JoltPhysicsEngine::add_dynamic_cube(ObjectID id, const Pose& pose,
                                         float radius, bool start_active) {
  impl_->add_dynamic_cube(id, pose, radius, start_active);
}

void JoltPhysicsEngine::add_static_plane(ObjectID id, const Plane& plane) {
  impl_->add_static_plane(id, plane);
}

void JoltPhysicsEngine::add_torque(ObjectID id, const vec3& axis, f32 torque) {
  impl_->add_torque(id, axis, torque);
}

void JoltPhysicsEngine::add_impulse(ObjectID id, const vec3& impulse) {
  impl_->add_impulse(id, impulse);
}
}  // namespace glue::physics