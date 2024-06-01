#pragma once

#include <bitset>
#include <concepts>
#include <glue/assert.hpp>
#include <glue/bitpack/pack_bits.hpp>
#include <glue/types.hpp>

namespace glue::bitpack {
template <typename V, std::floating_point L, std::integral B>
  requires std::same_as<std::remove_reference_t<V>, L>
inline constexpr void pack_quantize(Packer& packer, V&& value, L min, L max,
                                    B bits) {
  glue_assert(bits > 0);
  glue_assert(value >= min);
  glue_assert(value <= max);

  const auto range = max - min;
  const auto t = (value - min) / range;

  const u64 bit_range = (1ull << bits) - 1;
  u32 store = t * static_cast<L>(bit_range);
  packer.write_bits(store, bits);
}

template <typename V, std::floating_point L, std::integral B>
  requires std::same_as<std::remove_reference_t<V>, L>
inline constexpr void pack_quantize(Unpacker& unpacker, V&& value, L min, L max,
                                    B bits) {
  glue_assert(bits > 0);

  u32 store = 0;
  pack_bits(unpacker, store, 0, bits);

  const L t = store / static_cast<L>((1ull << bits) - 1);
  const auto range = max - min;
  value = min + range * t;
}

template <CPacker T, typename V, std::floating_point L, std::integral B>
  requires std::same_as<std::remove_reference_t<V>, L>
inline constexpr void pack_quantize_clamp(T& packer, V&& value, L min, L max,
                                          B bits) {
  value = glm::clamp(value, min, max);
  pack_quantize(packer, value, min, max, bits);
}
}  // namespace glue::bitpack