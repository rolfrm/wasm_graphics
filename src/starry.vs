#version 100
attribute vec2 pos;
varying out vec2 uv;
uniform vec2 offset;
void main() {
  vec3 p = vec3(pos, 1);  
  uv = p.xy + offset;
  gl_Position = vec4(p,1);
}

