#pragma once

#include <glad/gl.h>

#include <concepts>
#include <glue/resource.hpp>
#include <span>

namespace glue::gfx {
namespace detail {
struct BufferObjectDeleter {
  void operator()(GLuint bo) { glDeleteBuffers(1, &bo); }
};

using BufferObject = Resource<GLuint, detail::BufferObjectDeleter>;

template <GLenum TARGET>
struct TypedBufferObject : public BufferObject {
  static constexpr GLenum Target = TARGET;

  void bind() { glBindBuffer(Target, **this); }

  // TODO(vkon): replace these with functions that take lambdas to ensure buffer
  // is bound before ops?
  template <typename T, std::size_t N>
  void data(std::span<T, N> data, GLenum usage) {
    assert(usage == GL_STATIC_DRAW || usage == GL_DYNAMIC_DRAW ||
           usage == GL_STREAM_DRAW);
    glBufferData(Target, data.size_bytes(), data.data(), usage);
  }

  void init_to_size(GLsizei size, GLenum usage) {
    assert(usage == GL_STATIC_DRAW || usage == GL_DYNAMIC_DRAW ||
           usage == GL_STREAM_DRAW);
    glBufferData(Target, size, nullptr, usage);
  }

 protected:
  static GLuint create_buffer() {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    return buffer;
  }
};

template <typename T>
struct ElementTypeGLenum;

template <>
struct ElementTypeGLenum<GLubyte> {
  static constexpr GLenum value = GL_UNSIGNED_BYTE;
};

template <>
struct ElementTypeGLenum<GLushort> {
  static constexpr GLenum value = GL_UNSIGNED_SHORT;
};

template <>
struct ElementTypeGLenum<GLuint> {
  static constexpr GLenum value = GL_UNSIGNED_INT;
};

}  // namespace detail

// TODO(vkon): would be cool to enforce vertex layout + VAO + shader
// compatibility via the type system!
struct VertexBuffer : public detail::TypedBufferObject<GL_ARRAY_BUFFER> {
  static VertexBuffer create() { return {create_buffer()}; }
};

template <typename T>
concept ElementType = requires(T a) {
  { detail::ElementTypeGLenum<T>::value } -> std::convertible_to<GLenum>;
};

template <ElementType T>
struct ElementBuffer
    : public detail::TypedBufferObject<GL_ELEMENT_ARRAY_BUFFER> {
  static constexpr GLenum Type = detail::ElementTypeGLenum<T>::value;

  static ElementBuffer create() { return {create_buffer()}; }
};

template <typename T>
struct UniformBuffer : public detail::TypedBufferObject<GL_UNIFORM_BUFFER> {
  using ValueType = T;

  void set(const T& value) {
    glBufferData(Target, sizeof(value), &value, GL_DYNAMIC_DRAW);
  }

  static UniformBuffer<T> create() { return {create_buffer()}; }
};
}  // namespace glue::gfx