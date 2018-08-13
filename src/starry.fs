#version 100
precision lowp float;
//uniform vec4 color;
varying vec2 uv;
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}
// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(0.01, 0.0));
    float c = random(i + vec2(0.0, 0.01));
    float d = random(i + vec2(0.01, 0.01));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

void main(){
  float n = random(floor(uv * 200.0));
  float n2 = random(floor(uv * 200.0) + 10.0) * 2.0;
  n = pow(n, 4.0);
  n = n * 80.0 - 79.0;
  if(n < 0.0) discard;
  vec3 col = vec3(n2);
  if(n2>0.7){
     col.r *= 0.5;
     col.g *= 0.7;
  }else if(n2 > 0.5){
     col.b *= 0.5;
     col.g *= 0.7;
  
  } 
  gl_FragColor = vec4(col * 1.0, 1);;
  
}
