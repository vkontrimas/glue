#include <SDL.h>

#include <array>
#include <glue/physics.hpp>
#include <glue/world.hpp>

#include "cube_renderer.hpp"
#include "plane_renderer.hpp"
#include "window.hpp"

namespace glue {
namespace {
class Renderer {
 public:
  Renderer()
      : view_projection_uniforms_{0, {}},
        lighting_uniforms_{1, {}},
        cube_renderer_{view_projection_uniforms_, lighting_uniforms_},
        plane_renderer_{view_projection_uniforms_, lighting_uniforms_} {}

  void draw(const World& world) {
    {
      view_projection_uniforms_.bind_for_write();
      view_projection_uniforms_.set({world.camera});
    }

    view_projection_uniforms_.bind_for_shader();
    lighting_uniforms_.bind_for_shader();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    cube_renderer_.draw(world.cube);
    plane_renderer_.draw(world.ground);
  }

 private:
  gfx::UniformBlock<uniforms::ViewProjection> view_projection_uniforms_;
  gfx::UniformBlock<uniforms::Lighting> lighting_uniforms_;
  CubeRenderer cube_renderer_;
  PlaneRenderer plane_renderer_;
};
}  // namespace

void run() {
  auto window = create_window("Glue", 1280, 720);
  auto gl_context = init_gl(window.get());

  World world{};
  world.cube.position += vec3{0.0f, 10.0f, 0.0f};
  world.cube.rotation = glm::normalize(
      glm::angleAxis(35.0f, glm::normalize(vec3{1.0f, 0.5f, 2.0f})));
  world.camera.target = world.cube.position;

  Physics physics;
  physics.setup_static_objects(world);

  Renderer renderer;

  u64 previous_time = 0;
  u64 now = SDL_GetPerformanceCounter();

  {
    // draw the world once before showing the window
    renderer.draw(world);
    SDL_GL_SwapWindow(window.get());
    SDL_ShowWindow(window.get());
  }

  SDL_GL_SetSwapInterval(1);

  bool is_running = true;
  while (is_running) {
    previous_time = now;
    now = SDL_GetPerformanceCounter();

    float frame_delta_time = static_cast<float>(now - previous_time) /
                             static_cast<float>(SDL_GetPerformanceFrequency());
    LOG_EVERY_T(INFO, 0.5) << "dt: " << frame_delta_time * 1000.0f << "ms ("
                           << std::floor(1.0f / frame_delta_time) << " FPS)";

    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = false;
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE) {
        is_running = false;
      }
    }

    world.camera.target = world.cube.position;

    physics.update(frame_delta_time, world);

    renderer.draw(world);

    SDL_GL_SwapWindow(window.get());
  }
}
}  // namespace glue