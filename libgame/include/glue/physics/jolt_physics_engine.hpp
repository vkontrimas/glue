#pragma once

#include <glue/physics/base_physics_engine.hpp>
#include <memory>

namespace glue::physics {
class JoltPhysicsBackend;

class JoltPhysicsEngine final : public BasePhysicsEngine {
 public:
  JoltPhysicsEngine(f32 timestep);
  virtual ~JoltPhysicsEngine();

  virtual void add_dynamic_cube(ObjectID id, const Pose& pose, float radius,
                                bool start_active) override;
  virtual void add_static_plane(ObjectID id, const Plane& plane) override;

  virtual void add_torque(ObjectID id, const vec3& axis, f32 force) override;
  virtual void add_impulse(ObjectID id, const vec3& impulse) override;

 protected:
  virtual void step() override;
  virtual void read_pose(ObjectID object, Pose& pose) override;

 private:
  std::unique_ptr<JoltPhysicsBackend> backend_;
};
}  // namespace glue::physics