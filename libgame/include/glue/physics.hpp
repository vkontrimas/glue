#pragma once

#include <glog/logging.h>

#include <glue/physics/jolt_physics_engine.hpp>
#include <glue/physics/physics_engine.hpp>
#include <glue/types.hpp>
#include <memory>
#include <span>
#include <unordered_map>

namespace glue {
// I do like the pattern of doing DI statically.
// An interesting possibility is to create a static DI wrapper which internall
// indirects via a pointer. Then, in-editor you can dynmically replace systems
// as needed. Once a release build is being built, however, we type the chosen
// systems in statically.

// Although I must say: concepts are more annoying to use than I hoped.
// Rather than typing out one interface, you end up having to compose many
// concepts which makes me feel like I shouldn't use them this way.

// In future, write an interface class and a concept that only checks that T is
// derived from it.

template <physics::PhysicsEngine T>
class Physics final {
 public:
  template <typename FnPreUpdate>
  void update(float frame_delta_time, FnPreUpdate pre_update) {
    time_since_past_pose_ += frame_delta_time;
    while (time_since_past_pose_ >= engine_.kTimestep) {
      time_since_past_pose_ -= engine_.kTimestep;
      pre_update();
      engine_.step();
      read_back_poses(std::begin(objects_), std::end(objects_));
    }
  }

  bool exists(ObjectID id) const {
    return objects_.find(id) != std::end(objects_);
  }

  // This is stupid.

  // Future note: this was VERY stupid. It's such a pain in the ass to pipe new
  // functions through. Lesson learned. Next interface will be much thinner.

  // We'll create our own components for colliders, shapes, phys materials
  // Then, a special class somehow instantiated from the physics system will map
  // those onto Jolt (or whatever)

  // Calls will be thinly implemented:
  //  We'll get rid of PIMPL and just have a pointer to an opaque struct
  //  containing jolt resources accessed directly from the super-class.

  // We end up in a three call chain for something that could be much more
  // direct. Too much indirection but it does allow DI.

  // I want to expose this in a better way where the data is closer and we end
  // up with less indirection.

  // Perhaps we can attach 'components' to an Object first.
  // Then, just submit the object ID and the Physics subsystem can figure out
  // the mapping to Jolt world.
  //
  // For example:
  //   - if object has a collider and pose driven by a static transform, it is
  //     a static body.
  //   - if object has a collider and pose driven by rigidbody physics, it is
  //     a dynamic body.
  //   - dynamic transform makes it kinematic.
  //   - no collision = no physics contribution

  void add_static_plane(ObjectID id, const Plane& plane) {
    auto pair = objects_.emplace(id, Object{true, plane.pose, plane.pose});
    CHECK(pair.second) << "object already exists in physics system: "
                       << id.retrieve_name();
    engine_.add_static_plane(id, plane);
  }

  void add_dynamic_cube(ObjectID id, const Pose& pose, float radius,
                        bool start_active) {
    auto pair = objects_.emplace(id, Object{false, pose, pose});
    CHECK(pair.second) << "object already exists in physics system: "
                       << id.retrieve_name();
    engine_.add_dynamic_cube(id, pose, radius, start_active);
  }

  void add_torque(ObjectID id, const vec3& axis, f32 torque) {
    engine_.add_torque(id, axis, torque);
  }

  void add_impulse(ObjectID id, const vec3& impulse) {
    engine_.add_impulse(id, impulse);
  }

  // In the future:
  // on_collision_enter() allows registering callback via lambda for object ID
  // on_collision_exit() likewise

  Pose get_interpolated_pose(ObjectID object) const noexcept {
    auto it = objects_.find(object);
    if (it == std::end(objects_)) {
      return {};
    }

    const float alpha = time_since_past_pose_ / engine_.kTimestep;
    return lerp(it->second.past_pose, it->second.future_pose, alpha);
  }

 private:
  T engine_;

  struct Object {
    bool is_static = false;
    Pose past_pose;
    Pose future_pose;
  };

  std::unordered_map<ObjectID, Object> objects_;
  float time_since_past_pose_ = 0.0f;

  template <std::input_or_output_iterator It>
  void read_back_poses(It begin, It end) {
    for (auto it = begin; it != end; ++it) {
      if (it->second.is_static) {
        continue;
      }

      Object& o = it->second;

      using std::swap;
      swap(o.past_pose, o.future_pose);

      engine_.read_pose(it->first, o.future_pose);
    }
  }
};

using JoltPhysics = Physics<physics::JoltPhysicsEngine>;
}  // namespace glue