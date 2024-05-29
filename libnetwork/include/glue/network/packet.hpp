#pragma once

#include <algorithm>
#include <glue/assert.hpp>
#include <glue/pointers.hpp>
#include <glue/types.hpp>
#include <span>
#include <type_traits>

namespace glue::network {
/*
 * VARIABLE SIZE class.
 *
 * To enable reconfiguring packet sizes at runtime for testing.
 * In prod, I'd probably have statically sized packets.
 *
 * size_bytes = size of the data segment in bytes
 * index = packet index
 *
 * both of these could be reduced in size in the future for better packing.
 * KISS for now.
 */
class Packet final {
 public:
  static constexpr std::size_t kAlignment = sizeof(u32);

  using value_type = u8;

 public:
  constexpr Packet() = default;

  static constexpr std::size_t alloc_size_bytes(u32 packet_size) noexcept {
    const std::size_t desired_size = sizeof(Packet) - sizeof(data_) +
                                     std::max(kDefaultDataSize, packet_size);

    // compute aligned size by extending past the next padding boundary,
    // then mask off the least significant bits to align the end with the same
    // padding boundary.
    const std::size_t aligned_size =
        (desired_size + kAlignment - 1) & ~(kAlignment - 1);
    return aligned_size;
  }

  static Packet* unsafe_init(u8* start, u32 size, u32 index) {
    glue_assert(ptr_is_aligned(start, kAlignment));
    return new (start) Packet{size, index};
  }

  constexpr u32 size_bytes() const noexcept { return size_bytes_; }

  constexpr u32 index() const noexcept { return index_; }

  constexpr u8* data() noexcept { return data_; }
  constexpr const u8* data() const noexcept { return data_; }

  constexpr u8* begin() noexcept { return data(); }
  constexpr u8* end() noexcept { return data() + size_bytes(); }

  constexpr const u8* begin() const noexcept { return data(); }
  constexpr const u8* end() const noexcept { return data() + size_bytes(); }

  constexpr const u8* cbegin() const noexcept { return data(); }
  constexpr const u8* cend() const noexcept { return data() + size_bytes(); }

 private:
  static constexpr u32 kDefaultDataSize = sizeof(u32);

  constexpr Packet(u32 size_bytes, u32 index) noexcept
      : size_bytes_{size_bytes}, index_{index} {}

 private:
  u32 size_bytes_;
  u32 index_;

  u8 data_[kDefaultDataSize];
};

static_assert(sizeof(Packet) == Packet::alloc_size_bytes(0));
static_assert(alignof(Packet) == Packet::kAlignment,
              "Packet alignment sanity check.");
static_assert(
    std::is_trivial_v<Packet>,
    "Packet must be trivial. It is involved in many unsafe shenanigans.");
static_assert(std::is_standard_layout_v<Packet>);
};  // namespace glue::network