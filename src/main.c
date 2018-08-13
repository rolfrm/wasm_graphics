
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
#include <iron/utils.h>
#define ASSERT(x) if(x){}
#include "squares.h"
#include "particles.h"
#include "main.h"

void on_req_fullscreen(){
  //
  
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  UNUSED(button);UNUSED(mods);
  if(action == GLFW_PRESS){
    context * ctx= glfwGetWindowUserPointer(window);
    ctx->jump = true;
  }
    
}

unsigned query_sample_rate_of_audiocontexts(){
  return 44100;
}

vec2 get_drawing_size(){
  return vec2_new(1000,700);
}

int main(){
  glfwInit();
  //GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  
  GLFWwindow * win = glfwCreateWindow(700, 700, "", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);
  glfwSetMouseButtonCallback(win, mouse_button_callback);
    
  ASSERT(glewInit() == GLEW_OK);
  //glViewport(512,512,512,512);

  icy_mem * mem = icy_mem_create("game_state");
  icy_mem_realloc(mem, sizeof(context));

  context * ctx = mem->ptr;
  //memset(ctx,0,sizeof(*ctx));
  ctx->squares_file = (char *)"squares";
  ctx->particles_file = (char *)"particles";
  ctx->initialized = false;
  ctx->win = win;
  ctx->win_height = 0;
  ctx->win_width = 0
  while(glfwWindowShouldClose(ctx->win) == false){
    //iron_usleep(100000);
    mainloop(ctx);
  }
    
  glfwTerminate();
   
  return 0;

}
