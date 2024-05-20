#pragma once

namespace glue {
struct RunOptions {
  int window_position_x = -1;
  int window_position_y = -1;
  int window_width = 1280;
  int window_height = 720;
};

void run(const RunOptions& options);
}  // namespace glue