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
    entry(id).on_collision_enter.push_back(f);
    subscribe_on_collision_enter(id);
  }

  virtual void on_become_active(ObjectID id,
                                std::function<OnActiveCallback> f) override {
    entry(id).on_become_active.push_back(f);
  }

  virtual void on_become_inactive(
      ObjectID id, std::function<OnInactiveCallback> f) override {
    entry(id).on_become_inactive.push_back(f);
  }

 protected:
  virtual void step() override;
  virtual void read_pose(ObjectID object, Pose& pose) override;

 private:
  std::unique_ptr<JoltPhysicsBackend> backend_;

  struct Subscriptions {
    std::vector<std::function<BasePhysicsEngine::OnCollisionEnterCallback>>
        on_collision_enter;
    std::vector<std::function<BasePhysicsEngine::OnActiveCallback>>
        on_become_active;
    std::vector<std::function<BasePhysicsEngine::OnInactiveCallback>>
        on_become_inactive;
  };
  std::unordered_map<ObjectID, Subscriptions> subscriptions_;

  Subscriptions& entry(ObjectID id) {
    auto it_success_pair = subscriptions_.emplace(id, Subscriptions{});
    return it_success_pair.first->second;
  }

  void process_on_collision_enter_subscriptions();
  void process_on_become_active_subscriptions();
  void process_on_become_inactive_subscriptions();

  void subscribe_on_collision_enter(ObjectID id);
};
}  // namespace glue::physics