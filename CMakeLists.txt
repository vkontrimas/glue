cmake_minimum_required(VERSION 3.11)
project(glue)

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

# Options
option(GLUE_BUILD_TESTS "Build tests" ON)

# Third party libs
add_subdirectory(third_party/zlib-1.3.1)
add_subdirectory(third_party/glm-0.9.9.8)
add_subdirectory(third_party/SDL2-2.28.5)
add_subdirectory(third_party/glad)
add_subdirectory(third_party/imgui-v1.90-docking)
add_subdirectory(third_party/implot-0.16)
add_subdirectory(third_party/glog-0.7.0)
add_subdirectory(third_party/stb_image-2.29)
add_subdirectory(third_party/JoltPhysics-5.0.0/Build)
add_subdirectory(third_party/assimp-5.4.1)
add_subdirectory(third_party/CLI11-2.4.2)

if (GLUE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(third_party/googletest-1.14.0)
endif()

# Common typedefs, header-only utilities
add_library(common STATIC libcommon/src/objects/id.cpp)
target_link_libraries(common PUBLIC glm glog)
target_link_libraries(common PRIVATE zlib)
target_include_directories(common PUBLIC libcommon/include)
target_compile_features(common PUBLIC cxx_std_20)
target_compile_options(common PUBLIC -Wno-deprecated-volatile)

if (GLUE_BUILD_TESTS) 
    add_executable(
        tests_common
        libcommon/tests/objects/test_object_id.cpp
        libcommon/tests/collections/test_fixed_vec.cpp
        libcommon/tests/collections/test_fixed_circular_buffer.cpp
        libcommon/tests/collections/test_circular_buffer.cpp
        libcommon/tests/test_presence_window.cpp
    )
    target_link_libraries(tests_common PRIVATE common GTest::gtest_main GTest::gmock_main)
endif()

# Networking library
add_library(
    network
    STATIC
    libnetwork/src/network/socket.cpp 
) 
target_link_libraries(network PUBLIC common)
target_include_directories(network PUBLIC libnetwork/include)
target_compile_features(network PUBLIC cxx_std_20)

if (GLUE_BUILD_TESTS) 
    add_executable(
        tests_network
        libnetwork/tests/network/test_address.cpp
        libnetwork/tests/network/test_socket.cpp
        libnetwork/tests/network/test_packet.cpp
    )
    target_link_libraries(tests_network PRIVATE common network GTest::gtest_main GTest::gmock)
endif()

# Bitpacking library for compressing game data in network transmissions
add_library(
    bitpack
    INTERFACE
) 
target_link_libraries(bitpack INTERFACE common)
target_include_directories(bitpack INTERFACE libbitpack/include)
target_compile_features(bitpack INTERFACE cxx_std_20)

if (GLUE_BUILD_TESTS) 
    add_executable(
        tests_bitpack
        libbitpack/tests/bitpack/test_bitpack.cpp
        libbitpack/tests/bitpack/test_packer.cpp
        libbitpack/tests/bitpack/test_examples.cpp
    )
    target_link_libraries(tests_bitpack PRIVATE common bitpack GTest::gtest_main GTest::gmock)
endif()

# Gameplay logic
# (for client / server)
add_library(
    game STATIC
    libgame/src/physics/layers.cpp
    libgame/src/physics/jolt_physics_engine.cpp
    libgame/src/physics/jolt_physics_backend.cpp
    libgame/src/physics/jolt_setup_globals.cpp
)
target_include_directories(game PUBLIC libgame/include)
target_link_libraries(game PUBLIC common Jolt)
target_compile_features(game PUBLIC cxx_std_20)

if (GLUE_BUILD_TESTS) 
    add_executable(
        tests_game
        libgame/tests/physics/test_jolt_glm_compat.cpp
        libgame/tests/simulator/test_fixed_timestep.cpp
        libgame/tests/simulator/test_predictor_reconciler_simulator.cpp
    )
    target_link_libraries(tests_game PRIVATE game GTest::gtest_main GTest::gmock)
    target_include_directories(tests_game PRIVATE libgame/src)
endif()

# Client
add_executable(
    glue
    glue/src/main.cpp
    glue/src/run.cpp
    glue/src/gfx/shader.cpp
    glue/src/cube_renderer.cpp
    glue/src/plane_renderer.cpp
)
target_include_directories(glue PRIVATE glue/include)
target_link_libraries(glue common network game SDL2 glad imgui implot stb_image assimp CLI11)
target_compile_features(glue PUBLIC cxx_std_20)

if (APPLE)
    target_link_libraries(glue "-framework IOKit")
    target_link_libraries(glue "-framework Cocoa")
    target_link_libraries(glue "-framework OpenGL")
endif()

if (GLUE_BUILD_TESTS) 
    include(GoogleTest)
    gtest_discover_tests(tests_common)
    gtest_discover_tests(tests_game)
    gtest_discover_tests(tests_network)
    gtest_discover_tests(tests_bitpack)
endif()

# Tests