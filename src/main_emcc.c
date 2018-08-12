#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iron/types.h>
#include <iron/linmath.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "squares.h"
#include "particles.h"
#include "main.h"

#define ASSERT(x) if(x){}

void on_req_fullscreen(){
  emscripten_request_fullscreen(0, 1);
  
}


int main(){
  glfwInit();

  const int simulate_infinite_loop = 1; // call the function repeatedly
  const int fps = -1; // call the function as fast as lhe browser wants to render (typically 60fps)

  GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  int w, h;
  glfwGetMonitorPhysicalSize(monitor, &w, &h);
  printf("%i %i\n", w, h);
  //GLFWwindow * win = glfwCreateWindow(512, 512, "", monitor, NULL);
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  printf("%i %i %i\n", mode, mode->width, mode->height);
  //GLFWwindow* win = glfwCreateWindow(mode->width, mode->height, "My Title", monitor, NULL);  

  GLFWwindow * win = glfwCreateWindow(512, 512, "", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(2);  
  ASSERT(glewInit() == GLEW_OK);
  context ctx = {.win = win};
  emscripten_set_main_loop_arg((void *) &mainloop, &ctx, fps, simulate_infinite_loop);
 
    
  glfwTerminate();
   
  return EXIT_SUCCESS;

}
