#pragma once

#include <algorithm>
#include <array>
#include <glue/assert.hpp>
#include <glue/collections/circular_buffer_iterator.hpp>
#include <glue/types.hpp>
#include <iterator>

namespace glue {
template <typename T, std::size_t Capacity>
class FixedCircularBuffer final {
 public:
  using value_type = T;
  using iterator =
      collections::CircularBufferIterator<FixedCircularBuffer&, T, T*, T&>;
  using const_iterator =
      collections::CircularBufferIterator<const FixedCircularBuffer&, T,
                                          const T*, const T&>;

  static constexpr std::size_t capacity() noexcept { return Capacity; }

  FixedCircularBuffer() noexcept {}
  FixedCircularBuffer(std::initializer_list<T> list) {
    glue_assert(list.size() <= capacity());
    std::copy(std::begin(list), std::end(list), std::back_inserter(*this));
  }
  FixedCircularBuffer(std::size_t count, const T& value) {
    glue_assert(count <= capacity());
    std::fill_n(std::back_inserter(*this), count, value);
  }

  ~FixedCircularBuffer() {
    for (auto it = begin(); it != end(); ++it) {
      it->~T();
    }
  }

  FixedCircularBuffer(const FixedCircularBuffer& other) {
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
  }
  FixedCircularBuffer& operator=(const FixedCircularBuffer& other) {
    clear();
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
    return *this;
  }

  FixedCircularBuffer(FixedCircularBuffer&& other) noexcept {
    using std::swap;
    swap(*this, other);
  }
  FixedCircularBuffer& operator=(FixedCircularBuffer&& other) noexcept {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  std::size_t size() const noexcept { return size_; }
  bool empty() const noexcept { return size() == 0; }
  bool full() const noexcept { return size() == capacity(); }

  template <typename... TArgs>
  void emplace_back(TArgs&&... args) {
    glue_assert(size() < capacity());
    new (get_ptr(size())) T{std::forward<TArgs>(args)...};
    ++size_;
  }

  void push_back(const T& value) { emplace_back(value); }

  void pop_back() {
    glue_assert(size() > 0);
    get_ptr(size() - 1)->~T();
    --size_;
  }

  void pop_front() {
    glue_assert(size() > 0);
    get_ptr(0)->~T();
    begin_ = (begin_ + 1) % capacity();
    --size_;
  }

  void clear() {
    for (auto it = begin(); it != end(); ++it) {
      it->~T();
    }
    size_ = 0;
  }

  iterator begin() { return {*this, 0}; }
  iterator end() { return {*this, size()}; }

  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

  const_iterator cbegin() const { return {*this, 0}; }
  const_iterator cend() const { return {*this, size()}; }

  T& operator[](std::size_t index) { return *get_ptr(index); }
  const T& operator[](std::size_t index) const { return *get_ptr(index); }

  friend void swap(FixedCircularBuffer& a, FixedCircularBuffer& b) {
    using std::swap;
    swap(a.data_, b.data_);
    swap(a.begin_, b.begin_);
    swap(a.size_, b.size_);
  }

 private:
  static constexpr std::size_t capacity_bytes() noexcept {
    return capacity() * sizeof(T);
  }

  constexpr std::size_t data_index(std::size_t index) const noexcept {
    return (begin_ + index) % capacity();
  }

  T* get_ptr(std::size_t index) noexcept {
    return reinterpret_cast<T*>(data_) + data_index(index);
  }

  const T* get_ptr(std::size_t index) const noexcept {
    return reinterpret_cast<const T*>(data_) + data_index(index);
  }

 private:
  alignas(T) u8 data_[capacity_bytes()];
  std::size_t begin_ = 0;
  std::size_t size_ = 0;
};
};  // namespace glue