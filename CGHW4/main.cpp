#include <stdio.h>
#include <stdlib.h>
#include "iostream"
#include "string"
#include "vector"

#include <GL/glew.h>
#ifdef _WIN32
#include "freeglut/glut.h"
#elif __APPLE__
#include <GLUT/GLUT.h>
#endif
#include "textfile.h"
#include "glm.h"
#include "Mat.hpp"
#include "ObjWrapper.hpp"
#include "math.h"

#define FILENUM 3
#define PI 3.1415926
#define LIGHTNUM 3
#define DIRECTIONAL 1
#define POSITIONAL 2
#define SPOT 3
#define WINSIZE 800

LightSourceParameters dLight, pLight, sLight;

GLint iLocPosition;
GLint iLocNormal;
GLint iLocTexture;

GLint iLocMVP;
GLint iLocEye;
GLint iLocMDiffuse;
GLint iLocMAmbient;
GLint iLocMSpecular;
GLint iLocMShininess;

GLint iLocModelMat;
GLint iLocITModelMat;

GLint iLocPerpixelLighting;
GLint perpixelLighting = 1;

GLint iLocTextureMapping;
GLint textureMapping = 1;

GLint magFilterValue = GL_NEAREST;
GLint minFilterValue = GL_NEAREST;
GLenum wrapSValue = GL_REPEAT;
GLint wrapTValue = GL_REPEAT;

std::string files[FILENUM] = {"TextureModels/Zenigame.obj",
  "TextureModels/texturedknot.obj",
  "TextureModels/cube.obj"};

ObjWrapper OBJs[3];
ObjWrapper MultiOBJs[3];
int curModel = 0;
ObjWrapper* allOBJs = OBJs;
ObjWrapper* curOBJ;

void idle() {
	glutPostRedisplay();
}

void renderScene(void) {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(iLocPosition);
  glEnableVertexAttribArray(iLocNormal);
  glEnableVertexAttribArray(iLocTexture);
  
  Mat4 MVP = curOBJ->projectionMat * curOBJ->viewMat * curOBJ->modelMat;
  
  dLight.bind();
  pLight.bind();
  sLight.bind();
  Vec<4> eye(curOBJ->viewEye, 3);
  eye(3) = 1;
  glUniform4fv(iLocEye, 1, eye.data);
  glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, MVP.data);
  glUniformMatrix4fv(iLocModelMat, 1, GL_FALSE, curOBJ->modelMat.data);
  glUniformMatrix4fv(iLocITModelMat, 1, GL_FALSE,
                     curOBJ->modelMat.inverse().transpose().data);
  
  glUniform1i(iLocPerpixelLighting, perpixelLighting);
  glUniform1i(iLocTextureMapping, textureMapping);
  
  for (size_t i = 0; i < curOBJ->triangleGroups.size(); ++i) {
    glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0,
                          curOBJ->triangleGroups[i]->triangle_vertex);
    glVertexAttribPointer(iLocNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          curOBJ->triangleGroups[i]->triangle_normal);
    glVertexAttribPointer(iLocTexture, 2, GL_FLOAT, GL_FALSE, 0,
                          curOBJ->triangleGroups[i]->triangle_texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTValue);
    
    glBindTexture(GL_TEXTURE_2D, curOBJ->textureIDs[i]);
    
    glUniform4fv(iLocMDiffuse, 1, curOBJ->triangleGroups[i]->diffuse);
    glUniform4fv(iLocMAmbient, 1, curOBJ->triangleGroups[i]->ambient);
    glUniform4fv(iLocMSpecular, 1, curOBJ->triangleGroups[i]->specular);
    glUniform1f(iLocMShininess, curOBJ->triangleGroups[i]->shininess);
    
    glDrawArrays(GL_TRIANGLES, 0, curOBJ->triangleGroups[i]->numTriangles * 3);
  }
  
	glutSwapBuffers();
}

void showShaderCompileStatus(GLuint shader, GLint *shaderCompiled){
	glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
	if(GL_FALSE == (*shaderCompiled))
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar *errorLog = (GLchar*) malloc(sizeof(GLchar) * maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		fprintf(stderr, "%s", errorLog);

		glDeleteShader(shader);
		free(errorLog);
	}
}

