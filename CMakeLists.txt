cmake_minimum_required(VERSION 3.12)

project(Mandelbrot_Viewer2 LANGUAGES C CXX)

add_executable(main main.cpp)


#OpenGL
find_package(OpenGL REQUIRED)
target_include_directories(main PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(main ${OPENGL_LIBRARY})


#GLFW
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(main glfw)


#Dear ImGui
add_library(imgui STATIC ext/imgui/imgui.cpp ext/imgui/imgui_widgets.cpp ext/imgui/imgui_draw.cpp ext/imgui/imgui_demo.cpp ext/imgui/examples/imgui_impl_glfw.cpp ext/imgui/examples/imgui_impl_opengl2.cpp)
target_include_directories(imgui PUBLIC ext/imgui)
target_include_directories(imgui PUBLIC ${OPENGL_INCLUDE_DIR})

target_include_directories(main PUBLIC imgui)
target_link_libraries(main imgui)
