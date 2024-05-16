// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <glog/logging.h>

#include <cstdarg>
#include <glue/physics.hpp>
#include <glue/types.hpp>
#include <thread>

namespace glue {
namespace {

// JOLT trace logger function
static void trace_impl(const char* fmt, ...) {
  // format
  // (ripped from Jolt/HelloWorld.cpp)
  va_list list;
  va_start(list, fmt);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), fmt, list);
  va_end(list);

  // log
  LOG(INFO) << "JOLT: " << buffer;
}

// JOLT assert handler
#ifdef JPH_ENABLE_ASSERTS
static bool assert_failed_impl(const char* expression, const char* message,
                               const char* file, uint line) {
  LOG(FATAL) << "JOLT: " << file << ":" << line << ": (" << expression << ") "
             << (message != nullptr ? message : "");
  return true;
};
#endif  // JPH_ENABLE_ASSERTS

// Just handling the JPH::Factory class via RAII
// Extra work to kill singleton
struct JPHFactorySingletonInstance {
  JPHFactorySingletonInstance() : factory_{new JPH::Factory} {
    JPH::Factory::sInstance = factory_.get();
    JPH::RegisterTypes();
  }

  struct Deleter {
    void operator()(JPH::Factory* factory) {
      JPH::Factory::sInstance = nullptr;
      delete factory;
    }
  };

  std::unique_ptr<JPH::Factory, Deleter> factory_;
};

/*
 * Object layers are distinct from broadphase layers
 * Typically you have a N:M mapping, M < N.
 */
namespace Layers {
enum Layers : JPH::ObjectLayer { Static, Moving, Count };
};

struct ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
  bool ShouldCollide(JPH::ObjectLayer obj_a, JPH::ObjectLayer obj_b) {
    if (obj_a == Layers::Static) {
      return obj_b == Layers::Moving;
    } else if (obj_a == Layers::Moving) {
      return true;
    }
    LOG(FATAL) << "Unhandled collision pair";
  }
};

/*
 * Each broadphase layer defines a new bounding volume tree. Generally we'll at
 * least want to separate static and dynamic objects.
 */
namespace BroadPhaseLayers {
enum Layers : JPH::BroadPhaseLayer::Type { Static, Moving, Count };
}

class BroadPhaseLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
 public:
  BroadPhaseLayerInterfaceImpl() {
    object_to_broad_phase_[Layers::Static] =
        JPH::BroadPhaseLayer{BroadPhaseLayers::Static};
    object_to_broad_phase_[Layers::Moving] =
        JPH::BroadPhaseLayer{BroadPhaseLayers::Moving};
  }

  u32 GetNumBroadPhaseLayers() const override {
    return BroadPhaseLayers::Count;
  }

  JPH::BroadPhaseLayer GetBroadPhaseLayer(
      JPH::ObjectLayer layer) const override {
    CHECK(layer < Layers::Count);
    return object_to_broad_phase_[layer];
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  const char* GetBroadPhaseLayerName(
      JPH::BroadPhaseLayer jph_layer) const override {
    const auto layer = jph_layer.GetValue();
    if (layer == BroadPhaseLayers::Static) {
      return "Static";
    } else if (layer == BroadPhaseLayers::Moving) {
      return "Moving";
    }

    return "Undefined";
  }
#endif  // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

 private:
  JPH::BroadPhaseLayer object_to_broad_phase_[Layers::Count];
};

// Determine if object collides with broadphase layer
struct ObjectVsBroadPhaseLayerFilterImpl
    : public JPH::ObjectVsBroadPhaseLayerFilter {
  bool ShouldCollide(JPH::ObjectLayer layer_a,
                     JPH::BroadPhaseLayer layer_b) const override {
    const auto layer_b_val = layer_b.GetValue();
    if (layer_a == Layers::Static) {
      return layer_b_val == BroadPhaseLayers::Moving;
    } else if (layer_a == Layers::Moving) {
      return true;
    }

    LOG(FATAL) << "Unhandled object / broadphase pair";
    return false;
  }
};

JPH::Vec3 glm(vec3 pos) noexcept { return {pos.x, pos.y, pos.z}; }

JPH::Quat glm(quat o) noexcept { return {o.x, o.y, o.z, o.w}; }
}  // namespace

