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


int gWidth = 512;
int gHeight = 512;
int gMaxIterate = 100;
std::vector<float> gCenter(2);
std::vector<float> gScale(2);

std::atomic<bool> gRefreshRender;
std::vector<float> gPixelBuffer(3 * gWidth * gHeight);


void initRender() {
  gCenter[0] = 0;
  gCenter[1] = 0;

  gScale[0] = 1;
  gScale[1] = 1;
}


void requestRender() {
  gRefreshRender = true;
}


void renderMandelbrot(std::vector<float>& pixelBuffer) {
  for(int j = 0; j < gHeight; j++) {
    for(int i = 0; i < gWidth; i++) {
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

      pixelBuffer[0 + 3*i + gWidth*j] = double(break_iter)/gMaxIterate;
      pixelBuffer[1 + 3*i + gWidth*j] = double(break_iter)/gMaxIterate;
      pixelBuffer[2 + 3*i + gWidth*j] = double(break_iter)/gMaxIterate;
    }
  }
}


void renderThread() {
  while(1) {
    if (gRefreshRender) {
      renderMandelbrot(gPixelBuffer);
      gRefreshRender = false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
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
    }
    ImGui::End();

    if (refresh) {
      requestRender();
    }

    ImGui::Render();
    glViewport(0, 0, gWidth, gHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glRasterPos2i(-1, -1);
    glDrawPixels(gWidth, gHeight, GL_RGB, GL_FLOAT, static_cast<const GLvoid*>(gPixelBuffer.data()));

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
