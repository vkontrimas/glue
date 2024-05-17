#pragma once

#include <concepts>
#include <glue/objects/id.hpp>

namespace glue::objects {
template <typename T, typename Data>
concept ObjectInterface = requires(T a, Data data, ObjectID id) {
  { a.get_with_id(id) } -> std::same_as<Data>;
  { a.create_with_id(id) } -> std::same_as<Data>;
  { a.destroy_with_id(id) } -> std::same_as<void>;
};
}  // namespace glue::objects