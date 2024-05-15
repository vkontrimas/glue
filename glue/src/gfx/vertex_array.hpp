#pragma once

#include <glad/gl.h>

#include <glue/resource.hpp>

namespace glue::gfx {
namespace detail {
struct VertexArrayObjectDeleter {
  void operator()(GLuint vao) { glDeleteVertexArrays(1, &vao); }
};
}  // namespace detail

struct VertexArray : public Resource<GLuint, detail::VertexArrayObjectDeleter> {
  static VertexArray create() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return {vao};
  }

  void bind() { glBindVertexArray(**this); }
};
}  // namespace glue::gfx