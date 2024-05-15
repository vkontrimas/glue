#pragma once

#include <glad/gl.h>

#include <glue/resource.hpp>

namespace glue::gfx {
namespace detail {
struct ShaderDeleter {
  void operator()(GLuint shader) { glDeleteShader(shader); }
};

struct ProgramDeleter {
  void operator()(GLuint program) { glDeleteProgram(program); }
};
}  // namespace detail

struct Shader : public Resource<GLuint, detail::ShaderDeleter> {
  static Shader vertex(const char* source);
  static Shader fragment(const char* source);

 private:
  void compile(const char* source);
};

struct ShaderProgram : public Resource<GLuint, detail::ProgramDeleter> {
  template <typename... Tn>
  static ShaderProgram from(Tn&&... shaders) {
    auto arr = {std::forward<Tn>(shaders)...};
    return from_iter(std::begin(arr), std::end(arr));
  }

  void use() { glUseProgram(**this); }
  void link();

  GLint uniform_location(const char* name) {
    return glGetUniformLocation(**this, name);
  }

 private:
  template <typename T>
  static ShaderProgram from_iter(T begin, T end) {
    ShaderProgram program{glCreateProgram()};
    for (auto it = begin; it != end; ++it) {
      glAttachShader(*program, **it);
    }
    program.link();
    return program;
  }
};
}  // namespace glue::gfx