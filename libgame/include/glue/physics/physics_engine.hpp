#pragma once

#include <concepts>
#include <glue/types.hpp>

namespace glue::physics {
template <typename T>
concept AddCube =
    requires(T t, ObjectID id, const Pose& pose, float radius, bool s) {
      { t.add_dynamic_cube(id, pose, radius, s) } -> std::same_as<void>;
    };

template <typename T>
concept AddPlane = requires(T t, ObjectID id, const Plane& plane) {
  { t.add_static_plane(id, plane) } -> std::same_as<void>;
};

template <typename T>
concept PhysicsSimulator = requires(T t, ObjectID id, Pose& pose) {
  { t.step() } -> std::same_as<void>;
  { T::kTimestep } -> std::convertible_to<float>;
  { t.read_pose(id, pose) } -> std::same_as<void>;
};

template <typename T>
concept PhysicsEngine = PhysicsSimulator<T> && AddCube<T> && AddPlane<T>;
}  // namespace glue::physics