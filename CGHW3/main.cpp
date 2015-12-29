#include <stdio.h>
#include <stdlib.h>
#include "iostream"
#include "string"
#include "vector"

#include <GL/glew.h>
#include "textfile.h"
#ifdef _WIN32
#include "freeglut/glut.h"
#include "freeglut/glui.h"
#elif __APPLE__
#include <GLUT/GLUT.h>
#include <GLUI/GLUI.h>
#endif
#include "glm.h"
#include "Mat.hpp"
#include "ObjWrapper.hpp"
#include "math.h"

#define FILENUM 3
#define NORMALMODE 0
#define GEOMODE 1
#define VIEWMODE 2
#define PROJECTIONMODE 3
#define MULTIOBJSMODE 4
#define LIGHTMODE 5
#define PI 3.1415926
#define STEP 0.2
#define LIGHTNUM 3
#define DIRECTIONAL 1
#define POSITIONAL 2
#define SPOT 3

class LightSourceParameters {
public:
  LightSourceParameters() {
    on = 1;
  }
  
  void bind(GLuint p, std::string i) {
    std::string tmp = "lights[" + i + "].ambient";
    iLocAmbient = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].diffuse";
    iLocDiffuse = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].specular";
    iLocSpecular = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].specular_shininess";
    iLocSpecularShiniess = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].dLightDirection";
    iLocDLightDirection = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].spLightPosition";
    iLocSPLightPosition = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].sLightDirection";
    iLocSLightDirection = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].sLightExp";
    iLocSLightExp = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].sLightCosCutoff";
    iLocSLightCosCutoff = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].on";
    iLocOn = glGetUniformLocation(p, tmp.data());
    tmp = "lights[" + i + "].type";
    iLocType = glGetUniformLocation(p, tmp.data());
  }
  
  void bind() {
    glUniform1i(iLocOn, on);
    glUniform1i(iLocType, type);
    glUniform4fv(iLocAmbient, 1, ambient.data);
    glUniform4fv(iLocDiffuse, 1, diffuse.data);
    glUniform4fv(iLocSpecular, 1, specular.data);
    glUniform1f(iLocSpecularShiniess, specular_shininess);
    
    glUniform3fv(iLocDLightDirection, 1, dLightDirection.data);
    
    glUniform4fv(iLocSPLightPosition, 1, spLightPosition.data);
    
    glUniform3fv(iLocSLightDirection, 1, sLightDirection.data);
    glUniform1f(iLocSLightExp, sLightExp);
    glUniform1f(iLocSLightCosCutoff, sLightCosCutoff);
  }

  int on;
  int type;
  
  Vec<4> ambient;
  Vec<4> diffuse;
  Vec<4> specular;
  float specular_shininess;
  
  Vec<3> dLightDirection;
  
  Vec<4> spLightPosition;
  
  Vec<3> sLightDirection;
  float sLightExp;
  float sLightCosCutoff;
  
  GLint iLocOn;
  GLint iLocType;
  GLint iLocAmbient;
  GLint iLocDiffuse;
  GLint iLocSpecular;
  GLint iLocSpecularShiniess;
  GLint iLocDLightDirection;
  GLint iLocSPLightPosition;
  GLint iLocSLightDirection;
  GLint iLocSLightExp;
  GLint iLocSLightCosCutoff;
  
};

LightSourceParameters dLight, pLight, sLight;

GLint iLocPosition;
GLint iLocNormal;

GLint iLocMVP;
GLint iLocEye;
GLint iLocMDiffuse;
GLint iLocMAmbient;
GLint iLocMSpecular;
GLint iLocMShininess;

GLint iLocModelMat;
GLint iLocITModelMat;

std::string files[FILENUM] = {"NormalModels/teapot4KN.obj",
  "NormalModels/duck4KN.obj",
  "NormalModels/elephant16KN.obj"};

