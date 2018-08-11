#version 100
attribute vec2 pos;
uniform mat3 tform;

void main() {
  vec3 p = vec3(pos, 1);  
  p = tform * p;
  gl_Position = vec4(p,1);
}

