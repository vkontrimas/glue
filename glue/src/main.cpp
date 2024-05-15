#include <SDL.h>
#include <glog/logging.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace glue {
void run();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  const auto sdl_init_error = SDL_Init(SDL_INIT_VIDEO);
  CHECK(!sdl_init_error) << SDL_GetError();

  glue::run();

  SDL_Quit();
  return 0;
}