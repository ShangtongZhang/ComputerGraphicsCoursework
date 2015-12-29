uniform sampler2D image;
varying vec2 vv2texture;
varying vec4 vv4color;
uniform float shaderParam;

float gray(vec4 c){
  return clamp(dot(c.rgb, vec3(0.3, 0.59, 0.11)), 0.0, 1.0);
}

void main()
{
  int coloringMethod = 3;
  vec2 tex = vv2texture;
  vec3 colSg = vec3(0.078,0.396,0.671);
  vec3 colTint = vec3(0.078,0.396,0.671);
  vec3 colLo = vec3(0.314,0.314,0.569);
  vec3 colHi = vec3(0.812,0.812,0.569);
  
  vec4 color = texture2D(image, tex);
  float grey = gray(color);
  
  if (shaderParam == 0.0){
    color.rgb = colSg;
  }else{
    if (shaderParam == 1.0){
      color.rgb = mix(vec3(0,0,0), colTint, grey);
    }else{
      if (shaderParam == 2.0){
        color.rgb = mix(colLo, colHi, grey);
      }
    }
  }
  gl_FragColor = color * vv4color;
}
