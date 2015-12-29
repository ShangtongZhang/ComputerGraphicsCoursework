#include <stdio.h>
#include <stdlib.h>
#include "iostream"
#include "string"
#include "vector"

#include <GL/glew.h>
#include <GLUT/GLUT.h>
#include "textfile.h"
#include "glm.h"
#include "Mat4.hpp"
#include "math.h"

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

#define FILENUM 3
#define INF (1<<30)
#define NORMALMODE 0
#define GEOMODE 1
#define VIEWMODE 2
#define PROJECTIONMODE 3
#define MULTIOBJSMODE 4
#define PI 3.1415926
#define STEP 0.2

// Shader attributes
GLint iLocPosition;
GLint iLocColor;
GLint iLocMVP;

std::string files[FILENUM] = {"ColorModels/teapot4KC.obj",
  "ColorModels/tigerC.obj",
  "ColorModels/elephant16KC.obj"};

class ObjWrapper {
  
public:
  GLMmodel* OBJ;
  GLfloat* triangle_vertex;
  GLfloat* triangle_color;
  Mat4 modelMat;
  Mat4 viewMat;
  Mat4 projectionMat;
  GLfloat viewTarget[3] = {0, 0, -4};
  GLfloat viewEye[3] = {0, 0, 2};
  GLfloat viewUp[3] = {0, 1, 0};
  GLfloat geoParamScale[3] = {1, 1, 1};
  GLfloat geoParamTrans[3] = {0, 0, 0};
  GLfloat geoParamRotate[3] = {0, 0, 0};
  GLfloat left = -1, right = 1, bottom = -1, top = 1, near = 1, far = 5;
  bool isOrthographic = true;
  
  void init() {
    viewTarget[0] = viewTarget[1] = viewEye[0] = viewEye[1] = viewUp[0] = viewUp[2] = 0;
    viewTarget[2] = -4;
    viewEye[2] = 2;
    viewUp[1] = 1;
    for (size_t i = 0; i < 3; ++i) {
      geoParamScale[i] = 1;
      geoParamTrans[i] = geoParamRotate[i] = 0;
    }
    left = -1;
    right = 1;
    bottom = -1;
    top = 1;
    near = 1;
    far = 5;
    isOrthographic = true;
  }
  
  ObjWrapper() {
    init();
    OBJ = NULL;
    triangle_vertex = NULL;
    triangle_color = NULL;
    modelMat.identity();
    viewTransMat();
    orthographicMat();
//    perspectiveMat();
  }
  
  ~ObjWrapper() {
    if (OBJ) {
      glmDelete(OBJ);
    }
    if (triangle_color) {
      delete[] triangle_color;
    }
    if (triangle_vertex) {
      delete[] triangle_vertex;
    }
  }
  
  void loadGeoParamScale(GLfloat* p) {
    for (size_t i = 0; i < 3; ++i) {
      geoParamScale[i] = p[i];
    }
  }
  void loadGeoParamTrans(GLfloat* p) {
    for (size_t i = 0; i < 3; ++i) {
      geoParamTrans[i] = p[i];
    }
  }
  void loadGeoParamRotate(GLfloat* p) {
    for (size_t i = 0; i < 3; ++i) {
      geoParamRotate[i] = p[i];
    }
  }
  
  void toggleProjection() {
    if (isOrthographic) {
      perspectiveMat();
    } else {
      orthographicMat();
    }
    isOrthographic = !isOrthographic;
  }
  
  void updateObjCenter() {
    GLfloat maxv[3] = {-INF, -INF, -INF};
    GLfloat minv[3] = {INF, INF, INF};
    
    for (size_t i = 0; i < OBJ->numtriangles; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        for (size_t k = 0; k < 3; ++k) {
          maxv[k] = max(OBJ->vertices[3 * OBJ->triangles[i].vindices[j] + k], maxv[k]);
          minv[k] = min(OBJ->vertices[3 * OBJ->triangles[i].vindices[j] + k], minv[k]);
        }
      }
    }
    
    for (size_t i = 0; i < 3; ++i) {
      OBJ->position[i] = (maxv[i] + minv[i]) / 2;
    }
    
