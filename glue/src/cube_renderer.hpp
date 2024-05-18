#pragma once

#include "gfx/buffers.hpp"
#include "gfx/shader.hpp"
#include "gfx/uniform_block.hpp"
#include "gfx/vertex_array.hpp"
#include "uniforms/lighting.hpp"
#include "uniforms/view_projection.hpp"

namespace glue {
class CubeRenderer {
 public:
  CubeRenderer(
      const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block,
      const gfx::UniformBlock<uniforms::Lighting>& lighting_block);

  void draw(const Pose& pose, float width, float activity);

 private:
  gfx::VertexBuffer vbo_;
  gfx::ElementBuffer<GLubyte> ebo_;
  gfx::VertexArray vao_;
  gfx::ShaderProgram shader_;
  GLint model_uniform_;
  GLint scale_uniform_;
  GLint activity_uniform_;
  i32 cube_index_count_;
};

}  // namespace glue