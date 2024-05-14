add_library(imgui STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_opengl3.cpp
)
target_compile_features(imgui PUBLIC cxx_std_20)
target_include_directories(imgui PUBLIC . backends)
target_include_directories(imgui PRIVATE ../SDL2-2.28.5/include)