ObjWrapper OBJs[3];
ObjWrapper MultiOBJs[3];
int curModel = 0;
ObjWrapper* allOBJs = OBJs;
ObjWrapper* curOBJ;
int mode = NORMALMODE;

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
  glEnableVertexAttribArray(iLocNormal);
  
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
  
  for (size_t i = 0; i < curOBJ->triangleGroups.size(); ++i) {
    glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0,
                          curOBJ->triangleGroups[i]->triangle_vertex);
    glVertexAttribPointer(iLocNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          curOBJ->triangleGroups[i]->triangle_normal);
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

	vs = textFileRead("CGHW3.vert");
	fs = textFileRead("CGHW3.frag");

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
  
	iLocMVP		 = glGetUniformLocation(p, "mvp");
  iLocEye = glGetUniformLocation(p, "eye");
  
  iLocMDiffuse = glGetUniformLocation(p, "material.diffuse");
  iLocMAmbient = glGetUniformLocation(p, "material.ambient");
  iLocMSpecular = glGetUniformLocation(p, "material.specular");
  iLocMShininess = glGetUniformLocation(p, "material.shininess");
  
  iLocModelMat = glGetUniformLocation(p, "modelMat");
  iLocITModelMat = glGetUniformLocation(p, "itModelMat");
  
  dLight.bind(p, "0");
  pLight.bind(p, "1");
  sLight.bind(p, "2");

	glUseProgram(p);
}

void processMouse(int who, int state, int x, int y) {
}

void processMouseMotion(int x, int y) {
  
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
    case 'l':
      mode = LIGHTMODE;
      break;
//    case 'm':
//      if (isMultiOBJsMode) {
//        std::cout << "enter single obj mode" << std::endl;
//        allOBJs = OBJs;
//      } else {
//        std::cout << "enter multi objs mode" << std::endl;
//        allOBJs = MultiOBJs;
//      }
//      isMultiOBJsMode = !isMultiOBJsMode;
//      break;
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
      std::cin >> curOBJ->leftV >> curOBJ->rightV >> curOBJ->bottomV >>
          curOBJ->topV >> curOBJ->nearV >> curOBJ->farV;
      curOBJ->orthographicMat();
      break;
    case 'p':
      std::cout << "perspective:" << std::endl;
      std::cin >> curOBJ->leftV >> curOBJ->rightV >> curOBJ->bottomV >>
          curOBJ->topV >> curOBJ->nearV >> curOBJ->farV;
      curOBJ->perspectiveMat();
      break;
    case 't':
      std::cout << "toggle projection mode" << std::endl;
      curOBJ->toggleProjection();
      break;
  }
}

void handleLightMode(unsigned char key) {
  switch (key) {
    case 27:
      std::cout << "exit light mode\nnormal mode" << std::endl;
      mode = NORMALMODE;
      break;
    case 'd':
      break;
    case 'p':
      break;
    case 's':
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
    case LIGHTMODE:
      handleLightMode(key);
      break;
  }
}

#define ID_DD 1
#define ID_DA 2
#define ID_DS 3
#define ID_PD 4
#define ID_PA 5
#define ID_PS 6
#define ID_SE 7
#define ID_SC 8
#define ID_DON 9
#define ID_PON 10
#define ID_SON 11
#define ID_PPOS 13
#define ID_SPOS 14
#define ID_DDIR 15
#define ID_SDIR 16

