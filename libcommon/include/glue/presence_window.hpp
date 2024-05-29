#pragma once

#include <algorithm>
#include <array>
#include <glue/typedefs.hpp>

namespace glue {
class PresenceWindow final {
 public:
  constexpr PresenceWindow() noexcept = default;
  constexpr PresenceWindow(u32 index, u32 flags) noexcept
      : index_{index}, presence_{flags} {}

  static constexpr PresenceWindow combine(PresenceWindow a, PresenceWindow b) {
    if (a.latest() == b.latest()) {
      return {b.latest(), a.present_flags() | b.present_flags()};
    }

    std::array<PresenceWindow, 2> windows{a, b};
    std::sort(
        std::begin(windows), std::end(windows),
        [](const auto& a, const auto& b) { return a.latest() < b.latest(); });
    const auto& [smaller, bigger] = windows;

    if (bigger.oldest() > smaller.latest()) {
      return {bigger.latest(), 0};
    }

    // diff is guaranteed to be at least 1, because we quit early if a.latest()
    // == b.latest()
    const auto diff = bigger.latest() - smaller.latest();
    const u32 index_flag = 1u << (diff - 1);
    const u32 shifted_flags = smaller.present_flags() << diff;
    return {bigger.latest(),
            index_flag | shifted_flags | bigger.present_flags()};
  }

  constexpr u32 oldest() const noexcept {
    if (latest() < 32) {
      return 0;
    }
    return latest() - 32;
  }
  constexpr u32 latest() const noexcept { return index_; }
  constexpr u32 present_flags() const noexcept { return presence_; }

  void mark_present(u32 index) {
    if (index < oldest()) {
      return;
    }
    if (index == latest()) {
      return;
    }

    if (index > latest()) {
      const u32 diff = index - latest();
      if (diff > 32) {
        presence_ = 0;
      } else {
        const u32 index_flag = 1u << (diff - 1);
        const u32 shifted_flags = presence_ << diff;
        presence_ = index_flag | shifted_flags;
      }

      index_ = index;
    } else {
      // we know index is less than latest because we have == and > cases above
      const u32 diff = latest() - index;
      const u32 mask = 1u << (diff - 1);
      presence_ = presence_ | mask;
    }
  }

  constexpr bool operator[](u32 target) const noexcept {
    if (target == latest()) {
      return true;
    }
    if (target > latest()) {
      return false;
    }
    if (target < oldest()) {
      return false;
    }

    // we know target < latest, so diff already starts at 1
    // we subtract 1 because the first value smaller than latest is bit 0
    // (target == latest) doesn't need a bit check because we store it
    // implicitly
    const u32 diff = (latest() - target) - 1;

    // shift so bit we want is LSB, then mask it off. 1 = present, 0 = no
    // diffs of more than 32 are fine, as 0s are shifted in from the left.
    return ((presence_ >> diff) & 1u) == 1u;
  }

 private:
  u32 index_ = 0;
  u32 presence_ = 0;
};
}  // namespace glue