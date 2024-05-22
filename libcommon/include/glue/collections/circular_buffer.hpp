#pragma once

#include <glue/assert.hpp>
#include <glue/collections/circular_buffer_iterator.hpp>
#include <glue/types.hpp>

namespace glue {
template <typename T>
class CircularBuffer final {
 public:
  using value_type = T;
  using iterator =
      collections::CircularBufferIterator<CircularBuffer&, T, T*, T&>;
  using const_iterator =
      collections::CircularBufferIterator<const CircularBuffer&, T, const T*,
                                          const T&>;

  CircularBuffer() noexcept : data_{nullptr} {}
  explicit CircularBuffer(std::size_t capacity)
      : data_{alloc_capacity(capacity)}, capacity_{capacity} {}
  CircularBuffer(std::size_t count, const T& value)
      : data_{alloc_capacity(count)}, capacity_{count} {
    std::fill_n(std::back_inserter(*this), count, value);
  }
  CircularBuffer(std::size_t capacity, std::initializer_list<T> list)
      : data_{alloc_capacity(capacity)}, capacity_{capacity} {
    std::copy(std::begin(list), std::end(list), std::back_inserter(*this));
  }

  ~CircularBuffer() {
    clear();
    delete[] data_;
  }

  CircularBuffer(const CircularBuffer& other)
      : data_{alloc_capacity(other.capacity())}, capacity_{other.capacity()} {
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
  }
  CircularBuffer& operator=(const CircularBuffer& other) {
    CircularBuffer buf{other};
    using std::swap;
    swap(*this, buf);
    return *this;
  }

  CircularBuffer(CircularBuffer&& other) {
    using std::swap;
    swap(*this, other);
  }
  CircularBuffer& operator=(CircularBuffer&& other) {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  bool empty() const noexcept { return size() == 0; }
  bool full() const noexcept { return size() == capacity(); }

  template <typename... TArgs>
  void emplace_back(TArgs... args) {
    glue_assert(size() < capacity());
    new (get_ptr(size())) T{std::forward<TArgs>(args)...};
    size_++;
  }

  void push_back(const T& other) { emplace_back(other); }

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

  void resize(std::size_t new_capacity) {
    if (new_capacity == capacity()) {
      return;
    }

    CircularBuffer buf{new_capacity};
    std::copy(begin(), begin() + std::min(size(), new_capacity),
              std::back_inserter(buf));
    using std::swap;
    swap(*this, buf);
  }

  constexpr std::size_t capacity() const noexcept { return capacity_; }
  constexpr std::size_t size() const noexcept { return size_; }

  iterator begin() { return {*this, 0}; }
  iterator end() { return {*this, size()}; }

  const_iterator begin() const noexcept { return cbegin(); }
  const_iterator end() const noexcept { return cend(); }

  const_iterator cbegin() const noexcept { return {*this, 0}; }
  const_iterator cend() const noexcept { return {*this, size()}; }

  T& operator[](std::size_t index) { return *get_ptr(index); }
  const T& operator[](std::size_t index) const { return *get_ptr(index); }

  friend void swap(CircularBuffer& a, CircularBuffer& b) {
    using std::swap;
    swap(a.data_, b.data_);
    swap(a.capacity_, b.capacity_);
    swap(a.begin_, b.begin_);
    swap(a.size_, b.size_);
  }

 private:
  constexpr std::size_t data_index(std::size_t index) const noexcept {
    return (begin_ + index) % capacity();
  }

  constexpr std::size_t capacity_bytes() const noexcept {
    return capacity() * sizeof(T);
  }

  T* get_ptr(std::size_t index) {
    return reinterpret_cast<T*>(data_) + data_index(index);
  }

  const T* get_ptr(std::size_t index) const {
    return reinterpret_cast<const T*>(data_) + data_index(index);
  }

  u8* alloc_capacity(std::size_t capacity) {
    // alignment ok!
    return new u8[capacity * sizeof(T)];
  }

 private:
  u8* data_ = nullptr;
  std::size_t capacity_ = 0;
  std::size_t begin_ = 0;
  std::size_t size_ = 0;
};
};  // namespace glue