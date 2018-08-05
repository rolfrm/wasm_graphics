
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
  void * alc_device;
  void * alc_context;
  unsigned int  source;
}context;


void mainloop(context * ctx);
