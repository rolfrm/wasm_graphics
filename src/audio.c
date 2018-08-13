#include "audio.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int samplerate = 44100;
void sinefilter(float * out, int sample, audiothing * thing){
  float f = ((float *)&thing->data)[0];
  out[sample] += sin((float)(sample / (float) samplerate) * f * 2 * 3.14) * 0.5;
}

audiothing * create_sine(float freq){
  audiothing * thing = calloc(sizeof(audiothing), 1);
  ((float *)&thing->data)[0] = freq;
  thing->f = sinefilter;
  return thing;
}

typedef struct{
  float a,d,s,r;
}asdr;

void asdrfilter(float * out, int sample, audiothing * thing){
  float time = (float)((float)sample / (float) samplerate);
  if(thing->sub == NULL) return;
  thing->sub->f(out, sample, thing->sub);


  asdr adsr = *((asdr *)thing->data);
  if(time < adsr.a){
    out[sample] *= time / adsr.a;
  }else if(time < (adsr.a + adsr.d)){
    // nothing;
    time = time - adsr.a;
    out[sample] *= (1 - (time / adsr.d) * 0.2);
  }else if(time < (adsr.a + adsr.d + adsr.s)){
    out[sample] *= 0.8;
  }else if(time < (adsr.a + adsr.d + adsr.s + adsr.r)){
    time = time - adsr.a - adsr.d - adsr.s;
    out[sample] *=0.8 * (1 - time / adsr.s);
  }else{
    out[sample] = 0;
  }
  
}

audiothing * create_adsr(float a, float d, float s, float r){
  audiothing * thing = calloc(sizeof(audiothing), 1);
  asdr * adsr = malloc(sizeof(asdr));
  adsr->a = a;
  adsr->d = d;
  adsr->s = s;
  adsr->r = r;
  thing->f = asdrfilter;
  thing->data = adsr;
  return thing;
}
