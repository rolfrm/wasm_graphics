
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
  
}context;


void mainloop(context * ctx);
