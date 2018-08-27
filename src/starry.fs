#version 100
precision highp float;
varying vec2 uv;
uniform float scale;

#define HASHSCALE1 .1031
#define HASHSCALE3 vec3(.1031, .1030, .0973)
#define HASHSCALE4 vec4(.1031, .1030, .0973, .1099)

//  1 out, 2 in...
float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233) + 100.0)) * 43758.5453);
}

void main(){
  float n = hash12(floor(uv / scale * 150.0));
  
  if(n < 0.999) discard;
  float n2 = rand(floor(uv / scale * 150.0) + vec2(10.0, 30.0) );
  n2 = pow(n2, 0.5);
  vec3 col = vec3(n2);
  if(n2>0.7){  
    col.r *= 0.5;
    col.g *= 0.7;
  }else if(n2 > 0.5){
    col.b *= 0.5;
    col.g *= 0.7;
  }
  gl_FragColor = vec4(col, 1.0);
  
    
}
