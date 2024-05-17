#pragma once

#include <SDL.h>
#include <glad/gl.h>
#include <glog/logging.h>

#include <memory>

namespace glue {
struct SDLWindowDeleter {
  void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
};

inline auto create_window(const char* title, int width, int height) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // max opengl version on mac os
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  std::unique_ptr<SDL_Window, SDLWindowDeleter> window{SDL_CreateWindow(
      title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
      SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN)};
  CHECK(window) << SDL_GetError();

  return window;
}

struct SDLGLContextDeleter {
  void operator()(void* gl_context) { SDL_GL_DeleteContext(gl_context); }
};

inline auto init_gl(SDL_Window* window) {
  std::unique_ptr<void, SDLGLContextDeleter> gl_context{
      SDL_GL_CreateContext(window)};
  CHECK(gl_context) << SDL_GetError();

  const auto gl_version =
      gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress));
  CHECK(gl_version != 0) << "Failed to load GL";
  LOG(INFO) << "GL version " << GLAD_VERSION_MAJOR(gl_version) << "."
            << GLAD_VERSION_MINOR(gl_version);

  CHECK(GLAD_GL_EXT_texture_filter_anisotropic)
      << "anisotropic filtering unsupported :(";

  return gl_context;
}
}  // namespace glue