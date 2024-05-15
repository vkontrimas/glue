#include "shader.hpp"

#include <glog/logging.h>

namespace glue::gfx {
void ShaderProgram::link() {
  glLinkProgram(**this);

  GLint link_success = 0;
  glGetProgramiv(**this, GL_LINK_STATUS, &link_success);
  if (!link_success) {
    std::array<GLchar, 512> error;
    glGetProgramInfoLog(**this, error.size(), nullptr, error.data());
    LOG(FATAL) << "Shader program linking failed: " << error.data();
  }
}

void Shader::compile(const char* source) {
  glShaderSource(**this, 1, &source, nullptr);
  glCompileShader(**this);

  GLint compile_success = 0;
  glGetShaderiv(**this, GL_COMPILE_STATUS, &compile_success);
  if (!compile_success) {
    std::array<GLchar, 512> error;
    glGetShaderInfoLog(**this, error.size(), nullptr, error.data());
    LOG(FATAL) << "Shader compilation failed: " << error.data();
  }
}

Shader Shader::vertex(const char* source) {
  Shader shader{glCreateShader(GL_VERTEX_SHADER)};
  shader.compile(source);
  return shader;
}

Shader Shader::fragment(const char* source) {
  Shader shader{glCreateShader(GL_FRAGMENT_SHADER)};
  shader.compile(source);
  return shader;
}
}  // namespace glue::gfx