#include <SDL.h>
#include <glog/logging.h>

#include <CLI11.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "run.hpp"

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  CLI::App cli;

  glue::RunOptions options{};
  cli.add_option("--window-x", options.window_position_x, "Window x position");
  cli.add_option("--window-y", options.window_position_y, "Window y position");
  cli.add_option("--width", options.window_width, "Window width");
  cli.add_option("--height", options.window_width, "Window height");
  CLI11_PARSE(cli, argc, argv);

  const auto sdl_init_error = SDL_Init(SDL_INIT_VIDEO);
  CHECK(!sdl_init_error) << SDL_GetError();

  glue::run(options);

  SDL_Quit();
  return 0;
}