cmake_minimum_required(VERSION 3.11)
project(glue)

list(INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

# Options
option(GLUE_BUILD_TESTS "Build tests" ON)

# Third party libs
if (GLUE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(third_party/googletest-1.14.0)
endif()

add_subdirectory(third_party/glm-0.9.9.8)
add_subdirectory(third_party/SDL2-2.28.5)
add_subdirectory(third_party/glad)
add_subdirectory(third_party/imgui-v1.90-docking)
add_subdirectory(third_party/glog-0.7.0)
add_subdirectory(third_party/stb_image-2.29)
add_subdirectory(third_party/JoltPhysics-5.0.0/Build)

# Common typedefs, header-only utilities
add_library(common INTERFACE)
target_link_libraries(common INTERFACE glm glog)
target_include_directories(common INTERFACE libcommon/include)
target_compile_features(common INTERFACE cxx_std_20)
target_compile_options(common INTERFACE -Wno-deprecated-volatile)

if (GLUE_BUILD_TESTS) 
    add_executable(tests_common libcommon/tests/test_object_id.cpp)
    target_link_libraries(tests_common PRIVATE GTest::gtest_main)
endif()

# Gameplay logic
# (for client / server)
add_library(
    game OBJECT
    libgame/src/physics.cpp
)
target_include_directories(game PUBLIC libgame/include)
target_link_libraries(game PUBLIC common Jolt)
target_compile_features(game PUBLIC cxx_std_20)

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
target_link_libraries(glue common game SDL2 glad imgui stb_image)
target_compile_features(glue PUBLIC cxx_std_20)

if (APPLE)
    target_link_libraries(glue "-framework IOKit")
    target_link_libraries(glue "-framework Cocoa")
    target_link_libraries(glue "-framework OpenGL")
endif()

if (GLUE_BUILD_TESTS) 
    include(GoogleTest)
    gtest_discover_tests(tests_common)
endif()

# Tests