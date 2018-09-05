#version 100
attribute vec2 pos;
uniform mat3 tform;
varying vec2 offpos;
void main() {
  vec3 p = vec3(pos, 1);  
  p = tform * p;
  vec3 p2 = p - tform * vec3(0,0,1);
  offpos = (p2).xy;
  gl_Position = vec4(p,1);

}

