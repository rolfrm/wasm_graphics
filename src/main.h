
typedef struct {
  u32 program;
  u32 color_loc;
  u32 offset_loc;
  u32 scale_loc;
}starry;

typedef enum mouse_mode{
  GAME_MODE_UNKNOWN,
  GAME_MODE_TOUCH,
  GAME_MODE_PRESS
}mouse_mode;

typedef struct{
  GLFWwindow * win;
  float p, q;
  float pv, qv;
  int geoshader;
  int initialized;
  int color_uniform_loc;
  int tform_loc;
  int size_loc;

  int geom1_pts;
  int geom1;

  int square;  

  int current_vbo;
  int vbo_points;
  
  mat3 world_tform;
  
  void * alc_device;
  void * alc_context;
  unsigned int  _source;

  squares * squares;
  
  int player_id;
  int player_current_square;
  vec2 player_current_direction;
  vec2 player_gravity_dir;
  bool player_stick;
  float player_gravity;
  int jmpcnt;

  int current_level;

  float game_time;
  
  char * squares_file;
  
  int file;
  u64 file_modify;

  bool paused;
  int paused_cnt;

  particles * particles;
  int particle_counter;
  starry stars;

  u32 jmp_sound;
  u32 win_sound;
  u32 lose_sound;
  char * particles_file;

  int jump;

  int win_height, win_width;

  mouse_mode jmp_mode;
  sin_state * sin_states;
  char * sin_file;
}context;


typedef enum {
  SQUARE_BLOCK,
  SQUARE_PLAYER = 1,
  SQUARE_WIN,
  SQUARE_LOSE

}square_type_id;


void mainloop(context * ctx);
vec2 get_drawing_size();
void on_jumped(context * game);
