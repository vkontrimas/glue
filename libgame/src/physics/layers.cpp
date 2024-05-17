#include "layers.hpp"

#include <glog/logging.h>

namespace glue {
bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer obj_a,
                                              JPH::ObjectLayer obj_b) {
  if (obj_a == Layers::Static) {
    return obj_b == Layers::Moving;
  } else if (obj_a == Layers::Moving) {
    return true;
  }
  LOG(FATAL) << "Unhandled collision pair";
}

BroadPhaseLayerInterfaceImpl::BroadPhaseLayerInterfaceImpl() {
  object_to_broad_phase_[Layers::Static] =
      JPH::BroadPhaseLayer{BroadPhaseLayers::Static};
  object_to_broad_phase_[Layers::Moving] =
      JPH::BroadPhaseLayer{BroadPhaseLayers::Moving};
}

JPH::BroadPhaseLayer BroadPhaseLayerInterfaceImpl::GetBroadPhaseLayer(
    JPH::ObjectLayer layer) const {
  CHECK(layer < Layers::Count);
  return object_to_broad_phase_[layer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char* BroadPhaseLayerInterfaceImpl::GetBroadPhaseLayerName(
    JPH::BroadPhaseLayer jph_layer) const {
  const auto layer = jph_layer.GetValue();
  if (layer == BroadPhaseLayers::Static) {
    return "Static";
  } else if (layer == BroadPhaseLayers::Moving) {
    return "Moving";
  }

  return "Undefined";
}
#endif  // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(
    JPH::ObjectLayer layer_a, JPH::BroadPhaseLayer layer_b) const {
  const auto layer_b_val = layer_b.GetValue();
  if (layer_a == Layers::Static) {
    return layer_b_val == BroadPhaseLayers::Moving;
  } else if (layer_a == Layers::Moving) {
    return true;
  }

  LOG(FATAL) << "Unhandled object / broadphase pair";
  return false;
}
}  // namespace glue