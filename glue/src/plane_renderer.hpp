#pragma once

#include "gfx/buffers.hpp"
#include "gfx/shader.hpp"
#include "gfx/texture.hpp"
#include "gfx/uniform_block.hpp"
#include "gfx/vertex_array.hpp"
#include "uniforms/lighting.hpp"
#include "uniforms/view_projection.hpp"

namespace glue {
class PlaneRenderer {
 public:
  PlaneRenderer(
      const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block,
      const gfx::UniformBlock<uniforms::Lighting>& lighting_block);

  void draw(const Plane& plane);

 private:
  gfx::VertexBuffer vbo_;
  gfx::VertexArray vao_;
  gfx::ShaderProgram shader_;
  gfx::Texture texture_;
  GLint texture_unit_;
  GLint model_matrix_uniform_;
};
}  // namespace glue