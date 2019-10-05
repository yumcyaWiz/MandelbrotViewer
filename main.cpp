#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <chrono>
#include <future>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl2.h"

#include "complex.h"
#include "ext/ThreadPool.h"


int gWidth;
int gHeight;
int gMaxIterate;
std::vector<float> gCenter(2);
std::vector<float> gScale(2);

std::atomic<bool> gCancelRender;
std::atomic<bool> gRefreshRender;
std::vector<int> gNum_tiles(2);
std::vector<float> gPixelBuffer;

double g_xpos_prev;
double g_ypos_prev;
constexpr double g_drag_sensitibity = 0.001;
constexpr double g_scroll_sensitivity = 0.01;


void initRender() {
  gWidth = 512;
  gHeight = 512;
  gMaxIterate = 100;

  gNum_tiles[0] = 4;
  gNum_tiles[1] = 4;
  gPixelBuffer.resize(3 * gWidth * gHeight);

  gCenter[0] = 0;
  gCenter[1] = 0;

  gScale[0] = 1;
  gScale[1] = 1;
}


void requestRender() {
  if (gRefreshRender) {
    gCancelRender = true;
  }
  gRefreshRender = true;
}


bool renderMandelbrot(std::vector<float>& pixelBuffer) {
  const int num_threads = std::max(1U, std::thread::hardware_concurrency());
  //std::cout << "Rendering threads: " << num_threads << std::endl;

  ThreadPool pool(num_threads);
  std::vector<std::future<void>> results;

  for(int tile_x = 0; tile_x < gNum_tiles[0]; tile_x++) {
    for(int tile_y = 0; tile_y < gNum_tiles[1]; tile_y++) {
      results.push_back(pool.enqueue([&, tile_x, tile_y] {
        const int width_s = tile_x/double(gNum_tiles[0]) * gWidth;
        const int width_e = (tile_x + 1)/double(gNum_tiles[0]) * gWidth;
        const int height_s = tile_y/double(gNum_tiles[1]) * gHeight;
        const int height_e = (tile_y + 1)/double(gNum_tiles[1]) * gHeight;

        for(int j = height_s; j < height_e; j++) {
          for(int i = width_s; i < width_e; i++) {
            if(gCancelRender) return;

            Complex_d c(gCenter[0] + gScale[0]*(2.0*i - gWidth)/gWidth, gCenter[1] + gScale[1]*(2.0*j - gHeight)/gHeight);
            Complex_d z(0, 0);

            int break_iter = 0;
            for(int k = 0; k < gMaxIterate; k++) {
              z = z*z + c;
              if (length(z) > 2.0) {
                break_iter = k;
                break;
              }
            }

            pixelBuffer[0 + 3*i + 3*gWidth*j] = double(break_iter)/gMaxIterate;
            pixelBuffer[1 + 3*i + 3*gWidth*j] = double(break_iter)/gMaxIterate;
            pixelBuffer[2 + 3*i + 3*gWidth*j] = double(break_iter)/gMaxIterate;
          }
        }
      }));
    }
  }

  for(auto&& result : results) {
    result.get();
  }

  return true;
}


void renderThread() {
  while(1) {
    if (gRefreshRender) {
      const auto start_time = std::chrono::system_clock::now();
      renderMandelbrot(gPixelBuffer);

      if(gCancelRender) {
        gCancelRender = false;
      }
      else {
        std::cout << "Rendering Finished in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count() << " ms" << std::endl;
        gRefreshRender = false;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);

  gWidth = width;
  gHeight = height;
  gPixelBuffer.resize(3 * gWidth * gHeight);
  requestRender();
}


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  const double xdif = xpos - g_xpos_prev;
  const double ydif = ypos - g_ypos_prev;
  g_xpos_prev = xpos;
  g_ypos_prev = ypos;

  if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    gCenter[0] += g_drag_sensitibity*xdif;
    gCenter[1] += -g_drag_sensitibity*ydif;
    requestRender();
  }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  gScale[0] += g_scroll_sensitivity*yoffset;
  gScale[1] += g_scroll_sensitivity*yoffset;
  requestRender();
}


int main() {
  //Initialize GLFW
  if (!glfwInit()) {
    return -1;
  }

  GLFWwindow* window = glfwCreateWindow(512, 512, "Dear ImGui", NULL, NULL);
  if (window == NULL) {
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);

  //Initialize ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL2_Init();

  initRender();
  std::thread render_thread(renderThread);

  //Rendering Loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("UI");
    bool refresh = false;
    {
      refresh |= ImGui::InputInt2("Render Tiles", gNum_tiles.data());
      refresh |= ImGui::InputFloat2("Center", gCenter.data(), 6);
      refresh |= ImGui::InputFloat2("Scale", gScale.data(), 6);
      refresh |= ImGui::InputInt("Iteration", &gMaxIterate);
    }
    ImGui::End();

    if (refresh) {
      requestRender();
    }

    glViewport(0, 0, gWidth, gHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    glRasterPos2i(-1, -1);
    glDrawPixels(gWidth, gHeight, GL_RGB, GL_FLOAT, static_cast<const GLvoid*>(gPixelBuffer.data()));

    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  render_thread.join();

  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
