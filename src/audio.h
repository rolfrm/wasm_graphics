
struct _audiothing;
typedef struct _audiothing audiothing;

typedef void (* filter)(float * out, int sample, audiothing * thing);

struct _audiothing{
  void * data;
  filter f;
  audiothing * sub;
};

typedef struct{
  int samplerate;

}audiothing_ctx;

audiothing * create_sine(float freq);
audiothing * create_envelope(float a, float d, float r, float s);
audiothing * create_adsr(float a, float d, float s, float r);
extern int samplerate;
