
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
#include "starry.c"
#include "squares.h"
#include "squares.c"
#include "particles.h"
#include "particles.c"
#include "sin_state.h"
#include "sin_state.c"
#include "main.h"
#include "utils.h"
#include "audio.h"
#include "level1.c"

__thread int logd_enable = false;
void on_req_fullscreen();
unsigned query_sample_rate_of_audiocontexts();
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

void load_file_lines(const char * filedata, size_t len, void (* load_line)(const char * line, void * userdata), void * userdata){
  while(filedata != NULL && *filedata && len > 0){
    char * pt1 = memmem(filedata, len, "\n", 1);
    char * pt2 = memmem(filedata, len, "//", 2);
    char buffer[512];
    size_t l = 0;
    const char * loc = filedata;
    if(pt2 != NULL && (pt2 < pt1 || pt1 == NULL)){
      l = pt2 - filedata;
      filedata = pt2 + 2;
    }else if(pt1 != NULL){
      l = pt1 - filedata;
      filedata = pt1 + 1;
    }else{
      l = len;
      filedata += len;
    }
    len -= l;
    memcpy(buffer, loc, l);
    buffer[l] = 0;
    if(len > 0){
      load_line(buffer, userdata);
    }
  }
}

typedef struct{
  int id;
  context * ctx;
}level_load_data;

void load_level_data_f(const char * line, void * userdata){
  level_load_data * dat = userdata;
  int id = dat->id;
  context * ctx = dat->ctx;
  float x,y,w,h;
  char name[124];
  char mode[124];
  float m1,m2,m3,m4;
  int r = sscanf(line, "%f %f %f %f %s\n", &x, &y, &w, &h, &name);
  int current_square;
    if(r == 5){
      printf("%i :: %i, %f %f %f %f '%s'\n",id, r, x,y,w,h, name);
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
      
      printf("?? type: %i\n", type);
      if(type == SQUARE_LOSE){

	r = sscanf(line, "%f %f %f %f %s %s %f %f %f %f\n", &x, &y, &w, &h, &name, &mode, &m1,&m2,&m3,&m4 );
	printf("Reading again: %i\n", r);
	if(r > 5){
	  printf("Read mode: %s %f %f %f %f\n", mode, m1, m2, m3, m4);
	  if (strcmp(mode, "SIN") == 0){
	    
	    float dx = m1, dy = m2, freq = m3, phase = m4;
	    sin_state_set(dat->ctx->sin_states, id, vec2_new(dx,dy),freq,phase);
	    
	  }
	}else{
	  sscanf(line, "%f %f %f %f %s\n", &x, &y, &w, &h, &name);
	}

      }

      
      ASSERT(type != -1);
      squares_set(ctx->squares, id, vec2_new(x,y), vec2_new(w, h), type);
      if(type == SQUARE_PLAYER)
	dat->ctx->player_id = id;
      dat->id += 1;
      
    }else if(sscanf(line, "current_square: %i", &current_square) == 1){
      ctx->player_current_square = current_square;
    }else if(sscanf(line, "current_direction: %f %f", &x, &y) == 2){
      ctx->player_current_direction = vec2_new(x,y);
    }
}


void load_level_data(context * ctx, const char * filedata, size_t len){
  ctx->player_id = 1;
  ctx->player_current_square = 2;
  ctx->player_current_direction = vec2_new(0.1,0);
  ctx->player_stick = true;
  ctx->player_gravity = 1;
  level_load_data dat = {.id = 1, .ctx = ctx};
  load_file_lines(filedata, len, load_level_data_f, &dat);
}

void load_level_file(context * ctx, int fileid){
  UNUSED(ctx);
  FILE *cmd;
  char result[1024];
  char file[245];
  sprintf(file, "level%i.data", fileid);

  char scmd[1025];
  sprintf(scmd, "cat %s", file);
  u64 stamp = get_file_time2(file);
  bool reload = false;
  if(ctx->file == fileid && stamp == ctx->file_modify){
    return;
  }else if(ctx->file > 0){
    reload = true;
  }
  logd("RELOADING? %i\n");
  ctx->file = fileid;
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
  
  level_load_data dat = {.id = 1, .ctx = ctx};
  while (fgets(result, sizeof(result), cmd)) {
    load_level_data_f(result, &dat);

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
  sin_state_clear(ctx->sin_states);
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
    load_level_data(ctx, (char *)level1_data, level1_data_len);
  }else if(n == 6){
    load_level_file(ctx,2);
  }else if(n == 7){
    load_level_file(ctx,3);
  }else if(n == 8){
    load_level_file(ctx,4);
  }else{
    load_level_file(ctx,4);
    ctx->current_level = 8;
    printf("Game won: %i\n", n);
    //exit(0);
  }

}

