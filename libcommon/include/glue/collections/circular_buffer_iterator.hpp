#pragma once

#include <concepts>

namespace glue::collections {
template <typename TBuffer, typename T, typename TPtr, typename TRef>
class CircularBufferIterator final {
 public:
  using iterator_category =
      std::bidirectional_iterator_tag;  // TODO: make this random_access
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = TPtr;
  using reference = TRef;

  CircularBufferIterator(TBuffer buffer, std::size_t index) noexcept
      : buffer_{buffer}, index_{index} {}

  reference operator*() { return buffer_[index_]; }
  const reference operator*() const { return buffer_[index_]; }

  pointer operator->() { return &buffer_[index_]; }
  const pointer operator->() const { return &buffer_[index_]; }

  CircularBufferIterator& operator++() {
    ++index_;
    return *this;
  }

  CircularBufferIterator& operator--() {
    glue_assert(index_ > 0);
    --index_;
    return *this;
  }

  CircularBufferIterator operator++(int) {
    CircularBufferIterator temp = *this;
    ++index_;
    return temp;
  }

  CircularBufferIterator operator--(int) {
    glue_assert(index_ > 0);
    CircularBufferIterator temp = *this;
    --index_;
    return temp;
  }

  CircularBufferIterator operator+(std::size_t val) {
    return {buffer_, index_ + val};
  }

  friend bool operator==(const CircularBufferIterator& a,
                         const CircularBufferIterator& b) {
    return &a.buffer_ == &b.buffer_ && a.index_ == b.index_;
  }

  friend bool operator!=(const CircularBufferIterator& a,
                         const CircularBufferIterator& b) {
    return !(a == b);
  }

 private:
  TBuffer buffer_;
  std::size_t index_ = 0;
};

};  // namespace glue::collections