    GLfloat currentCenter[4] = {0, 0, 0, 0};
    for (size_t i = 0; i < 4; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        currentCenter[i] += modelMat(i, j) * OBJ->position[j];
      }
      currentCenter[i] += modelMat(i, 3) * 1;
    }
    
    for (size_t i = 0; i < 3; ++i) {
      OBJ->position[i] = currentCenter[i] / currentCenter[3];
    }
    
  }
  
  void normalizeModel() {
    GLfloat maxv[3] = {-INF, -INF, -INF};
    GLfloat minv[3] = {INF, INF, INF};
    GLfloat stride[3] = {0, 0, 0};
    
    for (size_t i = 0; i < OBJ->numtriangles; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        for (size_t k = 0; k < 3; ++k) {
          maxv[k] = max(OBJ->vertices[3 * OBJ->triangles[i].vindices[j] + k], maxv[k]);
          minv[k] = min(OBJ->vertices[3 * OBJ->triangles[i].vindices[j] + k], minv[k]);
        }
      }
    }
    
    for (size_t i = 0; i < 3; ++i) {
      OBJ->position[i] = (maxv[i] + minv[i]) / 2;
    }
    
    for (size_t i = 0; i < 3; ++i) {
      stride[i] = maxv[i] - minv[i];
    }
    
    int major = stride[0] > stride[1] ? 0 : 1;
    major = stride[major] > stride[2] ? major : 2;
    GLfloat majorStride = stride[major];
    
    updateObjCenter();
    geoTrans(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
    geoScale(2 / majorStride, 2 / majorStride, 2 / majorStride);
    
  }
  
  void loadObj(const std::string& file) {
    if (OBJ) {
      glmDelete(OBJ);
    }
    if (triangle_color) {
      delete[] triangle_color;
    }
    if (triangle_vertex) {
      delete[] triangle_vertex;
    }
    
    OBJ = glmReadOBJ(file.data());
    
    normalizeModel();
    
    triangle_vertex = new GLfloat[OBJ->numtriangles * 9];
    triangle_color = new GLfloat[OBJ->numtriangles * 9];
    for (size_t i = 0; i < OBJ->numtriangles; ++i) {
      
      int indv1 = OBJ->triangles[i].vindices[0];
      int indv2 = OBJ->triangles[i].vindices[1];
      int indv3 = OBJ->triangles[i].vindices[2];
      
      int indc1 = indv1;
      int indc2 = indv2;
      int indc3 = indv3;
      
      triangle_vertex[9 * i + 0] = OBJ->vertices[indv1 * 3 + 0];
      triangle_vertex[9 * i + 1] = OBJ->vertices[indv1 * 3 + 1];
      triangle_vertex[9 * i + 2] = OBJ->vertices[indv1 * 3 + 2];
      triangle_vertex[9 * i + 3] = OBJ->vertices[indv2 * 3 + 0];
      triangle_vertex[9 * i + 4] = OBJ->vertices[indv2 * 3 + 1];
      triangle_vertex[9 * i + 5] = OBJ->vertices[indv2 * 3 + 2];
      triangle_vertex[9 * i + 6] = OBJ->vertices[indv3 * 3 + 0];
      triangle_vertex[9 * i + 7] = OBJ->vertices[indv3 * 3 + 1];
      triangle_vertex[9 * i + 8] = OBJ->vertices[indv3 * 3 + 2];
      
      triangle_color[9 * i + 0] = OBJ->colors[indc1 * 3 + 0];
      triangle_color[9 * i + 1] = OBJ->colors[indc1 * 3 + 1];
      triangle_color[9 * i + 2] = OBJ->colors[indc1 * 3 + 2];
      triangle_color[9 * i + 3] = OBJ->colors[indc2 * 3 + 0];
      triangle_color[9 * i + 4] = OBJ->colors[indc2 * 3 + 1];
      triangle_color[9 * i + 5] = OBJ->colors[indc2 * 3 + 2];
      triangle_color[9 * i + 6] = OBJ->colors[indc3 * 3 + 0];
      triangle_color[9 * i + 7] = OBJ->colors[indc3 * 3 + 1];
      triangle_color[9 * i + 8] = OBJ->colors[indc3 * 3 + 2];
      
    }
  }
  
  void geoScale(GLfloat p1, GLfloat p2, GLfloat p3) {
    Mat4 m;
    m(0, 0) = p1;
    m(1, 1) = p2;
    m(2, 2) = p3;
    m(3, 3) = 1;
    modelMat = m * modelMat;
  }
  
  void geoTrans(GLfloat p1, GLfloat p2, GLfloat p3) {
    Mat4 m;
    m(0, 0) = 1;
    m(1, 1) = 1;
    m(2, 2) = 1;
    m(3, 3) = 1;
    m(0, 3) = p1;
    m(1, 3) = p2;
    m(2, 3) = p3;
    modelMat = m * modelMat;
  }
  
  void geoTrans() {
    geoTrans(geoParamTrans[0], geoParamTrans[1], geoParamTrans[2]);
  }
  
  void geoRotate(GLfloat p1, GLfloat p2, GLfloat p3) {
    Mat4 m1;
    m1(0, 0) = m1(3, 3) = 1;
    m1(1, 1) = m1(2, 2) = cos(p1);
    m1(1, 2) = -sin(p1);
    m1(2, 1) = sin(p1);
    Mat4 m2;
    m2(1, 1) = m2(3, 3) = 1;
    m2(0, 0) = m2(2, 2) = cos(p2);
    m2(0, 2) = sin(p2);
    m2(2, 0) = -sin(p2);
    Mat4 m3;
    m3(2, 2) = m3(3, 3) = 1;
    m3(0, 0) = m3(1, 1) = cos(p3);
    m3(0, 1) = -sin(p3);
    m3(1, 0) = sin(p3);
    modelMat = m3 * m2 * m1 * modelMat;
  }
  
  void localGeoScale(GLfloat p1, GLfloat p2, GLfloat p3) {
    updateObjCenter();
    geoTrans(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
    geoScale(p1, p2, p3);
    geoTrans(OBJ->position[0], OBJ->position[1], OBJ->position[2]);
//    return geoTrans(OBJ->position[0], OBJ->position[1], OBJ->position[2]) *
//    geoScale(p1, p2, p3) *
//    geoTrans(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
  }
  
  void localGeoScale() {
    localGeoScale(geoParamScale[0], geoParamScale[1], geoParamScale[2]);
  }
  
  void localGeoRotate(GLfloat p1, GLfloat p2, GLfloat p3) {
    updateObjCenter();
    geoTrans(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
    geoRotate(p1, p2, p3);
    geoTrans(OBJ->position[0], OBJ->position[1], OBJ->position[2]);
//    return geoTrans(OBJ->position[0], OBJ->position[1], OBJ->position[2]) *
//    geoRotate(p1, p2, p3) *
//    geoTrans(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
  }
  
  void localGeoRotate() {
    localGeoRotate(geoParamRotate[0], geoParamRotate[1], geoParamRotate[2]);
  }
  
  void viewTransMat() {
    GLfloat RZ[3];
    for (size_t i = 0; i < 3; ++i) {
      RZ[i] = viewEye[i] - viewTarget[i];
    }
    glmNormalize(RZ);
    
    GLfloat RX[3];
    GLfloat UP[3];
//    for (size_t i = 0; i < 3; ++i) {
//      UP[i] = viewUp[i] - viewEye[i];
//    }
    glmCross(viewUp, RZ, RX);
    glmNormalize(RX);
    
    GLfloat RY[3];
    glmCross(RZ, RX, RY);
    
    Mat4 R;
    for (size_t i = 0; i < 3; ++i) {
      R(0, i) = RX[i];
    }
    for (size_t i = 0; i < 3; ++i) {
      R(1, i) = RY[i];
    }
    for (size_t i = 0; i < 3; ++i) {
      R(2, i) = RZ[i];
    }
    R(3, 3) = 1;
    Mat4 T;
    T.identity();
    for (size_t i = 0; i < 3; ++i) {
      T(i, 3) = -viewEye[i];
    }
    viewMat = R * T;
  }
  
  void orthographicMat() {
    projectionMat.zero();
    projectionMat(0, 0) = 2 / (right - left);
    projectionMat(1, 1) = 2 / (top - bottom);
    projectionMat(2, 2) = -2 / (far - near);
    projectionMat(3, 3) = 1;
    projectionMat(0, 3) = - (right + left) / (right - left);
    projectionMat(1, 3) = - (top + bottom) / (top - bottom);
    projectionMat(2, 3) = - (far + near) / (far - near);
  }
  
  void perspectiveMat() {
    projectionMat.zero();
    projectionMat(0, 0) = 2 * near / (right - left);
    projectionMat(1, 1) = 2 * near / (top - bottom);
    projectionMat(3, 2) = -1;
    projectionMat(0, 2) = (right + left) / (right - left);
    projectionMat(1, 2) = (top + bottom) / (top - bottom);
    projectionMat(2, 2) = -(far + near) / (far - near);
    projectionMat(2, 3) = -2 * far * near / (far - near);
  }
};

ObjWrapper OBJs[3];
ObjWrapper MultiOBJs[3];
int curModel = 0;
ObjWrapper* allOBJs = OBJs;
ObjWrapper* curOBJ;
int mode = NORMALMODE;
bool isMultiOBJsMode = false;

GLfloat* viewParam[3];
GLfloat* geoParam[3];
std::string viewParamName[] = {"eye", "center", "up vector"};
std::string geoParamName[] = {"scale", "translation", "rotate"};
int viewLastParamInd = 0;
int geoLastParamInd = 0;

void changeOBJ() {
  curOBJ = &allOBJs[curModel];
  
  viewParam[0] = curOBJ->viewEye;
  viewParam[1] = curOBJ->viewTarget;
  viewParam[2] = curOBJ->viewUp;
  viewLastParamInd = 0;
  
  geoParam[0] = curOBJ->geoParamScale;
  geoParam[1] = curOBJ->geoParamTrans;
  geoParam[2] = curOBJ->geoParamRotate;
  geoLastParamInd = 0;
}

void idle() {
	glutPostRedisplay();
}

void renderScene(void) {

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocColor);
  
  if (isMultiOBJsMode) {
    for (size_t i = 0; i < FILENUM; ++i) {
      ObjWrapper* curOBJ = &MultiOBJs[i];
      Mat4 MVP = curOBJ->projectionMat * curOBJ->viewMat * curOBJ->modelMat;
      glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, curOBJ->triangle_vertex);
      glVertexAttribPointer(   iLocColor, 3, GL_FLOAT, GL_FALSE, 0, curOBJ->triangle_color);
      glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, MVP.data);
      glDrawArrays(GL_TRIANGLES, 0, curOBJ->OBJ->numtriangles * 3);
    }
    
  } else {
    Mat4 MVP = curOBJ->projectionMat * curOBJ->viewMat * curOBJ->modelMat;
    glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, curOBJ->triangle_vertex);
    glVertexAttribPointer(   iLocColor, 3, GL_FLOAT, GL_FALSE, 0, curOBJ->triangle_color);
    glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, MVP.data);
    glDrawArrays(GL_TRIANGLES, 0, curOBJ->OBJ->numtriangles * 3);
  }

	glutSwapBuffers();
}