u32 create_sound(short * samples, int count){
  ALuint buffers[1];
  alGenBuffers(1, buffers);
  
  ALenum format = AL_FORMAT_MONO16;
  alBufferData(buffers[0], format, samples, count, 47000);

  ALuint sources[1];
  alGenSources(1, sources);
  alSourcei(sources[0], AL_BUFFER, buffers[0]);
  return sources[0];

}


u32 create_soundf(float * samples, int count){
  ALuint buffers[1];
  alGenBuffers(1, buffers);
  
  ALenum format = AL_FORMAT_MONO16;
  short * data = malloc(count * sizeof(short));
  for(int i = 0; i < count; i++)
    data[i] = samples[i] * 30000.0;
  alBufferData(buffers[0], format, data, count, samplerate);
  free(data);
  ALuint sources[1];
  alGenSources(1, sources);
  alSourcei(sources[0], AL_BUFFER, buffers[0]);
  return sources[0];

}



u32 load_shader_program(const char * vscode, int vslen, const char * fscode, int fslen){
  GLuint vs = make_shader(GL_VERTEX_SHADER, (char *) vscode, vslen);
  GLuint fs = make_shader(GL_FRAGMENT_SHADER, (char *) fscode, fslen);
  GLuint program = 0;
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glBindAttribLocation(program, 0, "pos");
  glLinkProgram(program);
  return program;
}


