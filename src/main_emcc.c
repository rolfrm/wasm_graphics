#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include "main.h"

#define ASSERT(x) if(x){}

int main(){
  glfwInit();

  const int simulate_infinite_loop = 1; // call the function repeatedly
  const int fps = -1; // call the function as fast as lhe browser wants to render (typically 60fps)
  GLFWwindow * win = glfwCreateWindow(512, 512, "", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(2);  
  ASSERT(glewInit() == GLEW_OK);
  context ctx = {.win = win};
  emscripten_set_main_loop_arg((void *) &mainloop, &ctx, fps, simulate_infinite_loop);
 
    
  glfwTerminate();
   
  return EXIT_SUCCESS;

}
