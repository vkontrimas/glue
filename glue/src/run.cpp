#include "run.hpp"

#include <SDL.h>

#include <array>
#include <glue/debug/timer.hpp>
#include <glue/director/game_director.hpp>
#include <glue/physics.hpp>
#include <glue/physics/jolt_physics_engine.hpp>
#include <glue/simulator/predictor_reconciler_simulator.hpp>

#include "cube_renderer.hpp"
#include "debug/data_logger.hpp"
#include "imgui/imgui_context.hpp"
#include "plane_renderer.hpp"
#include "window.hpp"

namespace glue {
namespace {
// Next component-ification target
class Renderer {
 public:
  Renderer()
      : view_projection_uniforms_{0, {}},
        lighting_uniforms_{1, {}},
        cube_renderer_{view_projection_uniforms_, lighting_uniforms_},
        plane_renderer_{view_projection_uniforms_, lighting_uniforms_} {}

  void draw(const Plane& plane, std::span<CubeRenderer::Instance> cubes,
            const OrbitCamera& camera) {
    {
      view_projection_uniforms_.bind_for_write();
      view_projection_uniforms_.set({camera});
    }

    view_projection_uniforms_.bind_for_shader();
    lighting_uniforms_.bind_for_shader();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    cube_renderer_.draw(cubes);
    plane_renderer_.draw(plane);
  }

 private:
  gfx::UniformBlock<uniforms::ViewProjection> view_projection_uniforms_;
  gfx::UniformBlock<uniforms::Lighting> lighting_uniforms_;
  CubeRenderer cube_renderer_;
  PlaneRenderer plane_renderer_;
};

class MoveInput {
 public:
  void process_event(SDL_Event& event) {
    if (event.type != SDL_KEYUP && event.type != SDL_KEYDOWN) {
      return;
    }

    if (event.key.keysym.sym == SDLK_w) {
      input_forward_ = event.type == SDL_KEYDOWN;
    }
    if (event.key.keysym.sym == SDLK_s) {
      input_back_ = event.type == SDL_KEYDOWN;
    }
    if (event.key.keysym.sym == SDLK_a) {
      input_left_ = event.type == SDL_KEYDOWN;
    }
    if (event.key.keysym.sym == SDLK_d) {
      input_right_ = event.type == SDL_KEYDOWN;
    }
  }

  bool has_input() const { return input_direction() != vec3{0.0f}; }

  vec3 input_direction() const {
    vec3 direction{0.0f};
    if (input_forward_) {
      direction += vec3{0.0f, 0.0f, -1.0f};
    }
    if (input_back_) {
      direction += vec3{0.0f, 0.0f, 1.0f};
    }
    if (input_left_) {
      direction += vec3{-1.0f, 0.0f, 0.0f};
    }
    if (input_right_) {
      direction += vec3{1.0f, 0.0f, 0.0f};
    }

    return direction;
  }

  vec3 movement_direction(f32 camera_yaw) const {
    const float x = glm::cos(camera_yaw);
    const float z = glm::sin(camera_yaw);
    const vec3 forward{x, 0, z};
    const vec3 right = glm::cross(vec3{0, 1, 0}, forward);
    const vec3 input = input_direction();
    return glm::normalize(right * input.x + forward * input.z);
  }

