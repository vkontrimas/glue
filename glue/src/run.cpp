#include <SDL.h>

#include <array>
#include <glue/world.hpp>

#include "gfx/buffers.hpp"
#include "gfx/shader.hpp"
#include "gfx/vertex_array.hpp"
#include "window.hpp"

namespace {
constexpr auto kVertexShader = R"shader(
#version 410 core

layout(location = 0) in vec3 position;

void main() {
  gl_Position = vec4(position, 1.0);
}
)shader";

constexpr auto kFragmentShader = R"shader(
#version 410 core

out vec4 frag_color;

void main() {
  frag_color = vec4(1.0);
}
)shader";
}  // namespace

namespace glue {
class CubeRenderer {
 public:
  CubeRenderer()
      : shader_{
            gfx::ShaderProgram::from(gfx::Shader::vertex(kVertexShader),
                                     gfx::Shader::fragment(kFragmentShader))} {
    {
      GLuint buffers[2];
      glGenBuffers(2, buffers);

      vbo_ = gfx::VertexBuffer{buffers[0]};
      ebo_ = gfx::ElementBuffer<GLubyte>{buffers[1]};
    }

    {
      GLuint vao;
      glGenVertexArrays(1, &vao);

      vao_ = gfx::VertexArray{vao};
    }

    glBindVertexArray(*vao_);

    // TODO(vkon): mesh data!
    std::array<vec3, 8> vertices{};
    std::array<GLubyte, 3 * 2 * 6> triangles{};

    vbo_.bind();
    vbo_.data(std::span{vertices}, GL_STATIC_DRAW);

    ebo_.bind();
    ebo_.data(std::span{triangles}, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
  }

  void draw(const Pose& pose) {}

 private:
  gfx::VertexBuffer vbo_;
  gfx::ElementBuffer<GLubyte> ebo_;
  gfx::VertexArray vao_;
  gfx::ShaderProgram shader_;
};

void run() {
  auto window = create_window("Glue", 1280, 720);
  auto gl_context = init_gl(window.get());
  World world{};

  CubeRenderer cube_renderer;

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

    cube_renderer.draw(world.cube);

    SDL_GL_SwapWindow(window.get());
  }
}
}  // namespace glue