void setShaders() {
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("CGHW4.vert");
	fs = textFileRead("CGHW4.frag");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	// compile vertex shader
	glCompileShader(v);
	GLint vShaderCompiled;
	showShaderCompileStatus(v, &vShaderCompiled);
	if(!vShaderCompiled) exit(123);

	// compile fragment shader
	glCompileShader(f);
	GLint fShaderCompiled;
	showShaderCompileStatus(f, &fShaderCompiled);
	if(!fShaderCompiled) exit(456);

	p = glCreateProgram();

	// bind shader
	glAttachShader(p, f);
	glAttachShader(p, v);

	// link program
	glLinkProgram(p);

  iLocNormal = glGetAttribLocation(p, "av3normal");
	iLocPosition = glGetAttribLocation (p, "av4position");
  iLocTexture = glGetAttribLocation(p, "av2texture");
  
	iLocMVP		 = glGetUniformLocation(p, "mvp");
  iLocEye = glGetUniformLocation(p, "eye");
  
  iLocMDiffuse = glGetUniformLocation(p, "material.diffuse");
  iLocMAmbient = glGetUniformLocation(p, "material.ambient");
  iLocMSpecular = glGetUniformLocation(p, "material.specular");
  iLocMShininess = glGetUniformLocation(p, "material.shininess");
  
  iLocModelMat = glGetUniformLocation(p, "modelMat");
  iLocITModelMat = glGetUniformLocation(p, "itModelMat");
  
  iLocPerpixelLighting = glGetUniformLocation(p, "perpixelLighting");
  iLocTextureMapping = glGetUniformLocation(p, "textureMapping");
  
  dLight.bind(p, "0");
  pLight.bind(p, "1");
  sLight.bind(p, "2");

	glUseProgram(p);
}

int mouseStartX = 0;
int mouseStartY = 0;
bool leftButtonDown = false;
bool rightButtonDown = false;

void processMouse(int who, int state, int x, int y) {
  switch (who) {
    case GLUT_LEFT_BUTTON:
      if (state == GLUT_DOWN) {
        mouseStartX = x;
        mouseStartY = y;
        leftButtonDown = true;
      } else if (state == GLUT_UP) {
        leftButtonDown = false;
      }
      break;
    case GLUT_RIGHT_BUTTON:
      if (state == GLUT_DOWN) {
        mouseStartX = x;
        mouseStartY = y;
        rightButtonDown = true;
      } else if (state == GLUT_UP) {
        rightButtonDown = false;
      }
      break;
    case GLUT_MIDDLE_BUTTON:
      std::cout << state;
      if (state == GLUT_UP) {
        float scale = 1.05;
        curOBJ->localGeoScale(scale, scale, scale);
      } else if (state == GLUT_DOWN) {
        float scale = 0.95;
        curOBJ->localGeoScale(scale, scale, scale);
      }
      break;
  }
}

void processMouseMotion(int x, int y) {
  if (leftButtonDown) {
    float transX = (x - mouseStartX) / (float)WINSIZE * 2;
    float transY = -(y - mouseStartY) / (float)WINSIZE * 2;
    mouseStartX = x;
    mouseStartY = y;
    curOBJ->geoTrans(transX, transY, 0);
  } else if (rightButtonDown) {
    float rotateX = (x - mouseStartX) / (float)WINSIZE * 2 * PI;
    float rotateY = (y - mouseStartY) / (float)WINSIZE * 2 * PI;
    mouseStartX = x;
    mouseStartY = y;
    curOBJ->localGeoRotate(rotateY, rotateX, 0);
  }
}

