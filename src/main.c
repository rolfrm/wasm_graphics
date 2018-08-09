
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define ASSERT(x) if(x){}
#include "main.h"

int main(){
  glfwInit();
  GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  
  GLFWwindow * win = glfwCreateWindow(mode->width, mode->height, "", monitor, NULL);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);  
  ASSERT(glewInit() == GLEW_OK);
  context ctx = {.win = win};

  while(glfwWindowShouldClose(ctx.win) == false){
    mainloop(&ctx);
  }
    
  glfwTerminate();
   
  return 0;

}
