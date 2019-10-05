#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <atomic>
#include <chrono>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl2.h"

#include "complex.h"


int gWidth;
int gHeight;
int gMaxIterate;
std::vector<float> gCenter(2);
std::vector<float> gScale(2);

std::atomic<bool> gCancelRender;
std::atomic<bool> gRefreshRender;
std::vector<float> gPixelBuffer;

double xpos_prev;
double ypos_prev;
constexpr double drag_sensitibity = 0.001;
constexpr double scroll_sensitivity = 0.01;


void initRender() {
  gWidth = 512;
  gHeight = 512;
  gMaxIterate = 100;
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
  std::cout << "Rendering threads: " << num_threads << std::endl;

  std::vector<std::thread> workers;
  for(int thread_id = 0; thread_id < num_threads; thread_id++) {
    workers.emplace_back([&, thread_id, num_threads] {
      const int height_start = double(thread_id)/num_threads * gHeight;
      const int height_end = double(thread_id + 1)/num_threads * gHeight;

      for(int j = height_start; j < height_end; j++) {
        for(int i = 0; i < gWidth; i++) {
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
    });
  }

  for(auto& w : workers) {
    w.join();
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
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count() << " ms" << std::endl;
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
  const double xdif = xpos - xpos_prev;
  const double ydif = ypos - ypos_prev;
  xpos_prev = xpos;
  ypos_prev = ypos;

  if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    gCenter[0] += drag_sensitibity*xdif;
    gCenter[1] += -drag_sensitibity*ydif;
    requestRender();
  }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  gScale[0] += scroll_sensitivity*yoffset;
  gScale[1] += scroll_sensitivity*yoffset;
  requestRender();
}


int main() {
  //Initialize GLFW
  if (!glfwInit()) {
    return -1;
  }
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

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
