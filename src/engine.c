
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
#include "squares.h"
#include "squares.c"
#include "main.h"
#include "utils.h"
void on_req_fullscreen();

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
  ctx->tform_loc = glGetUniformLocation(program, "tform");

  ctx->geom1_pts = 3;
  float data[100];
  float ang = 2 * PI / (float)ctx->geom1_pts;
  for(int i = 0; i < ctx->geom1_pts; i++){
    data[i* 2] = cos(ang * i );
    data[i *2 + 1] = sin(ang * i) ;
  }
   
  ctx->geom1 = gl_array_2d(data, 5);

  ang = 2 * PI / (float)4;
  for(int i = 0; i < 4; i++){
    data[i* 2] = sin(ang * i - PI / 4);
    data[i *2 + 1] = cos(ang * i - PI / 4) ;
  }
  ctx->square = gl_array_2d(data, 5);

  ctx->squares = squares_create(NULL);

  squares_set(ctx->squares, 1, vec2_new(0,0), vec2_new(0.1, 0.1), 1);
  squares_set(ctx->squares, 2, vec2_new(0,-0.5), vec2_new(0.5, 0.5), 2);
  ctx->player_id = 1;
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
  alSourcef (ctx->source, AL_GAIN, 0);
}

void render_square(context * ctx, vec2 pos, vec2 size){
  if(ctx->current_vbo != ctx->square){
    glBindBuffer(GL_ARRAY_BUFFER, ctx->square);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    ctx->current_vbo = ctx->square;
    ctx->vbo_points = 4;
  }

  mat3 m1 = mat3_2d_translation(pos.x, pos.y);
  mat3 m2 = mat3_2d_scale(size.x, size.y);
  mat3 m3 = mat3_mul(ctx->world_tform, mat3_mul(m1, m2));

  glUniformMatrix3fv(ctx->tform_loc, 1, false, &m3.data[0][0]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, ctx->vbo_points);
}

void mainloop(context * ctx)
{
  if(!ctx->initialized)
    initialize(ctx);
  glUseProgram(ctx->geoshader);
  float rotation = 0;
  ctx->world_tform = mat3_mul(mat3_2d_rotation(rotation), mat3_2d_scale(0.3,0.3));
  
  glUniform4f(ctx->color_uniform_loc, 1,1,1,1);
  GLFWwindow * win = ctx->win;
  int f = glfwGetKey(win, GLFW_KEY_F);
  
  glClearColor(0,0,0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  for(size_t i = 0 ; i < ctx->squares->count; i++){
    render_square(ctx, ctx->squares->pos[i+1],ctx->squares->size[i+1]);
  }
  
  
  
  glfwSwapBuffers(ctx->win);
  glfwPollEvents();
  if(f ){
    on_req_fullscreen();
  }

  size_t idx = 0;
  squares_lookup(ctx->squares, &ctx->player_id, &idx, 1);
  //ASSERT(idx);
  ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_new(0.01, 0.01));

  
}
