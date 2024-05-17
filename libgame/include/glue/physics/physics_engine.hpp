#pragma once

#include <concepts>
#include <glue/types.hpp>

namespace glue::physics {
template <typename T>
concept CreateCube =
    requires(T t, ObjectID id, const Pose& pose, float radius) {
      { t.create_dynamic_cube(id, pose, radius) } -> std::same_as<void>;
    };

template <typename T>
concept CreatePlane = requires(T t, ObjectID id, const Plane& plane) {
  { t.create_static_plane(id, plane) } -> std::same_as<void>;
};

template <typename T>
concept PhysicsSimulator = requires(T t, ObjectID id, Pose& pose) {
  { t.step() } -> std::same_as<void>;
  { T::kTimestep } -> std::convertible_to<float>;
  { t.read_pose(id, pose) } -> std::same_as<void>;
};

template <typename T>
concept PhysicsEngine = PhysicsSimulator<T> && CreateCube<T> && CreatePlane<T>;
}  // namespace glue::physics