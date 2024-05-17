#include <SDL.h>

#include <array>
#include <glue/physics.hpp>

#include "cube_renderer.hpp"
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

  void draw(const Plane& plane, const Pose& player_cube,
            float player_cube_radius, const std::vector<Pose>& cubes,
            float cube_radius, const OrbitCamera& camera) {
    {
      view_projection_uniforms_.bind_for_write();
      view_projection_uniforms_.set({camera});
    }

    view_projection_uniforms_.bind_for_shader();
    lighting_uniforms_.bind_for_shader();

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.4f, 0.6f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    cube_renderer_.draw(player_cube, player_cube_radius);
    for (const auto& cube : cubes) {
      cube_renderer_.draw(cube, cube_radius);
    }
    plane_renderer_.draw(plane);
  }

 private:
  gfx::UniformBlock<uniforms::ViewProjection> view_projection_uniforms_;
  gfx::UniformBlock<uniforms::Lighting> lighting_uniforms_;
  CubeRenderer cube_renderer_;
  PlaneRenderer plane_renderer_;
};

void place_cubes(int rows, float cube_width, JoltPhysics& physics,
                 std::vector<ObjectID>& cube_ids) {
  cube_ids.reserve(rows * rows);

  float spacing = cube_width * 6.0f;
  vec3 start =
      -1 * spacing * 0.5f * vec3{rows, 0, rows} + vec3{0, cube_width, 0};
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < rows; ++j) {
      vec3 position = start + spacing * vec3{i, 0, j};

      const auto id = ObjectID::random();
      cube_ids.emplace_back(id);
      physics.add_dynamic_cube(id, {position, glm::identity<quat>()},
                               cube_width, false);
    }
  }
}

class MoveInput {
 public:
  explicit MoveInput(f32 torque) : torque_{torque} {}

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

  vec3 torque_axis(f32 camera_yaw) const {
    if (!has_input()) {
      return vec3{};
    }

    return glm::cross(vec3{0.0f, 1.0f, 0.0f}, movement_direction(camera_yaw));
  }

  f32 torque() const { return has_input() ? torque_ : 0.0f; }

 private:
  bool input_forward_ = false;
  bool input_back_ = false;
  bool input_left_ = false;
  bool input_right_ = false;
  f32 torque_;
};
}  // namespace

void run() {
  bool window_shown = false;
  auto window = create_window("Glue", 1280, 720);
  auto gl_context = init_gl(window.get());
  glue::imgui::ImGuiContext imgui{window.get(), gl_context.get()};

  JoltPhysics physics;

  const Plane ground_plane{{}, 3000.0f};
  physics.add_static_plane(ObjectID::random(), ground_plane);

  OrbitCamera camera;

  ObjectID player_id{"player"};
  constexpr float kPlayerCubeRadius = 0.5f;
  {
    constexpr vec3 player_start_position{0.0f, 3.0f, 0.0f};
    physics.add_dynamic_cube(player_id,
                             {player_start_position, glm::identity<quat>()},
                             kPlayerCubeRadius, true);
    camera.target = player_start_position;
  }
  MoveInput player_move_input{4000.0f};

  std::vector<ObjectID> cube_ids;
  constexpr float kCubeRadius = 0.2f;
  place_cubes(30, kCubeRadius, physics, cube_ids);
  std::vector<Pose> cube_poses{cube_ids.size(), Pose{}};

  Renderer renderer;

  u64 last_frame = 0;
  u64 previous_time = 0;
  u64 now = SDL_GetPerformanceCounter();

  bool show_imgui_demo = false;

  bool vsync = true;
  SDL_GL_SetSwapInterval(1);

  bool is_running = true;
  while (is_running) {
    previous_time = now;
    now = SDL_GetPerformanceCounter();

    float frame_delta_time = static_cast<float>(now - previous_time) /
                             static_cast<float>(SDL_GetPerformanceFrequency());

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
        physics.add_impulse(player_id, vec3{0.0f, 5000.0f, 0.0f});
      }
    }

    imgui.start_new_frame();

    ImGui::SetNextWindowSize({300, 100}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Always);
    if (ImGui::Begin("FPS", nullptr,
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoNavInputs)) {
      if (ImGui::TreeNode("Settings", "%03.3f ms (%3.0f FPS)",
                          frame_delta_time * 1000.0f,
                          1.0f / frame_delta_time)) {
        if (ImGui::Checkbox("V-sync", &vsync)) {
          if (vsync) {
            SDL_GL_SetSwapInterval(1);
          } else {
            SDL_GL_SetSwapInterval(0);
          }
        }

        ImGui::Separator();
        ImGui::Checkbox("ImGui demo window", &show_imgui_demo);

        ImGui::TreePop();
        ImGui::Spacing();
      }

      ImGui::End();
    }

    if (show_imgui_demo) {
      ImGui::ShowDemoWindow(&show_imgui_demo);
    }

    physics.update(frame_delta_time, [&]() {
      if (player_move_input.has_input()) {
        physics.add_torque(
            player_id, player_move_input.torque_axis(camera.position_rel.yaw),
            player_move_input.torque());
      }
    });

    const auto player_pose = physics.get_interpolated_pose(player_id);
    camera.target = player_pose.position;

    for (int i = 0; i < cube_poses.size(); ++i) {
      cube_poses[i] = physics.get_interpolated_pose(cube_ids[i]);
    }

    renderer.draw(ground_plane, player_pose, kPlayerCubeRadius, cube_poses,
                  kCubeRadius, camera);

    imgui.draw();

    SDL_GL_SwapWindow(window.get());
    if (!window_shown) {
      window_shown = true;
      SDL_ShowWindow(window.get());
    }
  }
}
}  // namespace glue