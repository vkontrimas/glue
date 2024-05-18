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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 2) in vec3 inst_position;
layout(location = 3) in float inst_scale;
layout(location = 4) in vec4 inst_rotation;
layout(location = 5) in float inst_activity;

out vec3 vert_normal;
out float activity_level;

vec4 quat_mult(vec4 left, vec4 right) {
  return vec4(
    cross(left.xyz, right.xyz) + right.w * left.xyz + left.w * right.xyz, 
    left.w * right.w - dot(left.xyz, right.xyz)
  );
}

vec4 quat_conjugate(vec4 quat) {
  return vec4(-quat.xyz, quat.w);
}

vec4 quat_rotate_vec(vec4 quat, vec4 vector) {
  vec4 conj = quat_conjugate(quat);
  return quat_mult(quat, quat_mult(vector, conj));
}

void main() {
  activity_level = inst_activity;
  vert_normal = quat_rotate_vec(inst_rotation, vec4(normal, 1.0)).xyz;
  vec4 vertex_position = vec4(quat_rotate_vec(inst_rotation, vec4(position, 1.0)).xyz * inst_scale + inst_position, 1.0);
  gl_Position = projection_matrix * view_matrix * vertex_position;
}
)shader";

constexpr auto kFragmentShader = R"shader(
#version 410 core

layout(std140) uniform lighting_block {
  vec4 ambient_light;
  vec4 sun_direction;
  vec4 sun_light;
};

in vec3 vert_normal;
in float activity_level;

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

  {
    GLuint buffers[3];
    glGenBuffers(3, buffers);

    vbo_ = gfx::VertexBuffer{buffers[0]};
    ebo_ = gfx::ElementBuffer<GLubyte>{buffers[1]};
    instance_buffer_ = gfx::VertexBuffer{buffers[2]};
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

  instance_buffer_.bind();
  instance_buffer_.init_to_size(sizeof(Instance) * kMaxInstances,
                                GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), 0);
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Instance),
                        (GLvoid*)sizeof(vec3));
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Instance),
                        (GLvoid*)(sizeof(vec3) + sizeof(f32)));
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Instance),
                        (GLvoid*)(sizeof(vec3) + sizeof(f32) + sizeof(vec4)));

  glVertexAttribDivisor(2, 1);
  glVertexAttribDivisor(3, 1);
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
}

void CubeRenderer::draw(std::span<CubeRenderer::Instance> instances) {
  auto instance_count = instances.size();
  if (instance_count > kMaxInstances) {
    LOG(ERROR) << "too many instances for cube renderer. given: "
               << instances.size() << " limit: " << kMaxInstances;
    instance_count = kMaxInstances;
  }

  glUseProgram(*shader_);

  instance_buffer_.bind();
  glBufferSubData(instance_buffer_.Target, 0, sizeof(Instance) * kMaxInstances,
                  instances.data());

  glBindVertexArray(*vao_);
  glDrawElementsInstanced(GL_TRIANGLES, cube_index_count_, ebo_.Type, nullptr,
                          instance_count);
}

}  // namespace glue