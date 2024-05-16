#pragma once

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

namespace glue::imgui {
class ImGuiContext {
 public:
  ImGuiContext(SDL_Window* window, void* gl_context) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();
  }

  ImGuiContext(const ImGuiContext&) = delete;
  ImGuiContext& operator=(const ImGuiContext&) = delete;

  ImGuiContext(ImGuiContext&&) = delete;
  ImGuiContext& operator=(ImGuiContext&&) = delete;

  void process_event(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
  }

  void start_new_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
  }

  void draw() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  ~ImGuiContext() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

 private:
};
}  // namespace glue::imgui