void processNormalKeys(unsigned char key, int x, int y) {
  float scale;
  switch(key) {
    case 27: /* the Esc key */
      exit(0);
      break;
    case 'h':
      std::cout << "ESC -- exit\nb -- last model\nn -- next model\np -- toggle projection mode\ns/S -- scale down/up\nl -- toggle perpixel lighting\nt -- toggle texture mapping\nm -- toggle min filter between linear and nearest\nM -- toggle mag filter between linear and nearest\nw -- toggle wrap_s between clamp and repeat\nW -- toggle wrap_t between clamp and repeat\n1 -- toggle directional light\n2 -- toggle positional light\n3 -- toggle spot light\nPress and drag left button to do translation\nPress and drag right button to do rotation" << std::endl;;
      break;
    case 'b':
      curModel = (curModel - 1 + FILENUM) % FILENUM;
      curOBJ = &allOBJs[curModel];
      std::cout << "current model : " << files[curModel] << std::endl;
      break;
    case 'n':
      curModel = (curModel + 1 + FILENUM) % FILENUM;
      curOBJ = &allOBJs[curModel];
      std::cout << "current model : " << files[curModel] << std::endl;
      break;
    case 'p':
      curOBJ->toggleProjection();
      break;
    case 's':
      scale = 0.95;
      curOBJ->localGeoScale(scale, scale, scale);
      break;
    case 'S':
      scale = 1.05;
      curOBJ->localGeoScale(scale, scale, scale);
      break;
    case 'l':
      perpixelLighting = (perpixelLighting + 1) % 2;
      break;
    case 't':
      textureMapping = (textureMapping  + 1) % 2;
      break;
    case 'm':
      if (minFilterValue == GL_LINEAR) {
        minFilterValue = GL_NEAREST;
        std::cout << "min filter : GL_LINEAR" << std::endl;
      } else {
        minFilterValue = GL_LINEAR;
        std::cout << "min filter : GL_NEAREST" << std::endl;
      }
      break;
    case 'M':
      if (magFilterValue == GL_NEAREST) {
        magFilterValue = GL_LINEAR;
        std::cout << "mag filter : GL_LINEAR" << std::endl;
      } else {
        magFilterValue = GL_NEAREST;
        std::cout << "mag filter : GL_NEAREST" << std::endl;
      }
      break;
    case 'w':
      if (wrapSValue == GL_REPEAT) {
        wrapSValue = GL_CLAMP;
        std::cout << "wrap_s : GL_CLAMP" << std::endl;
      } else {
        wrapSValue = GL_REPEAT;
        std::cout << "wrap_s : GL_REPEAT" << std::endl;
      }
      break;
    case 'W':
      if (wrapTValue == GL_CLAMP) {
        wrapTValue = GL_REPEAT;
        std::cout << "wrap_t : GL_REPEAT" << std::endl;
      } else {
        wrapTValue = GL_CLAMP;
        std::cout << "wrap_t : GL_CLAMP" << std::endl;
      }
      break;
    case '1':
      dLight.on = (dLight.on + 1) % 2;
      break;
    case '2':
      pLight.on = (pLight.on + 1) % 2;
      break;
    case '3':
      sLight.on = (sLight.on + 1) % 2;
      break;
  }
}

int main(int argc, char **argv) {
  
  dLight.type = DIRECTIONAL;
  pLight.type = POSITIONAL;
  sLight.type = SPOT;
  
  dLight.diffuse = Vec<4>(0.5, 0.5, 0.5, 1);
  dLight.ambient = Vec<4>(0.5, 0.5, 0.5, 1);
  dLight.specular = Vec<4>(1, 1, 1, 1);
  dLight.dLightDirection = Vec<3>(0, 0, -2);
  
  pLight.diffuse = Vec<4>(0.5, 0.5, 0.5, 1);
  pLight.ambient = Vec<4>(0.5, 0.5, 0.5, 1);
  pLight.specular = Vec<4>(1, 1, 1, 1);
  pLight.spLightPosition = Vec<4>(0, 0, 4, 1);
  
  sLight.diffuse = Vec<4>(0.5, 0.5, 0.5, 1);
  sLight.ambient = Vec<4>(0.5, 0.5, 0.5, 1);
  sLight.specular = Vec<4>(1, 1, 1, 1);
  sLight.spLightPosition = Vec<4>(0, 0, 2, 1);
  sLight.sLightDirection = Vec<3>(0, 0, -1);
  sLight.sLightExp = 1;
  sLight.sLightCosCutoff = 0.95;
  
  dLight.on = 0;
  pLight.on = 1;
  sLight.on = 0;
  
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowPosition(460, 40);
	glutInitWindowSize(WINSIZE, WINSIZE);
	int winID = glutCreateWindow("10320 CS550000 CG HW4 Shangtong Zhang");

	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	glutDisplayFunc (renderScene);
	glutIdleFunc    (idle);
	glutKeyboardFunc(processNormalKeys);
	glutMouseFunc   (processMouse);
	glutMotionFunc  (processMouseMotion);

  for (size_t i = 0; i < FILENUM; ++i) {
    OBJs[i].loadObj(files[i]);
  }
  
  curOBJ = &allOBJs[curModel];
  
	glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

	setShaders();

	glutMainLoop();

	return 0;
}

