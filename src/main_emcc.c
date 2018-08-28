#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iron/types.h>
#include <iron/linmath.h>
#include <iron/log.h>
#include <iron/utils.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_asm.h>

#include "squares.h"
#include "particles.h"
#include "sin_state.h"
#include "main.h"

#define ASSERT(x) if(x){}

void on_req_fullscreen(){
  //emscripten_request_fullscreen("canvas", 0);
  EmscriptenFullscreenChangeEvent fsce;
  EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);
  if (!fsce.isFullscreen) {
    printf("Requesting fullscreen..\n");
    ret = emscripten_request_fullscreen("canvas", 1);
    printf("req fullscreen\n");
  } else {
    printf("Exiting fullscreen..\n");
    ret = emscripten_exit_fullscreen();
    
    ret = emscripten_get_fullscreen_status(&fsce);
    
    if (fsce.isFullscreen) {
      fprintf(stderr, "Fullscreen exit did not work!\n");
    }  
  }
}



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  context * ctx= glfwGetWindowUserPointer(window);
  if(ctx->jmp_mode == GAME_MODE_TOUCH) return;
  ctx->jmp_mode = GAME_MODE_PRESS;
  UNUSED(button);UNUSED(mods);
  if(action == GLFW_PRESS){
    on_jumped(ctx);
    //on_req_fullscreen();

  }
    
}
unsigned query_sample_rate_of_audiocontexts() {
  char * code = "var AudioContext = window.AudioContext || window.webkitAudioContext; \
        var ctx = new AudioContext();\
        var sr = ctx.sampleRate;\
        ctx.close();\
        return sr;" ;
  return emscripten_asm_const_int(code);
}


int queryw() {
  char * code = "return document.querySelector(\"body\").getBoundingClientRect().width;" ;
  return emscripten_asm_const_int(code);
}

int queryh() {
  char * code = "return document.querySelector(\"body\").getBoundingClientRect().height;" ;
  return emscripten_asm_const_int(code);
}

vec2 get_drawing_size(){
  double w, h;
  emscripten_get_element_css_size("body", &w, &h);
  return vec2_new(w,h);
}

EM_BOOL touch_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
  context * ctx = userData;
  ctx->jump = true;
  if(ctx->jmp_mode == GAME_MODE_PRESS) return 0;
  ctx->jmp_mode = GAME_MODE_TOUCH;
  return 0;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
  if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "f") || e->which == 102)) {
    EmscriptenFullscreenChangeEvent fsce;
    EMSCRIPTEN_RESULT ret = emscripten_get_fullscreen_status(&fsce);
    if (!fsce.isFullscreen) {
      printf("Requesting fullscreen..\n");
      ret = emscripten_request_fullscreen("canvas", 1);
    } else {
      printf("Exiting fullscreen..\n");
      ret = emscripten_exit_fullscreen();

      ret = emscripten_get_fullscreen_status(&fsce);

      if (fsce.isFullscreen) {
        fprintf(stderr, "Fullscreen exit did not work!\n");
      }
    }
  }
  else if (eventType == EMSCRIPTEN_EVENT_KEYPRESS && (!strcmp(e->key, "Esc") || !strcmp(e->key, "Escape") || e->which == 27)) {
    emscripten_exit_soft_fullscreen();
  }
  return 0;
}



int main(){

  logd_enable = true;
  glfwInit();

  const int simulate_infinite_loop = 1; // call the function repeatedly
  const int fps = -1; // call the function as fast as lhe browser wants to render (typically 60fps)

  EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(0, 0, 1, key_callback);
  GLFWmonitor * monitor = glfwGetPrimaryMonitor();
  int w, h;
  glfwGetMonitorPhysicalSize(monitor, &w, &h);
  printf("%i %i\n", w, h);
  //GLFWwindow * win = glfwCreateWindow(1024, 1024, "", monitor, NULL);
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  printf("%i %i %i\n", mode, mode->width, mode->height);
  //GLFWwindow* win = glfwCreateWindow(mode->width, mode->height, "My Title", monitor, NULL);  
  logd("Initializing 1..\n");
  GLFWwindow * win = glfwCreateWindow(queryw(), queryh(), "", NULL, NULL);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(2);
  glfwSetMouseButtonCallback(win, mouse_button_callback);
    
  ASSERT(glewInit() == GLEW_OK);
  context ctx = {.win = win};
  ctx.particles_file = NULL;//"particles";
  ctx.squares_file = NULL;//"squares";
  emscripten_set_touchstart_callback(0, &ctx, 1, touch_callback);
  emscripten_set_main_loop_arg((void *) &mainloop, &ctx, fps, simulate_infinite_loop);
 
    
  glfwTerminate();
   
  return EXIT_SUCCESS;

}
