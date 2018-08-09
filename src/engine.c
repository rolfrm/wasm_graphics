
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <iron/types.h>
#include <iron/linmath.h>
#include <iron/math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "flat_geom.c"

#include "main.h"

void on_req_fullscreen();
typedef int (*q2cmp)(const void *, const void*, void * arg);

typedef struct {
  q2cmp cmp;
  void * arg;
}qsort2_data;

static __thread qsort2_data qsort2_arg;


int qsort2_cmp(const void * a, const void * b){
  return qsort2_arg.cmp(a,b,qsort2_arg.arg);
}

void qsort2(void * base, size_t nmemb, size_t size, int (*compare)(const void *, const void*, void * arg), void * arg){
  qsort2_arg.cmp = compare;
  qsort2_arg.arg = arg;
  qsort(base, nmemb, size, qsort2_cmp); 
}

i32 make_shader(u32 kind, char * source, u32 length){
  i32 ref = glCreateShader(kind);
  glShaderSource(ref,1,(const GLchar **)&source,(i32 *) &length);
  glCompileShader(ref);

  GLint status = 0;
  glGetShaderiv(ref, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE){
    GLint infoLogLength = 0;
    glGetShaderiv(ref, GL_INFO_LOG_LENGTH, &infoLogLength);
    char log[infoLogLength];
    glGetShaderInfoLog(ref, infoLogLength, NULL, log);
    printf("Failure to compile shader. Info log:\n **************************\n");
    printf("%s", log);
    printf("\n***************************\n");
    printf("Source:\n");
    printf("%s", source);
    printf("\n***************************\n");
    glDeleteShader(ref);
    return -1;
  }else{
    GLint infoLogLength = 0;
    glGetShaderiv(ref, GL_INFO_LOG_LENGTH, &infoLogLength);
    char log[infoLogLength];
    memset(log, 0, sizeof(log));
    glGetShaderInfoLog(ref, infoLogLength, NULL, log);
    printf("Shader Info log:\n **************************\n");
    printf("%s",log);
    printf("\n***************************\n");
    //printf("Source:\n");
    //printf("%s", source);
    //printf("\n***************************\n");
  }

  return ref;
}


u32 gl_array_2d(float * values, int n){
  u32 vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, n * sizeof(values[0]) * n, values, GL_STATIC_DRAW);
  return vbo;
}

void initialize(context * ctx){
  ctx->initialized = 1;
  GLuint vs = make_shader(GL_VERTEX_SHADER, (char *) src_flat_geom_vs, src_flat_geom_vs_len);
  GLuint fs = make_shader(GL_FRAGMENT_SHADER, (char *) src_flat_geom_fs, src_flat_geom_fs_len);
  GLuint program = 0;
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glBindAttribLocation(program, 0, "pos");
  glLinkProgram(program);
  ctx->geoshader = program;
  ctx->color_uniform_loc = glGetUniformLocation(program, "color");
  ctx->size_loc = glGetUniformLocation(program, "size");
  ctx->tform_loc = glGetUniformLocation(program, "tform");

  ctx->geom1_pts = 3;
  float data[ctx->geom1_pts * 2];
  float ang = 2 * PI / (float)ctx->geom1_pts;
  for(int i = 0; i < ctx->geom1_pts; i++){
    data[i* 2] = sin(ang * i);
    data[i *2 + 1] = cos(ang * i) ;
  }
   
  ctx->geom1 = gl_array_2d(data, 5);
  ALCdevice* device = alcOpenDevice(NULL);
  ALCcontext* context = alcCreateContext(device, NULL);
  ctx->alc_device = device;
  ctx->alc_context = context;
  alcMakeContextCurrent(context);
  
  ALfloat listenerPos[] = {0.0, 0.0, 1.0};
  ALfloat listenerVel[] = {0.0, 0.0, 0.0};
  ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

  alListenerfv(AL_POSITION, listenerPos);
  alListenerfv(AL_VELOCITY, listenerVel);
  alListenerfv(AL_ORIENTATION, listenerOri);

  
  ALuint buffers[1];

  alGenBuffers(1, buffers);
  int size = 200000;
  unsigned char* buffer = (unsigned char*) malloc(size);
  for(int i = 0; i < size; i++){
    buffer[i] = (unsigned char)(200 * (sin(pow((float)i / 2.0, 0.75)) * 0.5 + 0.5));
  }

  ALenum format = AL_FORMAT_MONO8;
  alBufferData(buffers[0], format, &buffer[0], size, 47000);

  ALuint sources[1];
  alGenSources(1, sources);
  alSourcei(sources[0], AL_BUFFER, buffers[0]);
  alSourcePlay(sources[0]);
  ctx->source = sources[0];

}

