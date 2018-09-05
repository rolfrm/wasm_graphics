#version 100
precision lowp float;
uniform vec4 color;
uniform vec4 color2;

varying vec2 offpos;
void main(){
  if(mod((offpos.x * (mod(offpos.y * 20.0, 2.0) > 1.0 ? -1.0 : 1.0) + offpos.y) * 20.0,2.0) > 1.0){
     gl_FragColor = color;
   }else{
     gl_FragColor = color2;
  }
}