void showShaderCompileStatus(GLuint shader, GLint *shaderCompiled){
	glGetShaderiv(shader, GL_COMPILE_STATUS, shaderCompiled);
	if(GL_FALSE == (*shaderCompiled))
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character.
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

	vs = textFileRead("CGHW2.vert");
	fs = textFileRead("CGHW2.frag");

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

	iLocPosition = glGetAttribLocation (p, "av4position");
	iLocColor    = glGetAttribLocation (p, "av3color");
	iLocMVP		 = glGetUniformLocation(p, "mvp");

	glUseProgram(p);
}

void processMouse(int who, int state, int x, int y) {
//	printf("(%d, %d) ", x, y);
//
//	switch(who){
//	case GLUT_LEFT_BUTTON:   printf("left button   "); break;
//	case GLUT_MIDDLE_BUTTON: printf("middle button "); break;
//	case GLUT_RIGHT_BUTTON:  printf("right button  "); break; 
//	case GLUT_WHEEL_UP:      printf("wheel up      "); break;
//	case GLUT_WHEEL_DOWN:    printf("wheel down    "); break;
//	default:                 printf("%-14d", who);     break;
//	}
//
//	switch(state){
//	case GLUT_DOWN:          printf("start ");         break;
//	case GLUT_UP:            printf("end   ");         break;
//	}
//
//	printf("\n");
}

