
typedef struct {
  u32 program;
  u32 color_loc;
  u32 offset_loc;
}starry;

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
  unsigned int  source;

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
  
  const char * squares_file;
  
  int file;
  u64 file_modify;

  bool paused;
  int paused_cnt;

  particles * particles;
  int particle_counter;
  starry stars; 
}context;


typedef enum {
  SQUARE_BLOCK,
  SQUARE_PLAYER = 1,
  SQUARE_WIN,
  SQUARE_LOSE

}square_type_id;


void mainloop(context * ctx);
