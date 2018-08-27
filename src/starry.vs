#version 100
attribute vec2 pos;
varying vec2 uv;
uniform vec2 offset;
void main() {  
  uv = pos.xy + offset;
  gl_Position = vec4(pos,0,1);
}

