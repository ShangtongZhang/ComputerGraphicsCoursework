#define LIGHTNUM 3
#define DIRECTIONAL 1
#define POSITIONAL 2
#define SPOT 3

struct LightSourceParameters {
  int on;
  int type;
  
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float specular_shininess;
  
  vec3 dLightDirection;
  
  vec4 spLightPosition;
  
  vec3 sLightDirection;
  float sLightExp;
  float sLightCosCutoff;
  
};

struct MaterialParameters {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

varying vec4 vv4normal;
varying vec2 vv2texture;
uniform sampler2D tex0;
uniform float shaderParam;
uniform LightSourceParameters lights[LIGHTNUM];

void main()
{
  vec4 color1 = texture2D(tex0, vv2texture);
  vec4 color2;
  
  float intensity = dot(normalize(lights[POSITIONAL].spLightPosition),vv4normal);
  
  if (intensity > 0.95)      color2 = vec4(1.0, 1.0, 1.0, 1.0);
  else if (intensity > 0.75) color2 = vec4(0.8, 0.8, 0.8, 1.0);
  else if (intensity > 0.50) color2 = vec4(0.6, 0.6, 0.6, 1.0);
  else if (intensity > 0.25) color2 = vec4(0.4, 0.4, 0.4, 1.0);
  else                       color2 = vec4(0.2, 0.2, 0.2, 1.0);
  
  gl_FragColor = color1 * color2;
}