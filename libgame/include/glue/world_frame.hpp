#pragma once

#include <glue/camera.hpp>
#include <glue/collections/fixed_vec.hpp>
#include <glue/physics/iphysics_engine.hpp>
#include <glue/types.hpp>
#include <iterator>
#include <memory>
#include <unordered_set>

namespace glue {
struct WorldFrame {
  static constexpr std::size_t kMaxCubes = 65536;

  u32 index = 0;
  OrbitCamera camera;
  FixedVec<Pose, kMaxCubes> cubes;
  FixedVec<u16, kMaxCubes> active_cubes;

  static auto init(OrbitCamera camera, ObjectID player_id, Pose player_pose,
                   f32 player_radius, std::size_t cube_array_width,
                   f32 cube_width, physics::IPhysicsEngine& physics) {
    auto frame = std::make_unique<WorldFrame>();
    frame->index = 0;
    frame->camera = camera;
    frame->camera.target = player_pose.position;

    frame->cubes.emplace_back(player_pose);
    physics.add_dynamic_cube(player_id, 0,
                             {player_pose.position, glm::identity<quat>()},
                             player_radius, true);

    const f32 spacing = 6.0f * cube_width;
    const vec3 start =
        -1 * spacing * 0.5f * vec3{cube_array_width, 0, cube_array_width} +
        vec3{0, cube_width, 0};

    std::size_t cube_index = 1;
    for (int i = 0; i < cube_array_width; ++i) {
      for (int j = 0; j < cube_array_width; ++j) {
        vec3 position = start + spacing * vec3{i, 0, j};

        frame->cubes.emplace_back(Pose{position, glm::identity<quat>()});

        const auto id = ObjectID::random();
        physics.add_dynamic_cube(id, cube_index,
                                 {position, glm::identity<quat>()}, cube_width,
                                 false);
        cube_index++;
      }
    }

    return frame;
  }

  static void interpolate(const WorldFrame& past, const WorldFrame& future,
                          f32 alpha, WorldFrame& out) {
    static std::unordered_set<u16> index_set;

    out.index = past.index;

    out.camera.target =
        glm::mix(past.camera.target, future.camera.target, alpha);

    index_set.clear();
    for (auto index : past.active_cubes) {
      index_set.emplace(index);
    }
    for (auto index : future.active_cubes) {
      index_set.emplace(index);
    }

    out.active_cubes.clear();
    std::copy(std::begin(index_set), std::end(index_set),
              std::back_inserter(out.active_cubes));

    for (auto index : out.active_cubes) {
      out.cubes[index].position = glm::mix(past.cubes[index].position,
                                           future.cubes[index].position, alpha);
      out.cubes[index].rotation = glm::slerp(
          past.cubes[index].rotation, future.cubes[index].rotation, alpha);
    }
  }
};
}  // namespace glue