void run_test();
void initialize(context * ctx){
  glfwSetWindowUserPointer(ctx->win, ctx);
  logd("Initializing..\n");
  run_test();
  ctx->initialized = 1;
  
  var program = load_shader_program((char *)src_flat_geom_vs, src_flat_geom_vs_len,(char *)src_flat_geom_fs, src_flat_geom_fs_len);
  ctx->geoshader = program;
  ctx->color_uniform_loc = glGetUniformLocation(program, "color");
  ctx->tform_loc = glGetUniformLocation(program, "tform");

  ctx->stars.program = load_shader_program((char *)src_starry_vs, src_starry_vs_len,(char *)src_starry_fs, src_starry_fs_len);
  ctx->stars.color_loc = glGetUniformLocation(ctx->stars.program, "color");
  ctx->stars.offset_loc = glGetUniformLocation(ctx->stars.program, "offset");
  ctx->stars.scale_loc = glGetUniformLocation(ctx->stars.program, "scale");
  
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

  ctx->squares = squares_create(ctx->squares_file);
  ctx->particles = particles_create(ctx->particles_file);
  ctx->sin_states = sin_state_create(ctx->sin_file);
  ctx->current_vbo = -1;

  load_level(ctx, 8);
  ALCdevice* device = alcOpenDevice(NULL);
  
  printf("ALC DEVICE: %p\n", device);

  ALCcontext* context = alcCreateContext(device, NULL);
  ctx->alc_device = device;
  ctx->alc_context = context;
  alcMakeContextCurrent(context);

  samplerate = query_sample_rate_of_audiocontexts();
  printf("ALC SAMPLE Rate: %i\n", samplerate);
  ALfloat listenerPos[] = {0.0, 0.0, 1.0};
  ALfloat listenerVel[] = {0.0, 0.0, 0.0};
  ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

  alListenerfv(AL_POSITION, listenerPos);
  alListenerfv(AL_VELOCITY, listenerVel);
  alListenerfv(AL_ORIENTATION, listenerOri);

  
  int size = 4000;
  float * buffer = malloc(size * sizeof(float));
  for(int i = 0; i < size; i++){
    buffer[i] = sin(pow(i * 0.02,2.0) * 0.1) * 0.5 + sin(pow(i * 0.03,2.0) * 0.2) * 0.5;
  }

  
  ctx->jmp_sound = create_soundf(buffer, size);
  
  free(buffer);
  size = 20000;
  buffer= malloc(size * sizeof(float));
  for(int i = 0; i < size; i++){
    buffer[i] = sin(i / 3.0) * sin(i / 10.0 * 1.0 * (0.9 + 0.1 * cos(i / 200.0))) ;
    buffer[i] *= (i > 10000 ? (i - 10000)/10000.0: 1);
    buffer[i] *= (i < 2000 ? i/2000.0: 1);
  }
  
  ctx->lose_sound = create_soundf(buffer, size);
  free(buffer);
  size = 50000;
  buffer= calloc(size * sizeof(float), 1);

  {
    var sine = create_sine(440);
    var env = create_adsr(0.1, 0.2, 0.4, 0.1);
    env->sub = sine;
    for(int i = 0; i < size; i++){
      //buffer[i] = sin(pow(i * 0.01, 2.0) * 0.1);
      env->f(buffer, i, env);
    }
  }
  {
    var sine = create_sine(440 * 1.5);
    var env = create_adsr(0.1, 0.2, 0.4, 0.1);
    env->sub = sine;
    for(int i = 5000; i < size; i++){
      //buffer[i] = sin(pow(i * 0.01, 2.0) * 0.1);
      env->f(buffer + 5000, i - 5000, env);
    }
  }
  {
    var sine = create_sine(440 * 1.6);
    var env = create_adsr(0.1, 0.2, 0.4, 0.1);
    env->sub = sine;
    for(int i = 10000; i < size; i++){
      //buffer[i] = sin(pow(i * 0.01, 2.0) * 0.1);
      env->f(buffer + 10000, i - 10000, env);
      buffer[i] *= 0.33;
    }
  }
  {
    var sine = create_sine(440 * 1.5);
    var env = create_adsr(0.1, 0.2, 0.4, 0.1);
    env->sub = sine;
    for(int i = 15000; i < size; i++){
      //buffer[i] = sin(pow(i * 0.01, 2.0) * 0.1);
      env->f(buffer + 15000, i - 15000, env);
      buffer[i] *= 0.33;
    }
  }
  //exit(0);
  
  ctx->win_sound = create_soundf(buffer, size);
  //free(buffer);
  
  //ctx->jmp_sound = ctx->win_sound;
  //ctx->lose_sound = ctx->win_sound;
  
  
  
  alSourcef (ctx->lose_sound, AL_GAIN, 0.25);
  alcSuspendContext(context);
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


void render_square2(context * ctx){
  if(ctx->current_vbo != ctx->square){
    glBindBuffer(GL_ARRAY_BUFFER, ctx->square);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);
    ctx->current_vbo = ctx->square;
    ctx->vbo_points = 4;
  }

  glDrawArrays(GL_TRIANGLE_FAN, 0, ctx->vbo_points);
}


vec2 square_distance(vec2 p_a, vec2 p_b, vec2 s_a, vec2 s_b){
  vec2 dist = vec2_abs(vec2_sub(p_a, p_b));
  vec2 ds = vec2_add(s_a, s_b);
  return vec2_sub(dist, ds);
}


void test_load_file_lines(const char * data, void * userdata){
  printf("DATA: %s\n", data);
  int ** lengths = userdata;
  **lengths = strlen(data);
  *lengths += 1;

}

void run_test(){
  vec2 d = square_distance(vec2_new(-0.5,-0.5), vec2_new(1,1), vec2_new(0.5,0.5), vec2_new(1, 1));
  vec2_print(d);

  {
    const char * data = "hello\nasdasd//xasd\n";

    int lengths[3] = {0};
    int *ptr = lengths;
    load_file_lines(data, strlen(data), test_load_file_lines, &ptr);
    ASSERT(lengths[0] == strlen("hello"));
    ASSERT(lengths[1] == strlen("asdasd"));
  }
  //ERROR("..\n");
}

void particle_push(context * ctx, vec2 pos, vec2 dir, float size, float lifetime, particle_type type){
  int id = ctx->particle_counter++;
  particles_set(ctx->particles, id, pos, dir, size, lifetime, type);
}
 
