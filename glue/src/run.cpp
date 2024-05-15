#include <SDL.h>

#include <array>
#include <glue/world.hpp>

#include "cube_renderer.hpp"
#include "plane_renderer.hpp"
#include "window.hpp"

namespace glue {
void run() {
  auto window = create_window("Glue", 1280, 720);
  auto gl_context = init_gl(window.get());

  World world{};
  world.ground.size = 400.0f;

  gfx::UniformBlock<uniforms::ViewProjection> view_projection_uniforms{
      0, {world.camera}};

  gfx::UniformBlock<uniforms::Lighting> lighting_uniforms{1, {}};

  CubeRenderer cube_renderer{view_projection_uniforms, lighting_uniforms};
  PlaneRenderer plane_renderer{view_projection_uniforms, lighting_uniforms};

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

    {
      view_projection_uniforms.bind_for_write();
      view_projection_uniforms.set({world.camera});
    }

    view_projection_uniforms.bind_for_shader();
    lighting_uniforms.bind_for_shader();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    cube_renderer.draw(world.cube);
    plane_renderer.draw(world.ground);

    SDL_GL_SwapWindow(window.get());
  }
}
}  // namespace glue