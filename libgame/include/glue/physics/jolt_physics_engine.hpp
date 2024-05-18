#pragma once

#include <glue/physics/base_physics_engine.hpp>
#include <memory>
#include <unordered_map>

namespace glue::physics {
class JoltPhysicsBackend;

class JoltPhysicsEngine final : public BasePhysicsEngine {
 public:
  JoltPhysicsEngine(f32 timestep);
  virtual ~JoltPhysicsEngine();

  virtual void add_dynamic_cube(ObjectID id, const Pose& pose, float radius,
                                bool start_active) override;
  virtual void add_static_plane(ObjectID id, const Plane& plane) override;

  virtual void add_torque(ObjectID id, const vec3& axis, f32 force) override;
  virtual void add_impulse(ObjectID id, const vec3& impulse) override;

  virtual void on_collision_enter(
      ObjectID id,
      std::function<BasePhysicsEngine::OnCollisionEnterCallback> f) override {
    auto it_success_pair = on_collision_enter_subscriptions_.emplace(
        id, std::vector<
                std::function<BasePhysicsEngine::OnCollisionEnterCallback>>{});
    it_success_pair.first->second.push_back(f);
    subscribe_on_collision_enter(id);
  }

 protected:
  virtual void step() override;
  virtual void read_pose(ObjectID object, Pose& pose) override;

 private:
  std::unique_ptr<JoltPhysicsBackend> backend_;
  std::unordered_map<
      ObjectID,
      std::vector<std::function<BasePhysicsEngine::OnCollisionEnterCallback>>>
      on_collision_enter_subscriptions_;

  void process_on_collision_enter_subscriptions();
  void subscribe_on_collision_enter(ObjectID id);
};
}  // namespace glue::physics