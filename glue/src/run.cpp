#include <SDL.h>

#include <array>
#include <glue/world.hpp>

#include "gfx/buffers.hpp"
#include "gfx/shader.hpp"
#include "gfx/uniform_block.hpp"
#include "gfx/vertex_array.hpp"
#include "uniforms/view_projection.hpp"
#include "window.hpp"

namespace {
constexpr auto kVertexShader = R"shader(
#version 410 core

layout(std140) uniform view_projection_block {
  mat4 view_matrix;
  mat4 projection_matrix;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

void main() {
  gl_Position = projection_matrix * view_matrix * vec4(position, 1.0);
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
  CubeRenderer(
      const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block)
      : shader_{
            gfx::ShaderProgram::from(gfx::Shader::vertex(kVertexShader),
                                     gfx::Shader::fragment(kFragmentShader))} {
    view_projection_block.connect(shader_, "view_projection_block");

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

    struct Vertex {
      vec3 position;
      vec3 normal;
    };

    std::array<Vertex, 24> vertices{{
        // Bottom face (y = -1.0f)
        {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}},

        // Top face (y = 1.0f)
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},

        // Front face (z = 1.0f)
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        // Back face (z = -1.0f)
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
        {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}},

        // Left face (x = -1.0f)
        {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}},
        {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},

        // Right face (x = 1.0f)
        {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    }};

    std::array<GLubyte, 36> triangles{// Bottom face (y = -1.0f)
                                      0, 1, 2, 2, 3, 0,

                                      // Top face (y = 1.0f)
                                      4, 5, 6, 6, 7, 4,

                                      // Front face (z = 1.0f)
                                      8, 9, 10, 10, 11, 8,

                                      // Back face (z = -1.0f)
                                      12, 13, 14, 14, 15, 12,

                                      // Left face (x = -1.0f)
                                      16, 17, 18, 18, 19, 16,

                                      // Right face (x = 1.0f)
                                      20, 21, 22, 22, 23, 20};

    vbo_.bind();
    vbo_.data(std::span{vertices}, GL_STATIC_DRAW);

    ebo_.bind();
    ebo_.data(std::span{triangles}, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid*)sizeof(vec3));
  }

  void draw(const Pose& pose) {
    glUseProgram(*shader_);
    glBindVertexArray(*vao_);
    glDrawElements(GL_TRIANGLES, 3 * 2 * 6, ebo_.Type, 0);
  }

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

  gfx::UniformBlock<uniforms::ViewProjection> view_projection_uniforms{
      0, {world.camera}};

  CubeRenderer cube_renderer{view_projection_uniforms};

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

    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    cube_renderer.draw(world.cube);

    SDL_GL_SwapWindow(window.get());
  }
}
}  // namespace glue