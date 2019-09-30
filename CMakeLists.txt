cmake_minimum_required(VERSION 3.12)

project(Mandelbrot_Viewer2 LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)

add_executable(main main.cpp)


#glad
add_library(glad STATIC ext/glad/src/glad.c)
target_include_directories(glad PUBLIC ext/glad/include)

target_include_directories(main PUBLIC glad)
target_link_libraries(main glad)
target_link_libraries(main dl)


#GLFW
pkg_search_module(GLFW REQUIRED glfw3)
target_include_directories(main PUBLIC ${GLFW_INCLUDE_DIRS})
target_link_libraries(main ${GLFW_STATIC_LIBRARIES})


#Dear ImGui
add_library(imgui STATIC ext/imgui/imgui.cpp ext/imgui/imgui_widgets.cpp ext/imgui/imgui_draw.cpp ext/imgui/imgui_demo.cpp ext/imgui/examples/imgui_impl_glfw.cpp ext/imgui/examples/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ext/imgui)
target_include_directories(imgui PUBLIC ext/glad/include)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_include_directories(main PUBLIC imgui)
target_link_libraries(main imgui)