GLUI_EditText *DD[4];
GLUI_EditText *DA[4];
GLUI_EditText *DS[4];
GLUI_EditText *PD[4];
GLUI_EditText *PA[4];
GLUI_EditText *PS[4];
GLUI_EditText *SE;
GLUI_EditText *SC;
GLUI_EditText *PPOS[3];
GLUI_EditText *SPOS[3];
GLUI_EditText *DDIR[3];
GLUI_EditText *SDIR[3];
GLUI_Button *DON;
GLUI_Button *PON;
GLUI_Button *SON;
void handleLightPanelButton(int id) {
  switch (id) {
    case ID_DD:
      for (int i = 0; i < 4; ++i) {
        dLight.diffuse(i) = DD[i]->get_float_val();
      }
      break;
    case ID_DA:
      for (int i = 0; i < 4; ++i) {
        dLight.ambient(i) = DA[i]->get_float_val();
      }
      break;
    case ID_DS:
      for (int i = 0; i < 4; ++i) {
        dLight.specular(i) = DS[i]->get_float_val();
      }
      break;
    case ID_PD:
      for (int i = 0; i < 4; ++i) {
        pLight.diffuse(i) = PD[i]->get_float_val();
      }
      break;
    case ID_PA:
      for (int i = 0; i < 4; ++i) {
        pLight.ambient(i) = PA[i]->get_float_val();
      }
      break;
    case ID_PS:
      for (int i = 0; i < 4; ++i) {
        pLight.specular(i) = PS[i]->get_float_val();
      }
      break;
    case ID_SE:
      sLight.sLightExp = SE->get_float_val();
      break;
    case ID_SC:
      sLight.sLightCosCutoff = SC->get_float_val();
      break;
    case ID_DON:
      dLight.on = (dLight.on + 1) % 2;
      break;
    case ID_PON:
      pLight.on = (pLight.on + 1) % 2;
      break;
    case ID_SON:
      sLight.on = (sLight.on + 1) % 2;
      break;
    case ID_PPOS:
      for (int i = 0; i < 3; ++i) {
        pLight.spLightPosition(i) = PPOS[i]->get_float_val();
      }
      pLight.spLightPosition(3) = 1;
      break;
    case ID_SPOS:
      for (int i = 0; i < 3; ++i) {
        sLight.spLightPosition(i) = SPOS[i]->get_float_val();
      }
      sLight.spLightPosition(3) = 1;
      break;
    case ID_DDIR:
      for (int i = 0; i < 3; ++i) {
        dLight.dLightDirection(i) = DDIR[i]->get_float_val();
      }
      break;
    case ID_SDIR:
      for (int i = 0; i < 3; ++i) {
        sLight.sLightDirection(i) = SDIR[i]->get_float_val();
      }
      break;
  }
}

