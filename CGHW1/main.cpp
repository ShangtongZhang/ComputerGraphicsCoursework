#include <stdio.h>
#include <stdlib.h>
#include "iostream"
#include "string"

#include <GL/glew.h>
#include <GLUT/GLUT.h>
#include "textfile.h"
#include "glm.h"

#ifndef GLUT_WHEEL_UP
# define GLUT_WHEEL_UP   0x0003
# define GLUT_WHEEL_DOWN 0x0004
#endif

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

#define FILENUM 4
#define INF (1<<30)

// Shader attributes
GLint iLocPosition;
GLint iLocColor;

std::string files[FILENUM] = {"ColorModels/teapot4KC.obj",
                              "ColorModels/boxC.obj",
                              "ColorModels/tigerC.obj",
                              "ColorModels/elephant16KC.obj"};

GLMmodel* OBJ = NULL;
GLfloat* triangle_vertex = NULL;
GLfloat* triangle_color = NULL;
int curModel = 0;

void normalizeModel()
{
  
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
    stride[i] = maxv[i] - minv[i];
  }
  
  int major = stride[0] > stride[1] ? 0 : 1;
  major = stride[major] > stride[2] ? major : 2;
  
  GLfloat majorStride = stride[major];
  for (size_t i = 0; i < 3; ++i) {
    stride[i] /= majorStride;
  }
  
  for (size_t i = 1; i <= OBJ->numvertices; ++i) {
    for (size_t j = 0; j < 3; ++j) {
      OBJ->vertices[3 * i + j] = (OBJ->vertices[3 * i + j] - minv[j]) * 2 * stride[j] /
          (maxv[j] - minv[j]) - stride[j];
    }
  }

}

void loadModel()
{
  if (OBJ) {
    glmDelete(OBJ);
  }
  if (triangle_color) {
    delete[] triangle_color;
  }
  if (triangle_vertex) {
    delete[] triangle_vertex;
  }
  
  std::cout << files[curModel] << std::endl;
	OBJ = glmReadOBJ(files[curModel].data());
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

void idle()
{
	glutPostRedisplay();
}

void renderScene(void)
{
	// clear canvas
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	glEnableVertexAttribArray(iLocPosition);
	glEnableVertexAttribArray(iLocColor);
  
	glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, triangle_vertex);
	glVertexAttribPointer(   iLocColor, 3, GL_FLOAT, GL_FALSE, 0, triangle_color);
  
	// draw the array we just bound
	glDrawArrays(GL_TRIANGLES, 0, OBJ->numtriangles * 3);
  
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

void setShaders()
{
	GLuint v, f, p;
	char *vs = NULL;
	char *fs = NULL;
  
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
  
	vs = textFileRead("CGHW1.vert");
	fs = textFileRead("CGHW1.frag");
  
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
  
	glUseProgram(p);
}


void processMouse(int who, int state, int x, int y)
{
//	printf("(%d, %d) ", x, y);
//  
//	switch(who){
//    case GLUT_LEFT_BUTTON:   printf("left button   "); break;
//    case GLUT_MIDDLE_BUTTON: printf("middle button "); break;
//    case GLUT_RIGHT_BUTTON:  printf("right button  "); break;
//    case GLUT_WHEEL_UP:      printf("wheel up      "); break;
//    case GLUT_WHEEL_DOWN:    printf("wheel down    "); break;
//    default:                 printf("%-14d", who);     break;
//	}
//  
//	switch(state){
//    case GLUT_DOWN:          printf("start ");         break;
//    case GLUT_UP:            printf("end   ");         break;
//	}
//  
//	printf("\n");
}

void processMouseMotion(int x, int y){  // callback on mouse drag
//	printf("(%d, %d) mouse move\n", x, y);
}

void processNormalKeys(unsigned char key, int x, int y) {
	switch(key) {
		case 27: /* the Esc key */
      if (OBJ) {
        glmDelete(OBJ);
      }
      if (triangle_color) {
        delete[] triangle_color;
      }
      if (triangle_vertex) {
        delete[] triangle_vertex;
      }
			exit(0);
			break;
    case 'h':
      std::cout << "ESC -- exit\nb -- back\nn -- next\ns -- solid mode\nw -- wireframe mode";
      break;
    case 'b':
      curModel = (curModel - 1 + FILENUM) % FILENUM;
      loadModel();
      break;
    case 'n':
      curModel = (curModel + 1 + FILENUM) % FILENUM;
      loadModel();
      break;
    case 'w':
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case 's':
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
	}
}

int main(int argc, char **argv) {
	// glut init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  
	// create window
	glutInitWindowPosition(460, 40);
	glutInitWindowSize(800, 800);
	glutCreateWindow("NCTU Shangtong Zhang CG HW1");
  
	glewInit();
	if(glewIsSupported("GL_VERSION_2_0")){
		printf("Ready for OpenGL 2.0\n");
	}else{
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}
  
	// load obj models through glm
	loadModel();
  
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
  
	// free
	glmDelete(OBJ);
  
	return 0;
}

