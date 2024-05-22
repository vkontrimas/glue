#pragma once

#include <algorithm>
#include <array>
#include <glue/assert.hpp>
#include <glue/types.hpp>
#include <iterator>

namespace glue {
template <typename T, std::size_t Capacity>
class CircularBuffer final {
 public:
  template <typename TBuffer, typename TPtr, typename TRef>
  class Iterator {
   public:
    using iterator_category =
        std::bidirectional_iterator_tag;  // TODO: make this random_access
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = TPtr;
    using reference = TRef;

    Iterator(TBuffer buffer, std::size_t index) noexcept
        : buffer_{buffer}, index_{index} {}

    reference operator*() { return *buffer_.get_ptr(index_); }
    const reference operator*() const { return *buffer_.get_ptr(index_); }

    pointer operator->() { return buffer_.get_ptr(index_); }
    const pointer operator->() const { return buffer_.get_ptr(index_); }

    Iterator& operator++() {
      ++index_;
      return *this;
    }

    Iterator& operator--() {
      glue_assert(index_ > 0);
      --index_;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp = *this;
      ++index_;
      return temp;
    }

    Iterator operator--(int) {
      glue_assert(index_ > 0);
      Iterator temp = *this;
      --index_;
      return temp;
    }

    friend bool operator==(const Iterator<TBuffer, TPtr, TRef>& a,
                           const Iterator<TBuffer, TPtr, TRef>& b) {
      return &a.buffer_ == &b.buffer_ && a.index_ == b.index_;
    }

    friend bool operator!=(const Iterator<TBuffer, TPtr, TRef>& a,
                           const Iterator<TBuffer, TPtr, TRef>& b) {
      return !(a == b);
    }

   private:
    TBuffer buffer_;
    std::size_t index_ = 0;
  };

 public:
  using value_type = T;
  using iterator = Iterator<CircularBuffer&, T*, T&>;
  using const_iterator = Iterator<const CircularBuffer&, const T*, const T&>;

  static constexpr std::size_t capacity() noexcept { return Capacity; }

  CircularBuffer() noexcept {}
  CircularBuffer(std::initializer_list<T> list) {
    glue_assert(list.size() <= capacity());
    std::copy(std::begin(list), std::end(list), std::back_inserter(*this));
  }
  CircularBuffer(std::size_t count, const T& value) {
    glue_assert(count <= capacity());
    std::fill_n(std::back_inserter(*this), count, value);
  }

  ~CircularBuffer() {
    for (auto it = begin(); it != end(); ++it) {
      it->~T();
    }
  }

  CircularBuffer(const CircularBuffer& other) {
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
  }
  CircularBuffer& operator=(const CircularBuffer& other) {
    clear();
    std::copy(std::begin(other), std::end(other), std::back_inserter(*this));
    return *this;
  }

  CircularBuffer(CircularBuffer&& other) noexcept {
    using std::swap;
    swap(*this, other);
  }
  CircularBuffer& operator=(CircularBuffer&& other) noexcept {
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

  friend void swap(CircularBuffer& a, CircularBuffer& b) {
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