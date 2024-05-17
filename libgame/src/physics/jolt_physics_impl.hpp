#pragma once

// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <glue/types.hpp>
#include <thread>

#include "jolt_factory_singleton_instance.hpp"
#include "layers.hpp"

namespace glue::physics {
class JoltPhysicsImpl {
 public:
  static constexpr float kTimestep = 1.0f / 60.0f;

  JoltPhysicsImpl(u32 max_rigidbodies, u32 mutex_count, u32 max_body_pairs,
                  u32 max_contact_constraints);

  void step();
  void read_pose(ObjectID id, Pose& pose);

  void create_dynamic_cube(ObjectID id, const Pose& pose, float radius);
  void create_static_plane(ObjectID id, const Plane& plane);

 private:
  JPHFactorySingletonInstance factory_singleton_instance_;
  JPH::TempAllocatorImpl temp_allocator_;
  JPH::JobSystemThreadPool job_system_;
  ObjectLayerPairFilterImpl object_layer_pair_filter_;
  BroadPhaseLayerInterfaceImpl broad_phase_layer_interface_;
  ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter_;
  JPH::PhysicsSystem physics_system_;
};
}  // namespace glue::physics
