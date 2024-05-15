#include "plane_renderer.hpp"

#include <stb_image.h>

namespace glue {
namespace {
constexpr auto kVertexShader = R"shader(
#version 410 core

layout(std140) uniform view_projection_block {
  mat4 view_matrix;
  mat4 projection_matrix;
};

uniform mat4 model_matrix;

layout(location = 0) in vec2 position;

out vec3 vert_normal;
out vec2 vert_uv;

void main() {
  mat3 rotation_scale = mat3(model_matrix);
  vert_normal = normalize(rotation_scale * vec3(0, 1, 0));

  {
    vec3 pos = rotation_scale * vec3(position.x + 0.5f, 0, -(position.y - 0.5f));
    vec3 local_x = normalize(rotation_scale * vec3(1, 0, 0));
    vec3 local_z = normalize(rotation_scale * vec3(0, 0, -1));
    vert_uv = vec2(dot(local_x, pos), dot(local_z, pos));
  }

  gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position.x, 0, position.y, 1);
}
)shader";

constexpr auto kFragmentShader = R"shader(
#version 410 core

layout(std140) uniform lighting_block {
  vec4 ambient_light;
  vec4 sun_direction;
  vec4 sun_light;
};

uniform sampler2D albedo_texture;

in vec3 vert_normal;
in vec2 vert_uv;

out vec4 frag_color;

void main() {
  vec4 albedo = texture(albedo_texture, vert_uv);
  vec4 light = ambient_light + dot(vert_normal, sun_direction.xyz) * sun_light;
  frag_color = light * albedo;
}
)shader";

void load_texture(gfx::Texture& texture) {
  int width, height, channels;
  const auto image =
      stbi_load("assets/floor.png", &width, &height, &channels, 3);
  CHECK(image) << "failed to load floor texture: " << stbi_failure_reason();
  CHECK_EQ(channels, 3) << "expected 3 channels in floor texture";

  glBindTexture(GL_TEXTURE_2D, *texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);

  stbi_image_free(image);
}
}  // namespace

PlaneRenderer::PlaneRenderer(
    const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block,
    const gfx::UniformBlock<uniforms::Lighting>& lighting_block)
    : vbo_{gfx::VertexBuffer::create()},
      vao_{gfx::VertexArray::create()},
      shader_{gfx::ShaderProgram::from(gfx::Shader::vertex(kVertexShader),
                                       gfx::Shader::fragment(kFragmentShader))},
      texture_{gfx::Texture::create()},
      texture_unit_{0} {
  view_projection_block.connect(shader_, "view_projection_block");
  lighting_block.connect(shader_, "lighting_block");

  model_matrix_uniform_ = shader_.uniform_location("model_matrix");

  {
    // set sampler texture unit once
    shader_.use();
    const auto sampler_uniform = shader_.uniform_location("albedo_texture");
    glUniform1i(sampler_uniform, texture_unit_);
  }

  load_texture(texture_);

  struct Vertex {
    vec2 position;
  };

  std::array<Vertex, 4> vertices{{
      {{-0.5f, 0.5f}},
      {{0.5f, 0.5f}},
      {{0.5f, -0.5f}},
      {{-0.5f, -0.5f}},
  }};

  vao_.bind();

  vbo_.bind();
  vbo_.data(std::span{vertices}, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
}

void PlaneRenderer::draw(const Plane& plane) {
  shader_.use();

  glActiveTexture(GL_TEXTURE0 + texture_unit_);
  glBindTexture(GL_TEXTURE_2D, *texture_);

  const auto model_matrix =
      glm::scale(glm::translate(glm::identity<mat4>(), plane.pose.position),
                 vec3{plane.size}) *
      glm::toMat4(plane.pose.rotation);
  glUniformMatrix4fv(model_matrix_uniform_, 1, GL_FALSE,
                     glm::value_ptr(model_matrix));

  vao_.bind();
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

}  // namespace glue