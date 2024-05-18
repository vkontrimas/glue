#pragma once

// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/ContactListener.h>

#include <mutex>
#include <unordered_map>
#include <vector>

namespace glue::physics {
class JoltContactListener : public JPH::ContactListener {
 public:
  virtual void OnContactAdded(const JPH::Body &body1, const JPH::Body &body2,
                              const JPH::ContactManifold &manifold,
                              JPH::ContactSettings &settings) override {
    on_contact_added_process_pair(body1.GetID(), body2.GetID());
    on_contact_added_process_pair(body2.GetID(), body1.GetID());
  }

  void subscribe_on_contact_added(JPH::BodyID body) {
    const std::lock_guard guard{on_contact_added_lock_};
    on_contact_added_subscriptions_.emplace(body, std::vector<JPH::BodyID>{});
  }

  template <typename F>
  void process_on_contact_added_queue(JPH::BodyID body, F f) {
    const std::lock_guard guard{on_contact_added_lock_};
    auto it = on_contact_added_subscriptions_.find(body);
    if (it == std::end(on_contact_added_subscriptions_)) {
      return;
    }
    for (auto other_body : it->second) {
      f(other_body);
    }
  }

  void clear_on_contact_added_queue(JPH::BodyID body) {
    const std::lock_guard guard{on_contact_added_lock_};
    auto it = on_contact_added_subscriptions_.find(body);
    if (it == std::end(on_contact_added_subscriptions_)) {
      return;
    }
    it->second.clear();
  }

 private:
  // mutex needed as OnContactAdded could be called from any thread!
  std::mutex on_contact_added_lock_;
  std::unordered_map<JPH::BodyID, std::vector<JPH::BodyID>>
      on_contact_added_subscriptions_;

  void on_contact_added_process_pair(JPH::BodyID target, JPH::BodyID other) {
    const std::lock_guard guard{on_contact_added_lock_};
    auto it = on_contact_added_subscriptions_.find(target);
    if (it == std::end(on_contact_added_subscriptions_)) {
      return;
    }
    it->second.push_back(other);
  }
};
}  // namespace glue::physics