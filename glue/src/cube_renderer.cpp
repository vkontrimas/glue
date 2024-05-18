#include "cube_renderer.hpp"

#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <vector>

namespace glue {
namespace {
struct CubeMesh {
  struct Vertex {
    vec3 position;
    vec3 normal;
  };

  std::vector<Vertex> vertices;
  std::vector<u8> indices;

  static CubeMesh load(const char* filename) {
    CubeMesh mesh;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filename, 0);
    CHECK(scene) << "failed to load mesh";
    CHECK(scene->mNumMeshes == 1) << "should have exactly one mesh";

    const aiMesh* loaded_mesh = scene->mMeshes[0];
    CHECK(loaded_mesh) << "mesh is null";
    CHECK(loaded_mesh->mVertices) << "mesh has no vertices";
    CHECK(loaded_mesh->mNormals) << "mesh has no vertices";
    CHECK(loaded_mesh->mFaces && loaded_mesh->mFaces->mIndices)
        << "mesh has no triangles";

    mesh.vertices.resize(loaded_mesh->mNumVertices);
    for (int i = 0; i < mesh.vertices.size(); ++i) {
      const aiVector3d& position = loaded_mesh->mVertices[i];
      mesh.vertices[i].position =
          vec3{static_cast<float>(position.x), static_cast<float>(position.y),
               static_cast<float>(position.z)};

      const aiVector3d& normal = loaded_mesh->mNormals[i];
      mesh.vertices[i].normal =
          vec3{static_cast<float>(normal.x), static_cast<float>(normal.y),
               static_cast<float>(normal.z)};
    }

    mesh.indices.reserve(36);
    for (int face_index = 0; face_index < loaded_mesh->mNumFaces;
         ++face_index) {
      for (int index_index = 0;
           index_index < loaded_mesh->mFaces[face_index].mNumIndices;
           ++index_index) {
        mesh.indices.push_back(static_cast<u8>(
            loaded_mesh->mFaces[face_index].mIndices[index_index]));
      }
    }

    return mesh;
  }
};

constexpr auto kVertexShader = R"shader(
#version 410 core

layout(std140) uniform view_projection_block {
  mat4 view_matrix;
  mat4 projection_matrix;
};

uniform mat4 model_matrix;
uniform float cube_scale;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 vert_normal;

void main() {
  // extract just the rotation and scale component
  // model matrix must be orthogonal in the top-left 3x3
  // ONLY UNIFORM SCALE IS ALLOWED!
  mat3 model_rotation_scale = mat3(model_matrix);
  vert_normal = model_rotation_scale * normal;
  gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position * cube_scale, 1.0);
}
)shader";

constexpr auto kFragmentShader = R"shader(
#version 410 core

layout(std140) uniform lighting_block {
  vec4 ambient_light;
  vec4 sun_direction;
  vec4 sun_light;
};

uniform float activity_level;

in vec3 vert_normal;

out vec4 frag_color;

void main() {
  vec4 albedo = mix(vec4(0.95, 0.95, 0.95, 1.0), vec4(0.909803, 0.282352, 0.33333, 1.0), activity_level);
  vec4 light = ambient_light + dot(vert_normal, sun_direction.xyz) * sun_light;
  frag_color = light * albedo;
}
)shader";
}  // namespace

CubeRenderer::CubeRenderer(
    const gfx::UniformBlock<uniforms::ViewProjection>& view_projection_block,
    const gfx::UniformBlock<uniforms::Lighting>& lighting_block)
    : shader_{
          gfx::ShaderProgram::from(gfx::Shader::vertex(kVertexShader),
                                   gfx::Shader::fragment(kFragmentShader))} {
  view_projection_block.connect(shader_, "view_projection_block");
  lighting_block.connect(shader_, "lighting_block");
  model_uniform_ = shader_.uniform_location("model_matrix");
  scale_uniform_ = shader_.uniform_location("cube_scale");
  activity_uniform_ = shader_.uniform_location("activity_level");

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

  auto mesh = CubeMesh::load("assets/cube.glb");
  cube_index_count_ = mesh.indices.size();

  vbo_.bind();
  vbo_.data(std::span{mesh.vertices}, GL_STATIC_DRAW);

  ebo_.bind();
  ebo_.data(std::span{mesh.indices}, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CubeMesh::Vertex), 0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CubeMesh::Vertex),
                        (GLvoid*)sizeof(vec3));
}

void CubeRenderer::draw(const Pose& pose, float width, float activity) {
  glUseProgram(*shader_);

  const mat4 model_matrix = pose.model_matrix();
  glUniformMatrix4fv(model_uniform_, 1, GL_FALSE, glm::value_ptr(model_matrix));

  glUniform1f(scale_uniform_, width);
  glUniform1f(activity_uniform_, activity);

  glBindVertexArray(*vao_);
  glDrawElements(GL_TRIANGLES, cube_index_count_, ebo_.Type, 0);
}

}  // namespace glue