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

attribute vec2 av2texture;
attribute vec4 av4position;
attribute vec3 av3normal;

uniform mat4 mvp;
uniform vec4 eye;
uniform mat4 modelMat;
uniform mat4 itModelMat;
uniform MaterialParameters material;
uniform LightSourceParameters lights[LIGHTNUM];

varying vec4 vv4color;
varying vec4 vv4normal;
varying vec2 vv2texture;
varying vec4 vv4position;

void main() {
  
  vv4position = av4position;

	vec4 color = vec4(0, 0, 0, 0);
  
  vec4 av4positionM = modelMat * av4position;
  av4positionM /= av4positionM.w;
  
  vec4 av4normal = vec4(av3normal, 0);
  vv4normal = itModelMat * av4normal;
  vv4normal = normalize(vv4normal);
  
  vec4 viewVec = eye - av4positionM;
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
      lineToLight = lights[i].spLightPosition.xyz - av4positionM.xyz;
      lineToLight = normalize(lineToLight);
      halfVector = lineToLight + viewVec.xyz;
      spotEffect = 1.0;
    } else if (lights[i].type == SPOT) {
      lineToLight = lights[i].spLightPosition.xyz - av4positionM.xyz;
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
    color += spotEffect * (material.ambient * lights[i].ambient +
        max(dot(lineToLight4, vv4normal), 0.0) * material.diffuse * lights[i].diffuse +
        pow(max(dot(halfVector4, vv4normal), 0.0), 6.0) * material.specular
        * lights[i].specular);
  }
  
	vv4color = color;
  
  vv2texture = av2texture;
	
	gl_Position = mvp * av4position;
}

