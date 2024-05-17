#pragma once

#include <glue/physics/physics_engine.hpp>
#include <memory>

namespace glue::physics {
class JoltPhysicsImpl;

class JoltPhysicsEngine {
 public:
  static constexpr float kTimestep = 1.0f / 60.0f;

  JoltPhysicsEngine();
  ~JoltPhysicsEngine();

  void step();
  void read_pose(ObjectID object, Pose& pose);

  void add_dynamic_cube(ObjectID id, const Pose& pose, float radius,
                        bool start_active);
  void add_static_plane(ObjectID id, const Plane& plane);

 private:
  std::unique_ptr<JoltPhysicsImpl> impl_;
};

static_assert(PhysicsEngine<JoltPhysicsEngine>);
}  // namespace glue::physics