void processMouseMotion(int x, int y) {  // callback on mouse drag
//	printf("(%d, %d) mouse move\n", x, y);
}

void doGeoTransformation() {
  if (geoLastParamInd == 0) {
    curOBJ->localGeoScale();
    curOBJ->geoParamScale[0] = curOBJ->geoParamScale[1] = curOBJ->geoParamScale[2] = 1;
  } else if (geoLastParamInd == 1) {
    curOBJ->geoTrans();
    curOBJ->geoParamTrans[0] = curOBJ->geoParamTrans[1] = curOBJ->geoParamTrans[2] = 0;
  } else if (geoLastParamInd == 2) {
    curOBJ->localGeoRotate();
    curOBJ->geoParamRotate[0] = curOBJ->geoParamRotate[1] = curOBJ->geoParamRotate[2] = 0;
  }
}

void handleGeoMode(unsigned char key) {
  GLfloat p[3];
  GLfloat step = 0;
  if (geoLastParamInd == 0) {
    step = (key == 'x' || key == 'y' || key == 'z') ? 1.1 : -0.9;
  } else if (geoLastParamInd == 1) {
    step = 0.2;
  } else if (geoLastParamInd == 2) {
    step = PI * 10 / 180;
  }
  switch (key) {
    case 27:
      mode = NORMALMODE;
      std::cout << "exit geometrical transformation\nnormal mode" << std::endl;
      break;
    case 's':
      std::cout << "scale:" << std::endl;
      std::cin >> p[0] >> p[1] >> p[2];
      curOBJ->loadGeoParamScale(p);
      curOBJ->localGeoScale(p[0], p[1], p[2]);
      break;
    case 't':
      std::cout << "translation:" << std::endl;
      std::cin >> p[0] >> p[1] >> p[2];
      curOBJ->loadGeoParamTrans(p);
      curOBJ->geoTrans(p[0], p[1], p[2]);
      break;
    case 'r':
      std::cout << "rotate:" << std::endl;
      std::cin >> p[0] >> p[1] >> p[2];
      p[0] = PI * p[0] / 180;
      p[1] = PI * p[1] / 180;
      p[2] = PI * p[2] / 180;
      curOBJ->loadGeoParamRotate(p);
      curOBJ->localGeoRotate(p[0], p[1], p[2]);
      break;
    case 'x':
      geoParam[geoLastParamInd][0] = step;
      doGeoTransformation();
      break;
    case 'X':
      geoParam[geoLastParamInd][0] = -step;
      doGeoTransformation();
      break;
    case 'y':
      geoParam[geoLastParamInd][1] = step;
      doGeoTransformation();
      break;
    case 'Y':
      geoParam[geoLastParamInd][1] = -step;
      doGeoTransformation();
      break;
    case 'z':
      geoParam[geoLastParamInd][2] = step;
      doGeoTransformation();
      break;
    case 'Z':
      geoParam[geoLastParamInd][2] = -step;
      doGeoTransformation();
      break;
    case 'S':
      geoLastParamInd = (geoLastParamInd + 1) % 3;
      std::cout << "current param : " << geoParamName[geoLastParamInd] << std::endl;
      break;
  }
}

