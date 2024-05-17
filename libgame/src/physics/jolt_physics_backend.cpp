#include "jolt_physics_backend.hpp"

#include <glog/logging.h>

namespace glue::physics {
JoltPhysicsBackend::JoltPhysicsBackend(u32 max_rigidbodies, u32 mutex_count,
                                       u32 max_body_pairs,
                                       u32 max_contact_constraints)
    : temp_allocator_{10 * 1024 * 1024},
      job_system_{2048, 8,
                  static_cast<i32>(std::thread::hardware_concurrency()) - 1} {
  physics_system_.Init(max_rigidbodies, mutex_count, max_body_pairs,
                       max_contact_constraints, broad_phase_layer_interface_,
                       object_vs_broad_phase_layer_filter_,
                       object_layer_pair_filter_);
}

void JoltPhysicsBackend::update(f32 timestep, i32 collision_steps) {
  physics_system_.Update(timestep, collision_steps, &temp_allocator_,
                         &job_system_);
}

void JoltPhysicsBackend::map_object_to_body(ObjectID id, JPH::BodyID body) {
  auto it_success_pair = object_id_to_body_id_.emplace(id, body);
  CHECK(it_success_pair.second) << "couldn't insert object. already mapped.";
}

JPH::BodyID JoltPhysicsBackend::get_body_id(ObjectID object) const {
  auto it = object_id_to_body_id_.find(object);
  CHECK(it != std::end(object_id_to_body_id_));
  return it->second;
}
}  // namespace glue::physics