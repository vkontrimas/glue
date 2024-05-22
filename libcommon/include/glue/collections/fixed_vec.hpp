#pragma once

#include <glog/logging.h>

#include <algorithm>
#include <array>
#include <glue/assert.hpp>
#include <glue/types.hpp>

namespace glue {
template <typename T, std::size_t Capacity>
class FixedVec final {
 public:
  using value_type = T;

  static constexpr std::size_t capacity() noexcept { return Capacity; }

  FixedVec() noexcept {}
  FixedVec(std::initializer_list<T> items) {
    glue_assert(items.size() <= capacity());
    std::copy(std::begin(items), std::end(items), std::back_inserter(*this));
  }
  FixedVec(std::size_t count, const T& item) {
    glue_assert(count <= capacity());
    std::fill_n(std::back_inserter(*this), count, item);
  }

  ~FixedVec() {
    for (auto it = begin(); it != end(); ++it) {
      it->~T();
    }
  }

  FixedVec(const FixedVec& other) {
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
  }
  FixedVec& operator=(const FixedVec& other) {
    clear();
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
    return *this;
  }

  FixedVec(FixedVec&& other) {
    using std::swap;
    swap(*this, other);
  }

  FixedVec& operator=(FixedVec&& other) {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  std::size_t size() const noexcept { return size_; }
  bool empty() const noexcept { return size() == 0; }
  bool full() const noexcept { return size() == capacity(); }

  void push_back(const T& val) { emplace_back(val); }

  template <typename... TArgs>
  void emplace_back(TArgs&&... args) {
    glue_assert(size() < capacity());
    new (get_ptr(size())) T{std::forward<TArgs>(args)...};
    ++size_;
  }

  void pop_back() {
    glue_assert(size() > 0);
    get_ptr(size() - 1)->~T();
    --size_;
  }

  void clear() {
    for (auto it = begin(); it != end(); ++it) {
      it->~T();
    }
    size_ = 0;
  }

  T& operator[](std::size_t index) noexcept { return *get_ptr(index); }

  const T& operator[](std::size_t index) const noexcept {
    return *get_ptr(index);
  }

  T* begin() { return get_ptr(0); }
  T* end() { return get_ptr(size()); }

  const T* begin() const { return cbegin(); }
  const T* end() const { return cend(); }

  const T* cbegin() const { return get_ptr(0); }
  const T* cend() const { return get_ptr(size()); }

  friend void swap(FixedVec& a, FixedVec& b) {
    using std::swap;
    swap(a.data_, b.data_);
    swap(a.size_, b.size_);
  }

 private:
  static constexpr std::size_t capacity_bytes() noexcept {
    return capacity() * sizeof(T);
  }

  T* get_ptr(std::size_t index) noexcept {
    return reinterpret_cast<T*>(&data_[0]) + index;
  }

  const T* get_ptr(std::size_t index) const noexcept {
    return reinterpret_cast<const T*>(&data_[0]) + index;
  }

 private:
  alignas(T) u8 data_[capacity_bytes()];
  std::size_t size_ = 0;
};
}  // namespace glue