class PhysicsImpl {
 public:
  PhysicsImpl(u32 max_rigidbodies, u32 mutex_count, u32 max_body_pairs,
              u32 max_contact_constraints)
      : temp_allocator_{10 * 1024 * 1024},
        job_system_{2048, 8,
                    static_cast<i32>(std::thread::hardware_concurrency()) - 1} {
    physics_system_.Init(max_rigidbodies, mutex_count, max_body_pairs,
                         max_contact_constraints, broad_phase_layer_interface_,
                         object_vs_broad_phase_layer_filter_,
                         object_layer_pair_filter_);
  }

  void setup_static_objects(const World& world) {
    auto& body_interface = physics_system_.GetBodyInterface();

    // ADD GROUND PLANE
    JPH::BoxShapeSettings ground_shape_settings{
        JPH::Vec3{world.ground.size, 1.0f, world.ground.size}};
    auto ground_shape_result = ground_shape_settings.Create();
    CHECK(!ground_shape_result.HasError());

    JPH::BodyCreationSettings ground_settings{
        ground_shape_result.Get(), glm(world.ground.pose.position),
        glm(world.ground.pose.rotation), JPH::EMotionType::Static,
        Layers::Static};
    JPH::Body* ground = body_interface.CreateBody(ground_settings);
    CHECK_NOTNULL(ground);

    body_interface.AddBody(ground->GetID(), JPH::EActivation::DontActivate);

    // ADD CUBE
    JPH::BoxShapeSettings cube_shape_settings{JPH::Vec3{1.0f, 1.0f, 1.0f}};
    auto cube_shape_result = cube_shape_settings.Create();
    CHECK(!cube_shape_result.HasError());

    JPH::BodyCreationSettings cube_settings{
        cube_shape_result.Get(), glm(world.cube.position),
        glm(world.cube.rotation), JPH::EMotionType::Dynamic, Layers::Moving};
    JPH::Body* cube = body_interface.CreateBody(cube_settings);
    CHECK_NOTNULL(cube);

    body_interface.AddBody(cube->GetID(), JPH::EActivation::Activate);
    cube_id_ = cube->GetID();
  }

  void update(float frame_delta_time, World& world) {
    remaining_simulation_time_ += frame_delta_time;

    int steps_this_frame = 0;
    constexpr float kTimestep = 1.0f / 60.0f;
    while (remaining_simulation_time_ > kTimestep) {
      constexpr i32 kCollisionSteps = 1;
      physics_system_.Update(kTimestep, kCollisionSteps, &temp_allocator_,
                             &job_system_);
      remaining_simulation_time_ -= kTimestep;
      ++steps_this_frame;
    }

    if (steps_this_frame > 0) {
      update_world_state(world);
    }
  }

 private:
  JPHFactorySingletonInstance factory_singleton_instance_;
  JPH::TempAllocatorImpl temp_allocator_;
  JPH::JobSystemThreadPool job_system_;
  ObjectLayerPairFilterImpl object_layer_pair_filter_;
  BroadPhaseLayerInterfaceImpl broad_phase_layer_interface_;
  ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter_;
  JPH::PhysicsSystem physics_system_;
  JPH::BodyID cube_id_;
  float remaining_simulation_time_ = 0.0f;

  void update_world_state(World& world) {
    auto& body_interface = physics_system_.GetBodyInterface();

    JPH::Vec3 cube_position = body_interface.GetCenterOfMassPosition(cube_id_);
    world.cube.position = {cube_position.GetX(), cube_position.GetY(),
                           cube_position.GetZ()};

    JPH::Quat cube_rotation = body_interface.GetRotation(cube_id_);
    world.cube.rotation = {cube_rotation.GetX(), cube_rotation.GetY(),
                           cube_rotation.GetZ(), cube_rotation.GetW()};
  }
};

Physics::Physics() {
  JPH::RegisterDefaultAllocator();

  JPH::Trace = trace_impl;
#ifdef JPH_ENABLE_ASSERTS
  JPH::AssertFailed = assert_failed_impl;
#endif

  impl_.reset(new PhysicsImpl{1024, 0, 1024, 1024});
}
Physics::~Physics() = default;

void Physics::setup_static_objects(const World& world) {
  impl_->setup_static_objects(world);
}

void Physics::update(float frame_delta_time, World& world) {
  impl_->update(frame_delta_time, world);
}
}  // namespace glue