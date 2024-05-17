#pragma once

#include <glue/physics/jolt_physics_engine.hpp>
#include <glue/physics/physics_engine.hpp>
#include <glue/types.hpp>
#include <glue/world.hpp>
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
  void update(float frame_delta_time) {
    time_since_past_pose_ += frame_delta_time;
    while (time_since_past_pose_ > engine_.kTimestep) {
      time_since_past_pose_ -= engine_.kTimestep;
      engine_.step();
      read_back_poses(std::begin(objects_), std::end(objects_));
    }
  }

  bool exists(ObjectID id) const {
    return objects_.find(id) != std::end(objects_);
  }

  // This is stupid.
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

  void add_dynamic_cube(ObjectID id, const Pose& pose, float radius) {
    auto pair = objects_.emplace(id, Object{true, pose, pose});
    CHECK(pair.second) << "object already exists in physics system: "
                       << id.retrieve_name();
    engine_.add_dynamic_cube(id, pose, radius);
  }

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

      using std::swap;
      swap(it->second.past_pose, it->second.future_pose);

      engine_.read_pose(it->first, it->second.future_pose);
    }
  }
};

using JoltPhysics = Physics<physics::JoltPhysicsEngine>;
}  // namespace glue