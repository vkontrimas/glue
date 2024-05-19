add_library(implot OBJECT implot.cpp implot_demo.cpp implot_items.cpp)
target_compile_features(implot PUBLIC cxx_std_17)
target_link_libraries(implot PUBLIC imgui)
target_include_directories(implot PUBLIC .)
