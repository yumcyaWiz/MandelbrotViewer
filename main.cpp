#include <iostream>
#include <vector>
#include <cmath>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl2.h"

#include "complex.h"


int gWidth = 512;
int gHeight = 512;
int gMaxIterate = 100;
std::vector<float> gPixelBuffer(3 * gWidth * gHeight);


void renderMandelbrot(std::vector<float>& pixelBuffer) {
  for(int j = 0; j < gHeight; j++) {
    for(int i = 0; i < gWidth; i++) {
      Complex_d c(double(i)/gWidth, double(j)/gHeight);
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

  renderMandelbrot(gPixelBuffer);

  //Rendering Loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Render();
    glViewport(0, 0, gWidth, gHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glRasterPos2i(-1, -1);
    glDrawPixels(gWidth, gHeight, GL_RGB, GL_FLOAT, static_cast<const GLvoid*>(gPixelBuffer.data()));

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
