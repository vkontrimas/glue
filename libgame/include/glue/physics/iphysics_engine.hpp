#pragma once

#include <chrono>
#include <concepts>
#include <functional>
#include <glue/debug/idata_logger.hpp>
#include <glue/debug/timer.hpp>
#include <glue/types.hpp>

namespace glue {
class WorldFrame;
}

namespace glue::physics {
class IPhysicsEngine {
 public:
  virtual ~IPhysicsEngine() = default;

  virtual void step(f64 timestep, WorldFrame& frame) = 0;

  /*
   * I want to expose these in a better way where the data is closer and we end
   * up with less indirection.
   *
   * Perhaps we can attach 'components' to an Object first.
   * Then, just submit the object ID and the Physics subsystem can figure out
   * the mapping to Jolt world.
   *
   * For example:
   *   - if object has a collider and pose driven by a static transform, it is
   *     a static body.
   *   - if object has a collider and pose driven by rigidbody physics, it is
   *     a dynamic body.
   *   - dynamic transform makes it kinematic.
   *   - no collision = no physics contribution
   */
  virtual void add_static_plane(ObjectID id, std::size_t stupid_index,
                                const Plane& plane) = 0;
  virtual void add_dynamic_cube(ObjectID id, std::size_t stupid_index,
                                const Pose& pose, float radius,
                                bool start_active) = 0;

  virtual void add_torque(ObjectID id, const vec3& axis, f32 torque) = 0;
  virtual void add_impulse(ObjectID id, const vec3& impulse) = 0;
  virtual void add_force(ObjectID id, const vec3& force) = 0;

  using OnCollisionEnterCallback = void(ObjectID);
  virtual void on_collision_enter(
      ObjectID id, std::function<OnCollisionEnterCallback> f) = 0;

  using OnActiveCallback = void();
  using OnInactiveCallback = void();
  virtual void on_become_active(ObjectID id,
                                std::function<OnActiveCallback> f) = 0;
  virtual void on_become_inactive(ObjectID id,
                                  std::function<OnInactiveCallback> f) = 0;
};
}  // namespace glue::physics