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
                                         float radius) {
  impl_->add_dynamic_cube(id, pose, radius);
}
void JoltPhysicsEngine::add_static_plane(ObjectID id, const Plane& plane) {
  impl_->add_static_plane(id, plane);
}
}  // namespace glue::physics