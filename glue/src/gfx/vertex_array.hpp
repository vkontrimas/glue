#pragma once

#include <glad/gl.h>

#include <glue/resource.hpp>

namespace glue::gfx {
namespace detail {
struct VertexArrayObjectDeleter {
  void operator()(GLuint vao) { glDeleteVertexArrays(1, &vao); }
};
}  // namespace detail

using VertexArray = Resource<GLuint, detail::VertexArrayObjectDeleter>;
}  // namespace glue::gfx