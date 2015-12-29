#define LIGHTNUM 3
#define DIRECTIONAL 1
#define POSITIONAL 2
#define SPOT 3

uniform sampler2D ustexture;

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

uniform mat4 mvp;
uniform vec4 eye;
uniform mat4 modelMat;
uniform mat4 itModelMat;
uniform MaterialParameters material;
uniform LightSourceParameters lights[LIGHTNUM];

uniform int perpixelLighting;
uniform int textureMapping;

varying vec4 vv4color;

varying vec4 vv4position;
varying vec4 vv4normal;
varying vec2 vv2texture;

void main() {
  
  vec4 texColor = texture2D(ustexture, vv2texture);
  
  vec4 pixelColor = vec4(0, 0, 0, 0);
  
  vec4 vv4positionM = modelMat * vv4position;
  vv4positionM /= vv4positionM.w;
  
  vec4 vv4normalN = normalize(vv4normal);
  
  vec4 viewVec = eye - vv4positionM;
  viewVec = normalize(viewVec);
  
  for (int i = 0; i < LIGHTNUM; ++i) {
    if (lights[i].on == 0) {
      continue;
    }
    vec3 halfVector;
    vec3 lineToLight;
    float spotEffect;
    
    if (lights[i].type == DIRECTIONAL){
      lineToLight = - lights[i].dLightDirection;
      halfVector = lineToLight + viewVec.xyz;
      spotEffect = 1.0;
    } else if (lights[i].type == POSITIONAL) {
      lineToLight = lights[i].spLightPosition.xyz - vv4positionM.xyz;
      lineToLight = normalize(lineToLight);
      halfVector = lineToLight + viewVec.xyz;
      spotEffect = 1.0;
    } else if (lights[i].type == SPOT) {
      lineToLight = lights[i].spLightPosition.xyz - vv4positionM.xyz;
      lineToLight = normalize(lineToLight);
      halfVector = lineToLight + viewVec.xyz;
      
      vec3 lineToVex = - lineToLight;
      lineToVex = normalize(lineToVex);
      vec3 spotlightDirection = normalize(lights[i].sLightDirection);
      float cosv = dot(lineToVex, spotlightDirection);
      if (cosv > lights[i].sLightCosCutoff) {
        spotEffect = pow(max(dot(lineToVex, spotlightDirection), 0.0), lights[i].sLightExp);
      } else {
        spotEffect = 0.0;
      }
    }
    
    halfVector = normalize(halfVector);
    lineToLight = normalize(lineToLight);
    vec4 halfVector4 = vec4(halfVector, 0);
    vec4 lineToLight4 = vec4(lineToLight, 0);
    pixelColor += spotEffect * (material.ambient * lights[i].ambient +
                           max(dot(lineToLight4, vv4normalN), 0.0) * material.diffuse * lights[i].diffuse +
                           pow(max(dot(halfVector4, vv4normalN), 0.0), 6.0) * material.specular
                           * lights[i].specular);
  }
  
  if (textureMapping == 1) {
    if (perpixelLighting == 1) {
//      gl_FragColor = 0.5 * pixelColor + 0.5 * texColor;
      gl_FragColor = pixelColor * texColor;
    } else {
//      gl_FragColor = 0.5 * vv4color + 0.5 * texColor;
      gl_FragColor = vv4color * texColor;
    }
  } else {
    gl_FragColor = perpixelLighting == 1 ? pixelColor : vv4color;
  }
  
}
