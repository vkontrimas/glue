#pragma once

#include <chrono>
#include <concepts>
#include <functional>
#include <glue/debug/idata_logger.hpp>
#include <glue/debug/timer.hpp>
#include <glue/types.hpp>

namespace glue::physics {

template <typename T>
concept PrePhysicsUpdateCallback = std::invocable<T>;

class BasePhysicsEngine {
 public:
  explicit BasePhysicsEngine(f32 timestep) noexcept : timestep_{timestep} {}
  virtual ~BasePhysicsEngine() = default;

  template <PrePhysicsUpdateCallback Fn>
  void update(float frame_delta_time, Fn pre_update = []() {}) {
    time_since_past_pose_ += frame_delta_time;
    while (time_since_past_pose_ >= timestep()) {
      time_since_past_pose_ -= timestep();
      pre_update();
      step();
      read_back_poses(std::begin(objects_), std::end(objects_));
    }
  }

  template <PrePhysicsUpdateCallback Fn>
  void update_timed(
      float frame_delta_time, debug::IDataLogger<f64>& update_time,
      Fn pre_update = []() {}) {
    time_since_past_pose_ += frame_delta_time;
    while (time_since_past_pose_ >= timestep()) {
      const debug::Timer timer;

      time_since_past_pose_ -= timestep();
      pre_update();
      step();
      read_back_poses(std::begin(objects_), std::end(objects_));

      update_time.log(timer.elapsed_ms<f64>());
    }
  }

  f32 timestep() const noexcept { return timestep_; }

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
  virtual void add_static_plane(ObjectID id, const Plane& plane) = 0;
  virtual void add_dynamic_cube(ObjectID id, const Pose& pose, float radius,
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

  /*
   * In the future: investigate more efficient ways to process objects in
   * batches!
   *
   * Also, we keep looking up objects by ID, the looking up again in Jolt,
   * we should instead expose a handle that gets returned which knows
   * how to get the required info directly without so much indirection!
   *
   * Individual ops like add_force(id, force) can then go to the handle and
   * batch ops go here!
   */
  Pose get_interpolated_pose(ObjectID object) const noexcept {
    auto it = objects_.find(object);
    if (it == std::end(objects_)) {
      return {};
    }

    const float alpha = time_since_past_pose_ / timestep();
    return lerp(it->second.past_pose, it->second.future_pose, alpha);
  }

 protected:
  virtual void step() = 0;
  virtual void read_pose(ObjectID id, Pose& pose) = 0;

  /*
   * This object management logic is an ugly hack and needs to go.
   * Another example where opaque component handle would benefit us.
   */
  void add_object(ObjectID id, const Pose& pose, bool is_static) {
    auto it_success_pair = objects_.emplace(id, Object{is_static, pose, pose});
    CHECK(it_success_pair.second)
        << "failed adding object to physics engine. (already exists)";
  }

 private:
  f32 timestep_;
  f32 time_since_past_pose_ = 0.0f;

  struct Object {
    bool is_static = false;
    Pose past_pose;
    Pose future_pose;
  };
  std::unordered_map<ObjectID, Object> objects_;

  template <std::input_or_output_iterator It>
  void read_back_poses(It begin, It end) {
    for (auto it = begin; it != end; ++it) {
      if (it->second.is_static) {
        continue;
      }

      Object& o = it->second;

      using std::swap;
      swap(o.past_pose, o.future_pose);

      read_pose(it->first, o.future_pose);
    }
  }
};
}  // namespace glue::physics