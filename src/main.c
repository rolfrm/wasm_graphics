
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iron/linmath.h>
#include <iron/types.h>
#include <iron/time.h>
#define ASSERT(x) if(x){}
#include "squares.h"
#include "main.h"

void on_req_fullscreen(){
  //
  
}


int main(){
  glfwInit();
  //GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  
  GLFWwindow * win = glfwCreateWindow(512, 512, "", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);  
  ASSERT(glewInit() == GLEW_OK);
  context ctx = {.win = win};

  while(glfwWindowShouldClose(ctx.win) == false){
    //iron_usleep(100000);
    mainloop(&ctx);
  }
    
  glfwTerminate();
   
  return 0;

}