void mainloop(context * ctx)
{
  if(!ctx->initialized){
    initialize(ctx);
  }
  alcMakeContextCurrent(ctx->alc_context);

  vec2 winsize = get_drawing_size();
  int nw = winsize.x;
  int nh = winsize.y;
  if(ctx->win_height != nh || ctx->win_width != nw){
    printf("NEW WINDOW SIZE: %i %i\n", nw, nh);
  
    glfwSetWindowSize(ctx->win, nw, nh);
    ctx->win_height = nh;
    ctx->win_width  = nw;
    int ms = MIN(nh,nw);
    int x = (nw - ms) / 2;
    int y = (nh - ms) / 2;
    glViewport(x,y,ms, ms);
  }
  


  
  size_t idx = 0;
  squares_lookup(ctx->squares, &ctx->player_id, &idx, 1);
  float rot = ((int)(ctx->game_time * 0.5)) * PI / 2.0;
  rot = 0;
  glUseProgram(ctx->geoshader);
  float world_scale = 0.2;
  ctx->world_tform = mat3_mul(mat3_2d_rotation(rot), mat3_mul( mat3_2d_scale(world_scale,world_scale), mat3_2d_translation(-ctx->squares->pos[idx].x,-ctx->squares->pos[idx].y)));
  
  glUniform4f(ctx->color_uniform_loc, 1,1,1,1);
  GLFWwindow * win = ctx->win;
  int f = glfwGetKey(win, GLFW_KEY_F);
  //int space = glfwGetKey(win, GLFW_KEY_SPACE);
  int enter = glfwGetKey(win, GLFW_KEY_ENTER);
  int r = glfwGetKey(win, GLFW_KEY_R);
  glClearColor(0.01,0.01,0.04, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  {  // render stars!

    glUseProgram(ctx->stars.program);
    int nh = ctx->win_height;
    int nw = ctx->win_width;
    int ms = MIN(nh,nw);
    glUniform1f(ctx->stars.scale_loc, 2 * 500.0 / ms);
    //glUniform4f(ctx->stars.color_loc, 0, 1, 0, 1);

    glUniform2f(ctx->stars.offset_loc, 0.01 * ctx->squares->pos[idx].x,0.01 * ctx->squares->pos[idx].y);
    render_square2(ctx);
    
    glUniform2f(ctx->stars.offset_loc, 0.05 * ctx->squares->pos[idx].x,0.05 * ctx->squares->pos[idx].y);
    render_square2(ctx);
    
    glUniform2f(ctx->stars.offset_loc, 0.03 * ctx->squares->pos[idx].x+ 2.0,0.03 * ctx->squares->pos[idx].y + 2.0);
    render_square2(ctx);
    glUniform2f(ctx->stars.offset_loc, 0.01 * ctx->squares->pos[idx].x+ 4.0,0.01 * ctx->squares->pos[idx].y + 4.0);
    render_square2(ctx);
    glUniform2f(ctx->stars.offset_loc, 0.005 * ctx->squares->pos[idx].x+ 18.0,0.005 * ctx->squares->pos[idx].y + 4.0);
    render_square2(ctx);
  }
  
  //
  glUseProgram(ctx->geoshader);
  
  for(size_t i = 0 ; i < ctx->squares->count; i++){
    square_type type = ctx->squares->type[i+1];
    if( type == SQUARE_LOSE){
      glUniform4f(ctx->color_uniform_loc, 1,0,0,1);
    }else if(type == SQUARE_WIN){
      glUniform4f(ctx->color_uniform_loc, 0,1,0,1);
    }else if(type == SQUARE_PLAYER){
      glUniform4f(ctx->color_uniform_loc, 0.5,0.5,1,1);
      vec2 p = ctx->squares->pos[i+1];
      vec2 s = ctx->squares->size[i+1];
      render_square(ctx, p,s);
      var s2 = vec2_mul(s, vec2_new(0.15,0.4));
      vec2 p2 = vec2_add(p, s2);
      glUniform4f(ctx->color_uniform_loc, 0.0,0.0,0,1);

      render_square(ctx, p2,s2);
      vec2 p3 = vec2_add(p2, vec2_new(s2.x * 3,0));
      render_square(ctx, p3,s2);
      
    }
    else{
      glUniform4f(ctx->color_uniform_loc, 1,1,1,1);
    }
    if(type != SQUARE_PLAYER)
      render_square(ctx, ctx->squares->pos[i+1],ctx->squares->size[i+1]);
  }

  int to_remove = 0;
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  var particles = ctx->particles;
  for(size_t _i = 0; _i < ctx->particles->count; _i++){
    
    size_t i = _i + 1;
    
    var lifetime = particles->lifetime[i] * 5;
    glUniform4f(ctx->color_uniform_loc, 1,1,0,lifetime < 1 ? lifetime : 1);
    render_square(ctx, particles->pos[i+1],vec2_new1(particles->size[i+1]));
    particles->pos[i] = vec2_add(particles->pos[i], vec2_scale(particles->dir[i], 0.01));
    
    particles->lifetime[i] -= 0.01;
    if(particles->lifetime[i] < 0){
      to_remove++;
    }
  }
  
  glDisable(GL_BLEND);
  if(to_remove > 0){
    int j =0;
    int ids[to_remove];
    var particles = ctx->particles;
    for(size_t _i = 0; _i < ctx->particles->count; _i++){
      size_t i = _i + 1;
      if(particles->lifetime[i] < 0){
	ids[j] = particles->id[i];
	j++;
      }
    }
    particles_remove(particles, ids, to_remove);
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
    if(MIN(dist.x, dist.y) < -0.00001){
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
      if(vec2_len(mov) > 0.000){    
	ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_scale(mov, 0.25));
      }
    }
      
    }
  {
    vec2 dist = square_distance(ctx->squares->pos[idx2],ctx->squares->pos[idx],ctx->squares->size[idx2],ctx->squares->size[idx]);

    float d = MAX(dist.x, dist.y);
    vec2 s1 = ctx->squares->size[idx];
    
    vec2 p2 = ctx->squares->pos[idx2];
    vec2 p1 = ctx->squares->pos[idx];
    vec2 dd = vec2_sub(p2, p1);
    if(dist.y > -s1.y && !ctx->player_stick ){
      // no vertical collision
      ctx->player_current_direction.y += SIGN(dd.y) * ctx->player_gravity * 0.5;
    }
    
    if(dist.x > -s1.x && !ctx->player_stick){
      
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
	  if(type == SQUARE_LOSE)
	    alSourcePlay(ctx->lose_sound);
	  if(type == SQUARE_WIN)
	    alSourcePlay(ctx->win_sound);
	  ctx->file = 0;
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
  if(ctx->jump && ctx->player_stick){

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
      var pos = ctx->squares->pos[idx];
      ctx->squares->pos[idx] = vec2_add(pos, vec2_scale(ctx->player_current_direction,0.03));
      ctx->squares->pos[idx] = vec2_add(pos, vec2_scale(ctx->player_current_direction,0.03));
      for(int i = 0; i < 10; i++){
	particle_push(ctx, pos, vec2_new(cos(2 * PI * randf32()), sin(2 * PI *randf32())), 0.04, 0.4, 1);
      }
      alSourcePlay(ctx->jmp_sound);
    }
  }
  ctx->jump = false;



  ctx->squares->pos[idx] = vec2_add(ctx->squares->pos[idx], vec2_scale(ctx->player_current_direction,0.06));
  }
  
  {
    var tsin = ctx->sin_states;
    size_t indexes[tsin->count];
    squares_lookup(ctx->squares, tsin->id + 1, indexes, tsin->count);
    for(size_t _i = 0; _i < tsin->count; _i++){

      size_t i = _i + 1;
      float cphase = tsin->phase[i];
      float freq = tsin->freq[i];
      vec2 d =tsin->dir[i];
      float nphase = cphase + 0.01;
      tsin->phase[i] = nphase;
      float dif = sin(nphase * freq) - sin(cphase * freq);

      var sqindex = indexes[_i];
      ctx->squares->pos[sqindex] = vec2_add(ctx->squares->pos[sqindex], vec2_scale(d, dif));
    }
  }
  //printf("Player: %i",ctx->player_stick); vec2_print(ctx->squares->pos[idx]); vec2_print(ctx->player_current_direction); printf("\n");

  ctx->game_time += 0.01;
  if(ctx->file > 0)
    load_level_file(ctx,ctx->file);
  //ctx->q += 0.001;

  if(r){
    ctx->file = -1;
    load_level(ctx, ctx->current_level); return;
  }
  if(enter && ctx->paused_cnt < 0){
    ctx->paused = !ctx->paused;
    ctx->paused_cnt = 20;
    vec2 dsize = get_drawing_size();
    printf("%f %f\n", dsize.x, dsize.y);
  }
  ctx->paused_cnt--;
  
}