void handleViewMode(unsigned char key) {
  switch (key) {
    case 27:
      mode = NORMALMODE;
      std::cout << "exit view transformation\nnormal mode" << std::endl;
      break;
    case 'c':
      std::cout << "view center:" << std::endl;
      std::cin >> curOBJ->viewTarget[0] >> curOBJ->viewTarget[1] >> curOBJ->viewTarget[2];
      viewLastParamInd = 1;
      break;
    case 'e':
      std::cout << "eye:" << std::endl;
      std::cin >> curOBJ->viewEye[0] >> curOBJ->viewEye[1] >> curOBJ->viewEye[2];
      viewLastParamInd = 0;
      break;
    case 'u':
      std::cout << "up vector:" << std::endl;
      std::cin >> curOBJ->viewUp[0] >> curOBJ->viewUp[1] >> curOBJ->viewUp[2];
      curOBJ->viewUp[2] = 0;
      viewLastParamInd = 2;
      break;
    case 'x':
      viewParam[viewLastParamInd][0] += STEP;
      break;
    case 'X':
      viewParam[viewLastParamInd][0] -= STEP;
      break;
    case 'y':
      viewParam[viewLastParamInd][1] += STEP;
      break;
    case 'Y':
      viewParam[viewLastParamInd][1] -= STEP;
      break;
    case 'z':
      viewParam[viewLastParamInd][2] += STEP;
      curOBJ->viewUp[2] = 0;
      break;
    case 'Z':
      viewParam[viewLastParamInd][2] -= STEP;
      curOBJ->viewUp[2] = 0;
      break;
    case 'S':
      viewLastParamInd = (viewLastParamInd + 1) % 3;
      std::cout << "current param : " << viewParamName[viewLastParamInd] << std::endl;
      break;
  }
  curOBJ->viewTransMat();
}

