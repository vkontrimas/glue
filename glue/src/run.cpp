#include <SDL.h>

#include <glue/world.hpp>

#include "window.hpp"

namespace glue {
void run() {
  auto window = create_window("Glue", 1280, 720);
  auto gl_context = init_gl(window.get());
  World world{};

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

    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_GL_SwapWindow(window.get());
  }
}
}  // namespace glue