static int n = 10;

float get_loc(context * ctx,int i){
  float l = (float)i / ((float)(n - 4));
  float loc = ctx->p*0.1 + l;
  loc = fmod(loc, 1.5);
  loc = pow(loc, 2);
    return loc; 
}


void render(context * ctx, int i){
  float loc = get_loc(ctx,i);
  mat3 rot = mat3_2d_rotation(loc + ctx->q);
  glUniformMatrix3fv(ctx->tform_loc, 1, false, &rot.data[0][0]);
  glUniform1f(ctx->size_loc, loc);
  //glUniform4f(ctx->color_uniform_loc, i % 2, (i>>1) %2, (i>>2) % 2, 1);
  glUniform4f(ctx->color_uniform_loc, 1,cos(ctx->p + i) * 0.5 + 0.5, sin(ctx->q + i) * 0.5 + 0.5, 1);
  glDrawArrays(GL_TRIANGLE_FAN, 0, ctx->geom1_pts);
  
}

int compr(const void * a, const void * b, void * c){
  int ia = *((int*)a);
  int ib = *((int*)b);
  float * f = c;
  float fa = f[ia];
  float fb = f[ib];
  if (fa > fb)
    return 1;
  
  if(fb > fa)
    return -1;
  return 0;
}

void mainloop(context * ctx)
{
  if(!ctx->initialized)
    initialize(ctx);
  glUseProgram(ctx->geoshader);
  GLFWwindow * win = ctx->win;
  int up = glfwGetKey(win, GLFW_KEY_UP);
  int down = glfwGetKey(win, GLFW_KEY_DOWN);
  int right = glfwGetKey(win, GLFW_KEY_RIGHT);
  int left = glfwGetKey(win, GLFW_KEY_LEFT);
  int space = glfwGetKey(win, GLFW_KEY_SPACE);
  int f = glfwGetKey(win, GLFW_KEY_F);
  //int up = glfwGetKey(win, GLFW_KEY_UP);
  //  int down = glfwGetKey(win, GLFW_KEY_DOWN);
  //  
  //  
  //  int w = glfwGetKey(win, GLFW_KEY_W);
  //  int s = glfwGetKey(win, GLFW_KEY_S);
  
  glClearColor(0,0,0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindBuffer(GL_ARRAY_BUFFER, ctx->geom1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);

  float locs[n];
  int is[n];
  for(int i = 0; i < n; i++){
    locs[i] = get_loc(ctx, i);
    is[i] = i;
  }
  
  //qsort (x, sizeof(x)/sizeof(*x), sizeof(*x), comp);
  qsort2(is, n, sizeof(is[0]), compr, locs);
  
  for(int i = n -1 ; i > 0; i--){
    render(ctx,is[i]);
  }
  
  glfwSwapBuffers(ctx->win);
  glfwPollEvents();
  ctx->pv += (up - down) * 0.01;
  ctx->qv += (left - right) * 0.005;
  ctx->p += ctx->pv;
  ctx->q += ctx->qv;
  float damp = 1.0;//0.999;
  if(space || f){
    damp = 0.9;
    int state;
    alGetSourcei(ctx->source, AL_SOURCE_STATE, &state);
    if(state == AL_STOPPED)
      alSourcePlay(ctx->source);
    
  }
  ctx->pv *= damp;
  ctx->qv *= damp;

  if(f ){
    on_req_fullscreen();
  }
}