 private:
  bool input_forward_ = false;
  bool input_back_ = false;
  bool input_left_ = false;
  bool input_right_ = false;
};
}  // namespace

void run(const RunOptions& options) {
  bool window_shown = false;
  auto window = create_window("Glue", options.window_position_x,
                              options.window_position_y, options.window_width,
                              options.window_height);
  auto gl_context = init_gl(window.get());
  glue::imgui::ImGuiContext imgui{window.get(), gl_context.get()};

  auto director = std::make_shared<director::GameDirector>();
  auto physics = std::make_shared<physics::JoltPhysicsEngine>();

  const auto ground_id = ObjectID::random();
  const Plane ground_plane{{}, 3000.0f};
  physics->add_static_plane(ground_id, 0, ground_plane);

  const ObjectID kPlayerID{"player"};
  constexpr f32 kPlayerCubeRadius = 0.5f;
  constexpr Pose kPlayerStartPose{vec3{0.0f, 3.0f, 0.0f},
                                  glm::identity<quat>()};
  constexpr f32 kCubeRadius = 0.2f;
  auto initial_frame =
      WorldFrame::init(OrbitCamera{}, kPlayerID, kPlayerStartPose,
                       kPlayerCubeRadius, 30, kCubeRadius, *physics);

  auto simulator = std::make_shared<simulator::PredictorReconcilerSimulator>(
      director, physics, *initial_frame, 1.0 / 60.0, 0.250);

  MoveInput player_move_input;

  // constexpr int kPlayerMaxJumps = 5;
  // int player_jump_count = 0;
  // float player_activity = 1.0f;
  // physics.on_collision_enter(kPlayerID, [&](ObjectID other) {
  //   if (other == ground_id) {
  //     player_jump_count = 0;
  //   }
  // });
  // physics.on_become_active(kPlayerID, [&]() { player_activity = 1.0f; });
  // physics.on_become_inactive(kPlayerID, [&]() { player_activity = 0.0f; });

  // std::vector<ObjectID> cube_ids;
  // std::vector<float> cube_activities;
  // constexpr float kCubeRadius = 0.2f;
  // place_cubes(30, kCubeRadius, physics, cube_ids, cube_activities);

  Renderer renderer;

  std::vector<CubeRenderer::Instance> cube_instances{
      initial_frame->cubes.size()};
  std::vector<u16> active_cube_instances;
  active_cube_instances.reserve(initial_frame->cubes.size());

  for (std::size_t i = 0; i < initial_frame->cubes.size(); ++i) {
    const auto& cube = initial_frame->cubes[i];
    auto& instance = cube_instances[i];
    instance.position = cube.position;
    instance.rotation = cube.rotation;
    instance.size = kCubeRadius;
    instance.activity = 0.0f;
  }
  cube_instances[0].size = kPlayerCubeRadius;

  const auto fill_instances = [&](const WorldFrame& frame) {
    for (auto index : frame.active_cubes) {
      const auto& pose = frame.cubes[index];
      auto& instance = cube_instances[index];
      instance.position = pose.position;
      instance.rotation = pose.rotation;
      instance.activity = 1.0f;
    }
  };

  const auto clear_activity = [&](auto vec) {
    for (auto index : vec) {
      cube_instances[index].activity = 0.0f;
    }
  };

  u64 game_start_time = SDL_GetPerformanceCounter();
  u64 last_frame = 0;
  u64 previous_time = 0;
  u64 now = SDL_GetPerformanceCounter();

  bool show_imgui_demo = false;
  bool show_implot_demo = false;

  constexpr double kDataLogWindow = 2.0;
  debug::DataLogger<double> frame_times{kDataLogWindow};
  debug::DataLogger<double> render_times{kDataLogWindow};
  debug::DataLogger<double> physics_times{kDataLogWindow};

  std::vector<double> ys_buffer, xs_buffer;
  const auto fill_buffers = [](const debug::DataLogger<double>& data,
                               std::vector<double>& ys,
                               std::vector<double>& xs) {
    const auto count = data.count();
    if (ys.size() < count) {
      ys.resize(count);
    }
    if (xs.size() < count) {
      xs.resize(count);
    }

    for (std::size_t i = 0; i < count; ++i) {
      const auto& entry = data[i];
      ys[i] = entry.data;
      xs[i] = entry.time;
    }
  };

  constexpr f64 kYLimit = 1000.0 / 30.0;
  double frame_logger_y_axis = kYLimit;
  const std::array<f64, 6> horizontal_lines{{
      1000.0,
      1000.0 / 10,
      1000.0 / 30,
      1000.0 / 60,
      1000.0 / 120,
      1000.0 / 240,
  }};
  const std::array<const char*, 6> line_labels{{
      "1",
      "10",
      "30",
      "60",
      "120",
      "240",
  }};
  static_assert(horizontal_lines.size() == line_labels.size());

  bool vsync = true;

  std::array<std::pair<double, const char*>, 5> timestep_limits{{
      {1.0 / 60.0, "60"},
      {1.0 / 90.0, "90"},
      {1.0 / 120.0, "120"},
      {1.0 / 240.0, "240"},
      {1.0 / 1000.0, "Unlimited"},
  }};
  int selected_timestep = 3;

  SDL_GL_SetSwapInterval(1);

  bool is_running = true;
  while (is_running) {
    const debug::Timer frame_timer;

    previous_time = now;
    now = SDL_GetPerformanceCounter();
    f64 frame_delta_time = static_cast<f64>(now - previous_time) /
                           static_cast<f64>(SDL_GetPerformanceFrequency());
    Input input{};

    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
      imgui.process_event(event);
      player_move_input.process_event(event);

      if (event.type == SDL_QUIT) {
        is_running = false;
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE) {
        is_running = false;
      } else if (event.type == SDL_KEYUP &&
                 event.key.keysym.sym == SDLK_SPACE) {
        input.jump = true;
      }
    }

    imgui.start_new_frame();

    ImGui::SetNextWindowSize({450, 700}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Always);
    if (ImGui::Begin("FPS", nullptr,
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoNavInputs)) {
      if (ImGui::TreeNode("GameData", "%03.3lf ms (%3.0lf FPS)",
                          frame_delta_time * 1000.0, 1.0 / frame_delta_time)) {
        if (ImPlot::BeginPlot("frame_stats", {-1, 140},
                              ImPlotFlags_NoFrame | ImPlotFlags_NoBoxSelect |
                                  ImPlotFlags_NoTitle | ImPlotFlags_NoMenus |
                                  ImPlotFlags_NoMouseText)) {
          ImPlot::SetupAxes("s", "ms",
                            ImPlotAxisFlags_NoLabel |
                                ImPlotAxisFlags_NoTickLabels |
                                ImPlotAxisFlags_AutoFit,
                            ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_LockMin);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%g ms");

          ImPlot::SetupAxisLinks(ImAxis_Y1, nullptr, &frame_logger_y_axis);

          ImPlot::SetupAxesLimits(0.0, kDataLogWindow, 0.0, kYLimit);
          ImPlot::SetupLegend(
              ImPlotLocation_South,
              ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);

          ImPlot::SetupAxis(ImAxis_Y2, nullptr,
                            ImPlotAxisFlags_AuxDefault |

                                ImPlotAxisFlags_NoMenus);
          ImPlot::SetupAxisTicks(ImAxis_Y2, horizontal_lines.data(),
                                 horizontal_lines.size(), line_labels.data(),
                                 false);
          ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, frame_logger_y_axis,
                                  ImPlotCond_Always);

          ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
          fill_buffers(frame_times, ys_buffer, xs_buffer);
          ImPlot::PlotLine("total", xs_buffer.data(), ys_buffer.data(),
                           frame_times.count());
          ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.4f);
          ImPlot::PlotShaded("total", xs_buffer.data(), ys_buffer.data(),
                             frame_times.count());
          ImPlot::PopStyleVar();

          fill_buffers(physics_times, ys_buffer, xs_buffer);
          ImPlot::PlotLine("physics", xs_buffer.data(), ys_buffer.data(),
                           physics_times.count());

          fill_buffers(render_times, ys_buffer, xs_buffer);
          ImPlot::PlotLine("render", xs_buffer.data(), ys_buffer.data(),
                           render_times.count());

          ImPlot::EndPlot();
        }

        ImGui::TreePop();
        ImGui::Spacing();
      }

