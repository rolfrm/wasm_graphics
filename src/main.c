
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
#include "sin_state.h"
#include "main.h"

#include <AL/al.h>
#include <AL/alc.h>
void on_req_fullscreen(){
  //
  
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  UNUSED(button);UNUSED(mods);
  if(action == GLFW_PRESS){
    context * ctx= glfwGetWindowUserPointer(window);
    on_jumped(ctx);
  }    
}
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods){
  UNUSED(scancode); UNUSED(mods);
  if(action == GLFW_PRESS && key == GLFW_KEY_SPACE){
    context * ctx= glfwGetWindowUserPointer(window);
    on_jumped(ctx);

  }
}

unsigned query_sample_rate_of_audiocontexts(){
  return 44100;
}
GLFWwindow * win;
vec2 get_drawing_size(){
  if(win != NULL){
    int w, h;
    glfwGetWindowSize(win, &w, &h);
    return vec2_new(w, h);
  }
  return vec2_new(1000,700);
}


float rnd(vec2 x){
  int a = (int)(x.x * 100);
  int b = (int)(x.y * 100);
      
   int c = 43051 + a * 6967 + a * a * 17383 + a * a * a * 29443 + a * a * a * a * 38921;
   int d =  6967 + b * 43051 + b * b * 29443 + b * b * b * 17383 + b * b * b * b * 38921;
   return (float)((c * d) % 10000) / 10000.0;   
}


int main(){
  //for(int i = 0; i < 100; i++)
  //printf("%f\n", rnd(vec2_new(1,i)));
  //return 0;
  glfwInit();
  //GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  
  win = glfwCreateWindow(512, 512, "my window", NULL, NULL);
  glfwMakeContextCurrent(win);
  //glfwSwapInterval(1);
  glfwSetMouseButtonCallback(win, mouse_button_callback);
  glfwSetKeyCallback(win, key_callback);
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
  ctx->win_width = 0;
  ctx->sin_file = (char *) "sin_file";
  while(glfwWindowShouldClose(ctx->win) == false){
    mainloop(ctx);
  }
    
  glfwTerminate();
   
  return 0;

}
