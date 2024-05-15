#pragma once

#include <glog/logging.h>

#include "buffers.hpp"
#include "shader.hpp"

namespace glue::gfx {
template <typename T>
class UniformBlock {
 public:
  UniformBlock(GLuint binding) : UniformBlock{binding, {}} {}

  UniformBlock(GLuint binding, const T& value)
      : binding_{binding}, buffer_{UniformBuffer<T>::create()} {
    buffer_.bind();
    buffer_.set(value);
  }

  void bind_for_shader() {
    glBindBufferBase(buffer_.Target, binding_, *buffer_);
  }
  void bind_for_write() { buffer_.bind(); }

  void set(const T& value) {
    CHECK_NE(*buffer_, 0) << "buffer not initialized in ctor";
    glBufferSubData(buffer_.Target, 0, sizeof(T), &value);
  }

  void connect(ShaderProgram& shader, const char* block_name) const {
    const auto block_index = shader.uniform_block_index(block_name);
    glUniformBlockBinding(*shader, block_index, binding_);
  }

 private:
  GLuint binding_;
  UniformBuffer<T> buffer_;
};
}  // namespace glue::gfx