      if (ImGui::TreeNode("settings")) {
        if (ImGui::Checkbox("V-sync", &vsync)) {
          if (vsync) {
            SDL_GL_SetSwapInterval(1);
          } else {
            SDL_GL_SetSwapInterval(0);
          }
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(vsync);
        ImGui::SliderInt(
            "FPS", &selected_timestep, 0, timestep_limits.size() - 1,
            timestep_limits[selected_timestep].second,
            ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::Checkbox("ImGui demo window", &show_imgui_demo);
        ImGui::Checkbox("ImPlot demo window", &show_implot_demo);

        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::End();
    }

    if (show_imgui_demo) {
      ImGui::ShowDemoWindow(&show_imgui_demo);
    }

    if (show_implot_demo) {
      ImPlot::ShowDemoWindow(&show_implot_demo);
    }

    input.direction = player_move_input.movement_direction(
        initial_frame->camera.position_rel.yaw);
    simulator->update_timed(frame_delta_time, input, physics_times);

    {
      const debug::Timer render_timer;

      clear_activity(active_cube_instances);
      simulator->current_world_frame(*initial_frame);
      fill_instances(*initial_frame);
      active_cube_instances.clear();
      std::copy(std::begin(initial_frame->active_cubes),
                std::end(initial_frame->active_cubes),
                std::back_inserter(active_cube_instances));

      int render_width, render_height;
      SDL_GetWindowSizeInPixels(window.get(), &render_width, &render_height);
      initial_frame->camera.params.aspect =
          static_cast<f32>(render_width) / static_cast<f32>(render_height);

      glViewport(0, 0, render_width, render_height);
      renderer.draw(ground_plane, cube_instances, initial_frame->camera);

      imgui.draw();

      render_times.log(render_timer.elapsed_ms<f64>());
    }

    const bool unlimited_frames =
        selected_timestep == timestep_limits.size() - 1;
    const f64 target_timestep_ms =
        timestep_limits[selected_timestep].first * 1000.0;
    const int sleep_step_ms = 1;
    const int spinlock_time_ms = 4;
    const f64 margin_ms = 0.4;

    while (!vsync && !unlimited_frames &&
           frame_timer.elapsed_ms<f64>() < target_timestep_ms - margin_ms) {
      if (target_timestep_ms - frame_timer.elapsed_ms<f64>() >
          spinlock_time_ms) {
        SDL_Delay(sleep_step_ms);
      }
    }

    SDL_GL_SwapWindow(window.get());
    if (!window_shown) {
      window_shown = true;
      SDL_ShowWindow(window.get());
    }

    frame_times.log(frame_timer.elapsed_ms<f64>());
  }
}
}  // namespace glue