#pragma once

#include <SDL.h>

#include <glue/assert.hpp>
#include <glue/debug/timer.hpp>
#include <glue/types.hpp>

namespace glue {
inline void limit_fps(f64 target_timestep_ms, const debug::Timer& frame_timer) {
  glue_assert(target_timestep_ms >= 0.0);

  /*
   * A kind of spinlock to pad out frame time to the desired timestep.
   *
   * We wait in discrete steps << total wait time to avoid OS scheduler
   * waiting too long to wake us. Then, we spin until we precisely hit
   * the target framerate plus some margin to allow time for buffer swap.
   */
  constexpr int sleep_step_ms = 1;
  constexpr int spinlock_time_ms = 4;
  constexpr f64 margin_ms = 0.4;
  while (frame_timer.elapsed_ms<f64>() < target_timestep_ms - margin_ms) {
    if (target_timestep_ms - frame_timer.elapsed_ms<f64>() > spinlock_time_ms) {
      SDL_Delay(sleep_step_ms);
    }
  }
}
}  // namespace glue