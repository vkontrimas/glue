#pragma once

#include <glue/types.hpp>
#include <vector>

namespace glue {
class ObjectID {
 public:
  static constexpr ObjectID None() noexcept { return ObjectID{0}; }

  constexpr ObjectID() noexcept : ObjectID{None()} {}
  explicit constexpr ObjectID(u32 id) noexcept : id_{id} {}

  constexpr u32 value() const noexcept { return id_; }

  constexpr bool operator!=(ObjectID other) const noexcept {
    return !(*this == other);
  }
  constexpr bool operator==(ObjectID other) const noexcept {
    return id_ == other.id_;
  }

 private:
  u32 id_;
};
}  // namespace glue

template <>
struct std::hash<glue::ObjectID> {
  constexpr std::size_t operator()(glue::ObjectID id) const noexcept {
    return std::hash<glue::u32>{}(id.value());
  }
};