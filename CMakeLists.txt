cmake_minimum_required(VERSION 3.11)
project(glue)

# Third party libs
add_subdirectory(third_party/glm-0.9.9.8)
add_subdirectory(third_party/SDL2-2.28.5)
add_subdirectory(third_party/glad)
add_subdirectory(third_party/imgui-v1.90-docking)
add_subdirectory(third_party/glog-0.7.0)

# Common typedefs, header-only utilities
add_library(common STATIC libcommon/src/placeholder.cpp)
target_link_libraries(common PUBLIC glm glog)
target_include_directories(common PUBLIC libcommon/include)
target_compile_features(common PUBLIC cxx_std_20)

# Gameplay logic
# (for client / server)
add_library(
    game STATIC
    libgame/src/placeholder.cpp
)
target_include_directories(game PUBLIC libgame/include)
target_link_libraries(game PUBLIC common)
target_compile_features(game PUBLIC cxx_std_20)

# Client
add_executable(
    glue
    glue/src/main.cpp
)
target_include_directories(glue PRIVATE glue/include)
target_link_libraries(glue common game SDL2 glad imgui)
target_compile_features(glue PUBLIC cxx_std_20)

if (APPLE)
    target_link_libraries(glue "-framework IOKit")
    target_link_libraries(glue "-framework Cocoa")
    target_link_libraries(glue "-framework OpenGL")
endif()
