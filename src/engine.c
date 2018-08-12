
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
#include <iron/utils.h>
#include <iron/log.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>

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

// get the time in microseconds, because nanoseconds is probably too accurate.
u64 get_file_time(const struct stat * stati){
  return stati->st_mtime;
}

u64 get_file_time2(const char * path){
  struct stat st;
  stat(path, &st);
  return st.st_mtime * 1000000;
}


FILE *popen(const char *command, const char *mode);
int pclose(FILE *stream);

void load_level_file(context * ctx, const char * file){
  UNUSED(ctx);
  FILE *cmd;
  char result[1024];

  char scmd[1025];
  sprintf(scmd, "cat %s", file);
  u64 stamp = get_file_time2(file);
  bool reload = false;
  if(ctx->file == file && stamp == ctx->file_modify){
    return;
  }else if(ctx->file != NULL){
    reload = true;
  }
  ctx->file = file;
  ctx->file_modify = stamp;
  cmd = popen(scmd, "r");
  if (cmd == NULL) {
    perror("popen");
    exit(EXIT_FAILURE);
  }
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  if(!reload)
    ctx->player_current_direction = vec2_new(0.1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;
  
  int id = 1;
  while (fgets(result, sizeof(result), cmd)) {
    float x,y,w,h;
    char name[124];
    if(strstr(result, "//") != NULL) continue;
    int r = sscanf(result, "%f %f %f %f %s\n", &x, &y, &w, &h, &name);
    int current_square;
    if(r == 5){
      printf("%i, %f %f %f %f '%s'\n", r, x,y,w,h, name);
      int type = -1;
      if(strcmp(name, "BLOCK") == 0){
	type = SQUARE_BLOCK;
      }else if (strcmp(name, "PLAYER") == 0){
	type = SQUARE_PLAYER;
      }else if (strcmp(name, "WIN") == 0){
	type = SQUARE_WIN;
      }else if (strcmp(name, "LOSE") == 0){
	type = SQUARE_LOSE;
      }
      ASSERT(type != -1);
      if(!(type == SQUARE_PLAYER && reload))
	squares_set(ctx->squares, id, vec2_new(x,y), vec2_new(w, h), type); 
      id += 1;
      
    }else if(sscanf(result, "current_square: %i", &current_square) == 1){
      if(!reload)
	ctx->player_current_square = current_square;
    }else if(sscanf(result, "current_direction: %f %f", &x, &y) == 2){
      if(!reload)
	ctx->player_current_direction = vec2_new(x,y);
    }

  }
  //exit(0);

}

void load_level4(context * ctx){
  squares_set(ctx->squares, 1, vec2_new(0,1.05), vec2_new(0.1, 0.1), SQUARE_PLAYER); 

  squares_set(ctx->squares, 2, vec2_new(0.5,0), vec2_new(1, 1), SQUARE_BLOCK);

  squares_set(ctx->squares, 10, vec2_new(1.75 ,1.00), vec2_new(0.25, 0.25), SQUARE_LOSE);

  squares_set(ctx->squares, 3, vec2_new(2,2.8), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 4, vec2_new(4,0), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 11, vec2_new(3.6 ,0.95), vec2_new(0.5, 0.1), SQUARE_LOSE);
  squares_set(ctx->squares, 21, vec2_new(3.6 ,0.98), vec2_new(0.2, 0.1), SQUARE_WIN);  
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  ctx->player_current_direction = vec2_new(0.1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;    
}

void load_level3(context * ctx){
  squares_set(ctx->squares, 1, vec2_new(0,1.05), vec2_new(0.1, 0.1), SQUARE_PLAYER); 
  
  squares_set(ctx->squares, 2, vec2_new(0,0), vec2_new(1, 1), SQUARE_BLOCK);
  for(int i = 0; i < 3; i++){
    squares_set(ctx->squares, 3 + i, vec2_new(2.5 + i * 2.5,0), vec2_new(1, 1), SQUARE_BLOCK);
    squares_set(ctx->squares, 10 + i, vec2_new(1.25 + i * 2.5,0), vec2_new(0.5, 0.5), SQUARE_LOSE);
  }
  for(int i = 0; i < 3; i++){
    squares_set(ctx->squares, 3 + i + 3, vec2_new(-2.5 - i * 2.5,0), vec2_new(1, 1), SQUARE_BLOCK);
    squares_set(ctx->squares, 10 + i + 3, vec2_new(-1.25 - i * 2.5,0), vec2_new(0.5, 0.5), SQUARE_LOSE);
  }
  squares_set(ctx->squares, 20, vec2_new(1.25,-1.0), vec2_new(0.25, 0.25), SQUARE_WIN);

  
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  ctx->player_current_direction = vec2_new(0.1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;  

}

void load_level2(context * ctx){
  squares_set(ctx->squares, 1, vec2_new(0,1), vec2_new(0.1, 0.1), SQUARE_PLAYER);
  
  squares_set(ctx->squares, 2, vec2_new(0,0), vec2_new(0.5, 0.5), SQUARE_BLOCK);
  squares_set(ctx->squares, 3, vec2_new(0,1.5), vec2_new(0.5, 0.5), SQUARE_BLOCK);
  squares_set(ctx->squares, 4, vec2_new(-1.5,0.0), vec2_new(0.5, 0.5), SQUARE_BLOCK);
  squares_set(ctx->squares, 5, vec2_new(-1.5,-1.5), vec2_new(0.5, 0.5), SQUARE_BLOCK);
  squares_set(ctx->squares, 6, vec2_new(2,0.0), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 7, vec2_new(4.5,0.0), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 8, vec2_new(4.5,2.5), vec2_new(1, 1), SQUARE_BLOCK);
  
  squares_set(ctx->squares, 30, vec2_new(2,2.5), vec2_new(1, 1), SQUARE_LOSE);
  
  squares_set(ctx->squares, 40, vec2_new(5.5,1.5), vec2_new(1, 1), SQUARE_WIN);
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  ctx->player_current_direction = vec2_new(-1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;
}

void load_level1(context * ctx){
  squares_clear(ctx->squares);
  squares_set(ctx->squares, 1, vec2_new(0,1), vec2_new(0.1, 0.1), SQUARE_PLAYER);
  squares_set(ctx->squares, 2, vec2_new(0,0), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 3, vec2_new(3,0), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 4, vec2_new(6.0,0), vec2_new(1, 1), SQUARE_BLOCK);
  squares_set(ctx->squares, 5, vec2_new(9,0), vec2_new(1, 1), SQUARE_BLOCK);
  
  //squares_set(ctx->squares, 30, vec2_new(2,2.5), vec2_new(1, 1), SQUARE_LOSE);
  
  squares_set(ctx->squares, 40, vec2_new(9,1), vec2_new(0.5, 0.5), SQUARE_WIN);
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  ctx->player_current_direction = vec2_new(0.1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;
  
}

void load_level0(context * ctx){

  squares_clear(ctx->squares);
  squares_set(ctx->squares, 1, vec2_new(90,2), vec2_new(0.1, 0.1), SQUARE_PLAYER);
  squares_set(ctx->squares, 2, vec2_new(0,0), vec2_new(100, 1), SQUARE_BLOCK);
  
  
  //squares_set(ctx->squares, 30, vec2_new(2,2.5), vec2_new(1, 1), SQUARE_LOSE);
  
  squares_set(ctx->squares, 30, vec2_new(95,1.6), vec2_new(0.5, 0.5), SQUARE_WIN);
  for(int i = 0; i < 20; i++)
    squares_set(ctx->squares, 40 + i, vec2_new(-100 + i * 5,3), vec2_new(0.5, 0.5), SQUARE_WIN);
      
  
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  ctx->player_current_direction = vec2_new(1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;
  
}



void load_level(context * ctx, int n){
  ctx->current_level = n;
  ctx->game_time = 0.0;
  ctx->player_stick = true;
  squares_clear(ctx->squares);
  
  if(n == 0){
    load_level0(ctx);
  }else if(n == 1){
    load_level1(ctx);
  }
  else if(n == 2){
    load_level2(ctx);
  }else if(n == 3){
    load_level3(ctx);
  }else if(n == 4){
    load_level4(ctx);
  }else if(n == 5){
    load_level_file(ctx,"level1.data");
  }else{
    printf("Game won: %i\n", n);
    exit(0);
  }

}




void run_test();
void initialize(context * ctx){
  run_test();
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
   
  ctx->geom1 = gl_array_2d(data, ctx->geom1_pts);

  ang = 2 * PI / (float)4;
  for(int i = 0; i < 4; i++){
    data[i* 2] = sin(ang * i - PI / 4) * sqrt(2);
    data[i *2 + 1] = cos(ang * i - PI / 4)* sqrt(2) ;
  }
  ctx->square = gl_array_2d(data, 4);

  ctx->squares = squares_create(NULL);

  load_level(ctx, 5);
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

vec2 square_distance(vec2 p_a, vec2 p_b, vec2 s_a, vec2 s_b){
  vec2 dist = vec2_abs(vec2_sub(p_a, p_b));
  vec2 ds = vec2_add(s_a, s_b);
  return vec2_sub(dist, ds);
}

void run_test(){
  vec2 d = square_distance(vec2_new(-0.5,-0.5), vec2_new(1,1), vec2_new(0.5,0.5), vec2_new(1, 1));
  vec2_print(d);
  
  //ERROR("..\n");
}

void mainloop(context * ctx)
{
  if(!ctx->initialized)
    initialize(ctx);

  size_t idx = 0;
  squares_lookup(ctx->squares, &ctx->player_id, &idx, 1);
  float rot = ((int)(ctx->game_time * 0.5)) * PI / 2.0;
  rot = 0;
  glUseProgram(ctx->geoshader);
  ctx->world_tform = mat3_mul(mat3_2d_rotation(rot), mat3_mul( mat3_2d_scale(0.2,0.2), mat3_2d_translation(-ctx->squares->pos[idx].x,-ctx->squares->pos[idx].y)));
  
  glUniform4f(ctx->color_uniform_loc, 1,1,1,1);
  GLFWwindow * win = ctx->win;
  int f = glfwGetKey(win, GLFW_KEY_F);
  int space = glfwGetKey(win, GLFW_KEY_SPACE);
  int enter = glfwGetKey(win, GLFW_KEY_ENTER);
  int r = glfwGetKey(win, GLFW_KEY_R);
  glClearColor(0,0,0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  for(size_t i = 0 ; i < ctx->squares->count; i++){
    square_type type = ctx->squares->type[i+1];
    if( type == SQUARE_LOSE){
      glUniform4f(ctx->color_uniform_loc, 1,0,0,1);
    }else if(type == SQUARE_WIN){
      glUniform4f(ctx->color_uniform_loc, 0,1,0,1);
    }else{
      glUniform4f(ctx->color_uniform_loc, 1,1,1,1);
    }
    render_square(ctx, ctx->squares->pos[i+1],ctx->squares->size[i+1]);
  }
  
  glfwSwapBuffers(ctx->win);
  glfwPollEvents();
  if(f ){
    on_req_fullscreen();
  }

  if(!ctx->paused){
  size_t idx2 = 0;
  squares_lookup(ctx->squares, &ctx->player_current_square, &idx2, 1);
  //ASSERT(idx);
  if(ctx->player_stick){
    // fix player unless jumping.
    vec2 dist = square_distance(ctx->squares->pos[idx2],ctx->squares->pos[idx],ctx->squares->size[idx2],ctx->squares->size[idx]);
    printf("Fixit?..\n");
    vec2_print(dist);logd("\n");
    if(MIN(dist.x, dist.y) < -0.00001){
      printf("yes..\n");
      vec2 p2 = ctx->squares->pos[idx2];
      vec2 p1 = ctx->squares->pos[idx];
      vec2 d3 = vec2_sub(p2, p1);
      if(dist.x > dist.y){
	d3.y = 0;
      }else{
	d3.x = 0;
      }
      d3 = vec2_normalize(vec2_scale(d3, -1));
      
      vec2 perp = d3;
      
      vec2 mov = vec2_scale(vec2_mul(dist, perp), -1);
      logd("move: ");vec2_print(mov);logd("\n");
      if(vec2_len(mov) > 0.000){    
	ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_scale(mov, 0.25));
      }
    }
      
    }
  {
    vec2 dist = square_distance(ctx->squares->pos[idx2],ctx->squares->pos[idx],ctx->squares->size[idx2],ctx->squares->size[idx]);

    float d = MAX(dist.x, dist.y);
    vec2 s1 = ctx->squares->size[idx];
    
    printf("Leave orbit?.. %f %f %f %f %f\n", d, dist.x, dist.y, s1.x, s1.y);
    vec2 p2 = ctx->squares->pos[idx2];
    vec2 p1 = ctx->squares->pos[idx];
    vec2 dd = vec2_sub(p2, p1);
    if(dist.y > -s1.y && !ctx->player_stick ){
      // no vertical collision
      printf(">> %f\n", SIGN(dd.y) * ctx->player_gravity * 0.5);
      ctx->player_current_direction.y += SIGN(dd.y) * ctx->player_gravity * 0.5;
    }
    
    if(dist.x > -s1.x && !ctx->player_stick){
      printf(">>>> %f\n", SIGN(dd.x) * ctx->player_gravity * 0.5);
      // no vertical collision
      ctx->player_current_direction.x += SIGN(dd.x) * ctx->player_gravity * 0.5;
    }
    //float l = vec2_len(ctx->player_current_direction);
    //if(l < 1)
    //ctx->player_current_direction = vec2_scale(ctx->player_current_direction, 1.0 / l );
    //ctx->player_current_direction = vec2_normalize(ctx->player_current_direction);
    if(d > 0.000){ // going to orbit!!
      ctx->player_stick = false;
    }else{
      ctx->player_stick = true;
    }
    
  }
  float l = vec2_len(ctx->player_current_direction);
  if(l < 1 && ctx->player_stick){
    printf("accelerate..\n");
    ctx->player_current_direction = vec2_scale(ctx->player_current_direction, 1.1 );
  }
  if( l > 1){
    ctx->player_current_direction = vec2_scale(ctx->player_current_direction, 0.9 );
  }

  float mindist = 10000;
  int minid = -1;
  
  for(size_t _i = 0 ; _i < ctx->squares->count; _i++){
    size_t i = ctx->squares->count - _i;
    square_type type = ctx->squares->type[i];
    if(ctx->squares->type[i] != SQUARE_PLAYER){
      size_t idx2 = i;
      vec2 dist = square_distance(ctx->squares->pos[idx2],ctx->squares->pos[idx],ctx->squares->size[idx2],ctx->squares->size[idx]);
      //float d = MAX(dist.x, dist.y);
      float d = dist.x;
      if(d < 0){
	d = dist.y;
      }else if(dist.y > 0){
	d = sqrt(d * d + dist.y * dist.y) ;
      }
      if(d < mindist && type == SQUARE_BLOCK){
	mindist = d;
	minid = ctx->squares->id[i];
      }
      if(d < -0.0001){
	if(type == SQUARE_WIN || type == SQUARE_LOSE){
	  ctx->file = NULL;
	  load_level(ctx, ctx->current_level + (type == SQUARE_WIN ? 1 : 0)); return;
	}else if(type == SQUARE_BLOCK){
	  if(dist.x < dist.y){
	    // collided top/bottom.
	    ctx->player_current_direction.y = 0;
	  }else{
	    ctx->player_current_direction.x = 0;
	  }
	  
	  if(vec2_len(ctx->player_current_direction) < 0.0001){
	    if(dist.x < dist.y){
	    // collided top/bottom.
	      //ctx->player_current_direction.x = 1;
	    }else{
	      //ctx->player_current_direction.y = 1;
	    }
	  }
	  //ctx->player_current_direction = vec2_normalize(ctx->player_current_direction);
	  ctx->player_gravity = 1;
	  ctx->player_stick = true;
	}
      }
    }
  }
  
  ctx->player_current_square = minid;
  if(space && ctx->player_stick && ctx->jmpcnt < 0){
    logd("JMP!\n");
    vec2 dist = square_distance(ctx->squares->pos[idx2],ctx->squares->pos[idx],ctx->squares->size[idx2],ctx->squares->size[idx]);
    vec2 p2 = ctx->squares->pos[idx2];
    vec2 p1 = ctx->squares->pos[idx];
    vec2 d = vec2_sub(p2, p1);
    vec2 da = vec2_new(-d.y, d.x);
    float ang = SIGN( vec2_mul_inner(ctx->player_current_direction, da));
    
    if(MIN(dist.x, dist.y) < -0.00001){
      mat2 rot = mat2_rotation(ang * PI/4);
      vec2 jmp = mat2_mul_vec2(rot, ctx->player_current_direction);
      ctx->player_current_direction = jmp;
      ctx->player_gravity = 0.05;
      ctx->player_stick = false;
      ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_scale(ctx->player_current_direction,0.03));
      ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_scale(ctx->player_current_direction,0.03));
      ctx->jmpcnt = 10;
    }
  }



  ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_scale(ctx->player_current_direction,0.06));
  }
  printf("Player: %i",ctx->player_stick); vec2_print(ctx->squares->pos[idx]); vec2_print(ctx->player_current_direction); printf("\n");
  ctx->jmpcnt--;

  ctx->game_time += 0.01;
  if(ctx->file != NULL)
    load_level_file(ctx,ctx->file);
  //ctx->q += 0.001;

  if(r){
    ctx->file = NULL;
    load_level(ctx, ctx->current_level); return;
  }
  if(enter && ctx->paused_cnt < 0){
    ctx->paused = !ctx->paused;
    ctx->paused_cnt = 20;
  }
  ctx->paused_cnt--;
  
}
