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
class JoltPhysicsBackend {
 public:
  JoltPhysicsBackend(u32 max_rigidbodies, u32 mutex_count, u32 max_body_pairs,
                     u32 max_contact_constraints);

  void update(f32 timestep, i32 collision_steps);

  /*
   * This is an example of an inefficiency we could avoid if we
   * exposed an opaque PhysicsComponent handle instead.
   *
   * (Containing BodyID and relevant physics system pointer.)
   */
  void map_object_to_body(ObjectID object, JPH::BodyID body);
  JPH::BodyID get_body_id(ObjectID object) const;

  JPH::PhysicsSystem& physics_system() { return physics_system_; }

 private:
  JPHFactorySingletonInstance factory_singleton_instance_;
  JPH::TempAllocatorImpl temp_allocator_;
  JPH::JobSystemThreadPool job_system_;
  ObjectLayerPairFilterImpl object_layer_pair_filter_;
  BroadPhaseLayerInterfaceImpl broad_phase_layer_interface_;
  ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter_;
  JPH::PhysicsSystem physics_system_;
  std::unordered_map<ObjectID, JPH::BodyID> object_id_to_body_id_;
};
}  // namespace glue::physics
