#pragma once

// clang-format off
// Must be included before the rest of Jolt headers!
#include <Jolt/Jolt.h>
// clang-format on

#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <glue/types.hpp>
#include <mutex>
#include <unordered_set>

namespace glue::physics {
class JoltActivationListener : public JPH::BodyActivationListener {
 public:
  virtual void OnBodyActivated(const JPH::BodyID &body,
                               u64 user_data) override {
    const std::lock_guard guard{become_active_lock_};
    become_active_.push_back(body);
    active_bodies_.emplace(body);
  }

  template <typename F>
  void process_active_queue(F f) {
    const std::lock_guard guard{become_active_lock_};
    for (auto body_id : become_active_) {
      f(body_id);
    }
  }

  void clear_active_queue() {
    const std::lock_guard guard{become_active_lock_};
    become_active_.clear();
  }

  virtual void OnBodyDeactivated(const JPH::BodyID &body,
                                 u64 user_data) override {
    const std::lock_guard guard{become_inactive_lock_};
    become_inactive_.push_back(body);
    active_bodies_.erase(body);
  }

  template <typename F>
  void process_inactive_queue(F f) {
    const std::lock_guard guard{become_inactive_lock_};
    for (auto body_id : become_inactive_) {
      f(body_id);
    }
  }

  void clear_inactive_queue() {
    const std::lock_guard guard{become_inactive_lock_};
    become_inactive_.clear();
  }

  auto active_bodies() const { return active_bodies_; }

 private:
  std::mutex become_active_lock_;
  std::vector<JPH::BodyID> become_active_;

  std::mutex become_inactive_lock_;
  std::vector<JPH::BodyID> become_inactive_;

  std::unordered_set<JPH::BodyID> active_bodies_;
};
}  // namespace glue::physics
