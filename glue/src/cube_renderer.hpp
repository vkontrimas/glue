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
  // TODO(vkon): pack this down
  // size and activity can easily fit in 32 bits
  struct Instance {
    vec3 position;
    f32 size;
    quat rotation;
    f32 activity;
  };

  static constexpr u32 kMaxInstances = 4096;

  CubeRenderer(
      const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block,
      const gfx::UniformBlock<uniforms::Lighting>& lighting_block);

  void draw(std::span<Instance> instances);

 private:
  gfx::VertexBuffer vbo_;
  gfx::ElementBuffer<GLubyte> ebo_;
  gfx::VertexArray vao_;
  gfx::ShaderProgram shader_;
  i32 cube_index_count_;
  gfx::VertexBuffer instance_buffer_;
};

}  // namespace glue