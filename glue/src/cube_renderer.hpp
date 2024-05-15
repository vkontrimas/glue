#pragma once

#include "gfx/buffers.hpp"
#include "gfx/shader.hpp"
#include "gfx/uniform_block.hpp"
#include "gfx/vertex_array.hpp"
#include "uniforms/view_projection.hpp"

namespace glue {
class CubeRenderer {
 public:
  CubeRenderer(
      const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block);

  void draw(const Pose& pose);

 private:
  gfx::VertexBuffer vbo_;
  gfx::ElementBuffer<GLubyte> ebo_;
  gfx::VertexArray vao_;
  gfx::ShaderProgram shader_;
};

}  // namespace glue