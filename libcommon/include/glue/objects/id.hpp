#pragma once

#include <glue/typedefs.hpp>
#include <string>
#include <vector>

namespace glue::objects {
class ObjectID {
 public:
  static constexpr ObjectID None() noexcept {
    return ObjectID{static_cast<u32>(0)};
  }

  constexpr ObjectID() noexcept : ObjectID{None()} {}
  explicit constexpr ObjectID(u32 id) noexcept : id_{id} {}

  // TODO(vkon): make these constexpr by implementing a constexpr CRC32 or
  // similar
  explicit ObjectID(const char* string_name);
  explicit ObjectID(const std::string& string_name);

  constexpr auto value() const noexcept { return id_; }

  constexpr bool operator!=(ObjectID other) const noexcept {
    return !(*this == other);
  }
  constexpr bool operator==(ObjectID other) const noexcept {
    return id_ == other.id_;
  }

  const char* retrieve_name() const;

 private:
  u32 id_;
};
}  // namespace glue::objects

template <>
struct std::hash<glue::objects::ObjectID> {
  constexpr std::size_t operator()(glue::objects::ObjectID id) const noexcept {
    return std::hash<glue::u32>{}(id.value());
  }
};