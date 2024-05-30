#pragma once

#include <concepts>
#include <glue/assert.hpp>
#include <glue/types.hpp>
#include <span>

namespace glue::bitpack::detail {
class BasePacker {
 public:
  using size_t = std::size_t;
  using value_t = u32;

  static constexpr size_t kValueSizeBits = sizeof(value_t) * 8;

 public:
  constexpr BasePacker() = default;
  virtual ~BasePacker() = default;

  explicit constexpr BasePacker(std::span<value_t> data) noexcept
      : data_{data} {}

  constexpr size_t capacity() const noexcept { return data_.size(); }
  constexpr size_t capacity_bits() const noexcept {
    return capacity() * kValueSizeBits;
  }

  constexpr size_t current() const noexcept {
    return current_bit() / kValueSizeBits;
  }
  constexpr size_t current_bit() const noexcept { return bit_position_; }

  constexpr size_t next() const noexcept { return current() + 1; }

 protected:
  std::span<value_t> data_{};
  size_t bit_position_{0};
};
}  // namespace glue::bitpack::detail

namespace glue::bitpack {
struct Packer final : public detail::BasePacker {
  constexpr Packer() = default;
  explicit constexpr Packer(std::span<value_t> data) noexcept
      : detail::BasePacker{data} {}

  /*
   * Write count bits and advance position.
   */
  constexpr void write_bits(value_t value, size_t count) {
    glue_assert(count <= kValueSizeBits);
    glue_assert(current_bit() + count <= capacity_bits());

    value &= static_cast<value_t>((1ull << count) - 1);

    constexpr auto kAlignMask = kValueSizeBits - 1;
    const size_t space = kValueSizeBits - (current_bit() & kAlignMask);
    if (space >= count) {
      const auto shift = space - count;
      data_[current()] |= value << shift;
      bit_position_ += count;
    } else {
      const value_t high_value = value >> (count - space);
      write_bits(high_value, space);
      write_bits(value, count - space);
    }
  }
};

struct Unpacker final : public detail::BasePacker {
  constexpr Unpacker() = default;
  explicit constexpr Unpacker(std::span<value_t> data) noexcept
      : detail::BasePacker{data} {}

  /*
   * Read count bits and advance position.
   */
  constexpr value_t read_bits(size_t count) {
    glue_assert(count <= kValueSizeBits);
    glue_assert(current_bit() + count <= capacity_bits());

    const value_t value_mask = static_cast<value_t>((1ull << count) - 1);

    constexpr auto kAlignMask = kValueSizeBits - 1;
    const size_t space = kValueSizeBits - (current_bit() & kAlignMask);
    if (space >= count) {
      const auto shift = space - count;
      const value_t value = (data_[current()] >> shift) & value_mask;
      bit_position_ += count;
      return value;
    } else {
      const value_t high_mask = (1 << space) - 1;
      const value_t high_value = read_bits(space);
      const value_t low_value = read_bits(count - space);
      return (high_value << (count - space)) | low_value;
    }
  }
};

template <class T>
concept CPacker = std::derived_from<T, detail::BasePacker>;
}  // namespace glue::bitpack