int main(int argc, char **argv) {
  
  dLight.type = DIRECTIONAL;
  pLight.type = POSITIONAL;
  sLight.type = SPOT;
  
  dLight.diffuse = Vec<4>(0.5, 0.5, 0.5, 1);
  dLight.ambient = Vec<4>(0.5, 0.5, 0.5, 1);
  dLight.specular = Vec<4>(0.5, 0.5, 0.5, 1);
  dLight.dLightDirection = Vec<3>(0, 0, -2);
  
  pLight.diffuse = Vec<4>(1, 1, 1, 1);
  pLight.ambient = Vec<4>(0, 0, 0, 1);
  pLight.specular = Vec<4>(0.1, 0.1, 0.1, 1);
  pLight.spLightPosition = Vec<4>(4, 0, 0, 1);
  
  sLight.diffuse = Vec<4>(0.5, 0.5, 0.5, 1);
  sLight.ambient = Vec<4>(0, 0, 0, 1);
  sLight.specular = Vec<4>(0.1, 0.1, 0.1, 1);
  sLight.spLightPosition = Vec<4>(0, 0, 1, 1);
  sLight.sLightDirection = Vec<3>(0, 0, -1);
  sLight.sLightExp = 1;
  sLight.sLightCosCutoff = 0.9;
  
  dLight.on = 0;
  pLight.on = 0;
  sLight.on = 1;
  
  for (size_t i = 0; i < FILENUM; ++i) {
    OBJs[i].loadObj(files[i]);
  }
  
  changeOBJ();
  
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// create window
	glutInitWindowPosition(460, 40);
	glutInitWindowSize(800, 800);
	int winID = glutCreateWindow("10320 CS550000 CG HW3 Shangtong Zhang");

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
  
  GLUI_Master.set_glutIdleFunc(idle);
  GLUI* glui = GLUI_Master.create_glui("light panel", 0);
  glui->add_statictext("directional light");
  DON = glui->add_button("ON/OFF", ID_DON, (GLUI_Update_CB)handleLightPanelButton);
  glui->add_separator();
  glui->add_statictext("diffuse");
  DD[0] = glui->add_edittext("r:", GLUI_EDITTEXT_FLOAT);
  DD[1] = glui->add_edittext("g:", GLUI_EDITTEXT_FLOAT);
  DD[2] = glui->add_edittext("b:", GLUI_EDITTEXT_FLOAT);
  DD[3] = glui->add_edittext("a:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_DD, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("ambient");
  DA[0] = glui->add_edittext("r:", GLUI_EDITTEXT_FLOAT);
  DA[1] = glui->add_edittext("g:", GLUI_EDITTEXT_FLOAT);
  DA[2] = glui->add_edittext("b:", GLUI_EDITTEXT_FLOAT);
  DA[3] = glui->add_edittext("a:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_DA, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("specular");
  DS[0] = glui->add_edittext("r:", GLUI_EDITTEXT_FLOAT);
  DS[1] = glui->add_edittext("g:", GLUI_EDITTEXT_FLOAT);
  DS[2] = glui->add_edittext("b:", GLUI_EDITTEXT_FLOAT);
  DS[3] = glui->add_edittext("a:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_DS, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("direction");
  DDIR[0] = glui->add_edittext("x:", GLUI_EDITTEXT_FLOAT);
  DDIR[1] = glui->add_edittext("y:", GLUI_EDITTEXT_FLOAT);
  DDIR[2] = glui->add_edittext("z:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_DDIR, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_column();
  
  glui->add_statictext("positional light");
  PON = glui->add_button("ON/OFF", ID_PON, (GLUI_Update_CB)handleLightPanelButton);
  glui->add_separator();
  glui->add_statictext("diffuse");
  PD[0] = glui->add_edittext("r:", GLUI_EDITTEXT_FLOAT);
  PD[1] = glui->add_edittext("g:", GLUI_EDITTEXT_FLOAT);
  PD[2] = glui->add_edittext("b:", GLUI_EDITTEXT_FLOAT);
  PD[3] = glui->add_edittext("a:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_PD, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("ambient");
  PA[0] = glui->add_edittext("r:", GLUI_EDITTEXT_FLOAT);
  PA[1] = glui->add_edittext("g:", GLUI_EDITTEXT_FLOAT);
  PA[2] = glui->add_edittext("b:", GLUI_EDITTEXT_FLOAT);
  PA[3] = glui->add_edittext("a:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_PA, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("specular");
  PS[0] = glui->add_edittext("r:", GLUI_EDITTEXT_FLOAT);
  PS[1] = glui->add_edittext("g:", GLUI_EDITTEXT_FLOAT);
  PS[2] = glui->add_edittext("b:", GLUI_EDITTEXT_FLOAT);
  PS[3] = glui->add_edittext("a:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_PS, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("light position");
  PPOS[0] = glui->add_edittext("x:", GLUI_EDITTEXT_FLOAT);
  PPOS[1] = glui->add_edittext("y:", GLUI_EDITTEXT_FLOAT);
  PPOS[2] = glui->add_edittext("z:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_PPOS, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_column();
  glui->add_statictext("spot light");
  SON = glui->add_button("ON/OFF", ID_SON, (GLUI_Update_CB)handleLightPanelButton);
  glui->add_separator();
  SE = glui->add_edittext("exp:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_SE, (GLUI_Update_CB)handleLightPanelButton);
  
  SC = glui->add_edittext("cos_cutoff:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_SC, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("light position");
  SPOS[0] = glui->add_edittext("x:", GLUI_EDITTEXT_FLOAT);
  SPOS[1] = glui->add_edittext("y:", GLUI_EDITTEXT_FLOAT);
  SPOS[2] = glui->add_edittext("z:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_SPOS, (GLUI_Update_CB)handleLightPanelButton);
  
  glui->add_statictext("direction");
  SDIR[0] = glui->add_edittext("x:", GLUI_EDITTEXT_FLOAT);
  SDIR[1] = glui->add_edittext("y:", GLUI_EDITTEXT_FLOAT);
  SDIR[2] = glui->add_edittext("z:", GLUI_EDITTEXT_FLOAT);
  glui->add_button("ok", ID_SDIR, (GLUI_Update_CB)handleLightPanelButton);

	// main loop
	glutMainLoop();

	return 0;
}

