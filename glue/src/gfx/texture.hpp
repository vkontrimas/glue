#pragma once

#include <glad/gl.h>

#include <glue/resource.hpp>

namespace glue::gfx {
namespace detail {
struct TextureDeleter {
  void operator()(GLuint texture) { glDeleteTextures(1, &texture); }
};
}  // namespace detail

struct Texture : public Resource<GLuint, detail::TextureDeleter> {
  static Texture create() {
    GLuint texture;
    glGenTextures(1, &texture);
    return {texture};
  }
};
}  // namespace glue::gfx