void handleNormalMode(unsigned char key) {
  switch(key) {
		case 27: /* the Esc key */
			exit(0);
			break;
    case 'h':
      std::cout << "ESC -- exit\nb -- back\nn -- next\ng -- geometrical transformation mode\nv -- view transformation mode\np -- projection mode\nm -- toggle single/multi obj mode" << std::endl;;
      break;
    case 'b':
      curModel = (curModel - 1 + FILENUM) % FILENUM;
      changeOBJ();
      std::cout << "current model : " << files[curModel] << std::endl;
      break;
    case 'n':
      curModel = (curModel + 1 + FILENUM) % FILENUM;
      changeOBJ();
      std::cout << "current model : " << files[curModel] << std::endl;
      break;
    case 'g':
      mode = GEOMODE;
      std::cout << "geometrical transformation\nt -- translation\ns -- scale\nr -- rotate\nS -- switch param for adjusting\nx/X -- +/- first value\ny/Y -- +/- second value\nz/Z -- +/- third value\nesc -- exit geometrical transformation mode" << std::endl;
      break;
    case 'v':
      mode = VIEWMODE;
      std::cout << "view transformation\ne -- eye\nc -- center\nu -- up vector\nS -- switch param for adjusting\nx/X -- +/- first value\ny/Y -- +/- second value\nz/Z -- +/- third value\nesc -- exit view transformation mode" << std::endl;
      break;
    case 'p':
      mode = PROJECTIONMODE;
      std::cout << "projection mode\no -- orthographic\np -- perspective\nt -- toggle projection mode\nesc -- exit projection mode" << std::endl;
      break;
    case 'm':
      if (isMultiOBJsMode) {
        std::cout << "enter single obj mode" << std::endl;
        allOBJs = OBJs;
      } else {
        std::cout << "enter multi objs mode" << std::endl;
        allOBJs = MultiOBJs;
      }
      isMultiOBJsMode = !isMultiOBJsMode;
      break;
	}
}

void handleProjectionMode(unsigned char key) {
  switch (key) {
    case 27:
      std::cout << "exit projection mode\nnormal mode" << std::endl;
      mode = NORMALMODE;
      break;
    case 'o':
      std::cout << "orthographic:" << std::endl;
      std::cin >> curOBJ->left >> curOBJ->right >> curOBJ->bottom >>
          curOBJ->top >> curOBJ->near >> curOBJ->far;
      curOBJ->orthographicMat();
      break;
    case 'p':
      std::cout << "perspective:" << std::endl;
      std::cin >> curOBJ->left >> curOBJ->right >> curOBJ->bottom >>
          curOBJ->top >> curOBJ->near >> curOBJ->far;
      curOBJ->perspectiveMat();
      break;
    case 't':
      std::cout << "toggle projection mode" << std::endl;
      curOBJ->toggleProjection();
      break;
  }
}

void processNormalKeys(unsigned char key, int x, int y) {
  switch (mode) {
    case NORMALMODE:
      handleNormalMode(key);
      break;
    case GEOMODE:
      handleGeoMode(key);
      break;
    case VIEWMODE:
      handleViewMode(key);
      break;
    case PROJECTIONMODE:
      handleProjectionMode(key);
      break;
  }
}

int main(int argc, char **argv) {
  
  for (size_t i = 0; i < FILENUM; ++i) {
    OBJs[i].loadObj(files[i]);
    MultiOBJs[i].loadObj(files[i]);
    MultiOBJs[i].geoScale(0.5, 0.5, 0.5);
  }
  
  MultiOBJs[0].geoTrans(-0.5, -0.5, 0);
  MultiOBJs[1].geoTrans(-0.5, 0.5, 0);
  MultiOBJs[2].geoTrans(0.5, 0.5, 0);
  
  changeOBJ();
  
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	glutInitWindowPosition(460, 40);
	glutInitWindowSize(800, 800);
	glutCreateWindow("10320 CS550000 CG HW2 Shangtong Zhang");

	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	// register glut callback functions
	glutDisplayFunc (renderScene);
	glutIdleFunc    (idle);
	glutKeyboardFunc(processNormalKeys);
	glutMouseFunc   (processMouse);
	glutMotionFunc  (processMouseMotion);

	glEnable(GL_DEPTH_TEST);

	// set up shaders here
	setShaders();

	// main loop
	glutMainLoop();

	return 0;
}

