#pragma once

// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include <glue/types.hpp>

namespace glue {
/*
 * Object layers are distinct from broadphase layers
 * Typically you have a N:M mapping, M < N.
 */
namespace Layers {
enum Layers : JPH::ObjectLayer { Static, Moving, Count };
};

/*
 * Each broadphase layer defines a new bounding volume tree. Generally we'll at
 * least want to separate static and dynamic objects.
 */
namespace BroadPhaseLayers {
enum Layers : JPH::BroadPhaseLayer::Type { Static, Moving, Count };
}

// Determines which pair of object layers collide
struct ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
  bool ShouldCollide(JPH::ObjectLayer obj_a, JPH::ObjectLayer obj_b);
};

// Determine which pair of object - broadphase layers collide
struct ObjectVsBroadPhaseLayerFilterImpl
    : public JPH::ObjectVsBroadPhaseLayerFilter {
  bool ShouldCollide(JPH::ObjectLayer layer_a,
                     JPH::BroadPhaseLayer layer_b) const override;
};

// Establishes a mapping between Object and Broadphase layers
class BroadPhaseLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
 public:
  BroadPhaseLayerInterfaceImpl();

  u32 GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::Count;
  }

  JPH::BroadPhaseLayer GetBroadPhaseLayer(
      JPH::ObjectLayer layer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  const char* GetBroadPhaseLayerName(
      JPH::BroadPhaseLayer jph_layer) const override;
#endif  // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

 private:
  JPH::BroadPhaseLayer object_to_broad_phase_[Layers::Count];
};
}  // namespace glue