#pragma once

#include <concepts>
#include <glue/objects/id.hpp>

namespace glue::objects {
template <typename T, typename Data>
concept ObjectGet = requires(T a, ObjectID id) {
  // Retrieve object data using ID
  { a.get_with_id(id) } -> std::convertible_to<Data>;
};

template <typename T, typename Data>
concept ObjectSet = requires(T a, ObjectID id, Data data) {
  // Set object data using ID
  { a.set_with_id(id, data) } -> std::same_as<void>;
};

template <typename T, typename Data>
concept ObjectCreate = requires(T a, ObjectID id, Data data) {
  { a.create_with_id(id) } -> std::convertible_to<Data>;
  { a.create_with_id(id, data) } -> std::convertible_to<Data>;
};

template <typename T, typename Data>
concept ObjectDestroy = requires(T a, ObjectID id) {
  { a.destroy_with_id(id) } -> std::same_as<void>;
};

template <typename T, typename Data>
concept ObjectAccess = ObjectGet<T, Data> || ObjectSet<T, Data>;

template <typename T, typename Data>
concept ObjectLifecycle = ObjectCreate<T, Data> || ObjectDestroy<T, Data>;

template <typename T, typename Data>
concept ObjectInterface = ObjectAccess<T, Data> || ObjectLifecycle<T, Data>;
}  // namespace glue