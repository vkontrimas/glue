#include <SDL.h>
#include <glog/logging.h>

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  const auto sdl_init_error = SDL_Init(SDL_INIT_VIDEO);
  CHECK(sdl_init_error == 0) << "Failed to initialize SDL: " << SDL_GetError();

  SDL_Window* window =
      SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       1280, 720, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
  CHECK(window) << "Couldn't create window: " << SDL_GetError();

  bool is_running = true;
  while (is_running) {
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = false;
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE) {
        is_running = false;
      }
    }

    SDL_GL_SwapWindow(window);
  }

  SDL_Quit();
  return 0;
}