#include <SDL.h>
#include <glad/gl.h>
#include <glog/logging.h>

#include <glue/world.hpp>
#include <memory>

using namespace glue;

namespace {
struct SDLWindowDeleter {
  void operator()(SDL_Window* window) { SDL_DestroyWindow(window); }
};

auto create_window(const char* title, int width, int height) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // max opengl version on mac os
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  std::unique_ptr<SDL_Window, SDLWindowDeleter> window{SDL_CreateWindow(
      title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
      SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL)};
  CHECK(window) << SDL_GetError();

  return window;
}

struct SDLGLContextDeleter {
  void operator()(void* gl_context) { SDL_GL_DeleteContext(gl_context); }
};

auto init_gl(SDL_Window* window) {
  std::unique_ptr<void, SDLGLContextDeleter> gl_context{
      SDL_GL_CreateContext(window)};
  CHECK(gl_context) << SDL_GetError();

  const auto gl_version =
      gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress));
  CHECK(gl_version != 0) << "Failed to load GL";
  LOG(INFO) << "GL version " << GLAD_VERSION_MAJOR(gl_version) << "."
            << GLAD_VERSION_MINOR(gl_version);

  return gl_context;
}
}  // namespace

void run() {
  auto window = create_window("Glue", 1280, 720);
  auto gl_context = init_gl(window.get());
  World world{};

  bool is_running = true;
  while (is_running) {
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        is_running = false;
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE) {
        is_running = false;
      }
    }

    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_GL_SwapWindow(window.get());
  }
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;

  const auto sdl_init_error = SDL_Init(SDL_INIT_VIDEO);
  CHECK(!sdl_init_error) << SDL_GetError();

  run();

  SDL_Quit();
  return 0;
}