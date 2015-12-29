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
#include "math.h"

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

class TriangleGroup {
public:
  GLfloat* triangle_vertex;
  GLfloat* triangle_texture;
  GLfloat* triangle_normal;
//  GLfloat* triangle_face_normal;
  GLfloat diffuse[4];
  GLfloat ambient[4];
  GLfloat specular[4];
  GLfloat shininess;
  int numTriangles;
  int imageWidth;
  int imageHeight;
  unsigned char* image;
  std::string textureName;
  
  void copy4(GLfloat* dst, GLfloat* src) {
    for (int i = 0; i < 4; ++i) {
      dst[i] = src[i];
    }
  }
  
  ~TriangleGroup() {
//    if (triangle_face_normal) {
//      delete[] triangle_face_normal;
//    }
    if (triangle_normal) {
      delete[] triangle_normal;
    }
    if (triangle_vertex) {
      delete[] triangle_vertex;
    }
    if (triangle_texture) {
      delete[] triangle_texture;
    }
    if (image) {
      delete[] image;
    }
  }
  
  void setTexture(const std::string filename) {
    FILE *file = fopen(filename.data(), "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, file);
    
    imageWidth = *(int*)&info[18];
    imageHeight = *(int*)&info[22];
    unsigned long imageSize = imageWidth * imageHeight * 3;
    
    if (image != NULL) {
      delete[] image;
    }
    image = new unsigned char[imageSize];
    fread(image, sizeof(unsigned char), imageSize, file);
    fclose(file);
    
    for (unsigned int i = 0; i < imageSize; i += 3) {
      unsigned char temp = image[i];
      image[i] = image[i+2];
      image[i+2] = temp;
    }
  }
  
  TriangleGroup(GLMgroup* group, GLMmodel* OBJ) {
    copy4(diffuse, OBJ->materials[group->material].diffuse);
    copy4(ambient, OBJ->materials[group->material].ambient);
    copy4(specular, OBJ->materials[group->material].specular);
    shininess = OBJ->materials[group->material].shininess;
    
    numTriangles = group->numtriangles;
    triangle_vertex = new GLfloat[numTriangles * 9];
    triangle_texture = new GLfloat[numTriangles * 6];
    triangle_normal = new GLfloat[numTriangles * 9];
    
    std::string filename = std::string("TextureModels/") + std::string(OBJ->materials[group->material].textureImageName);
    textureName = filename;
    FILE *file = fopen(filename.data(), "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, file);
    
    imageWidth = *(int*)&info[18];
    imageHeight = *(int*)&info[22];
    unsigned long imageSize = imageWidth * imageHeight * 3;
    
    image = new unsigned char[imageSize];
    fread(image, sizeof(unsigned char), imageSize, file);
    fclose(file);
    
    for (unsigned int i = 0; i < imageSize; i += 3) {
      unsigned char temp = image[i];
      image[i] = image[i+2];
      image[i+2] = temp;
    }
    
//    triangle_face_normal = new GLfloat[numTriangles * 3];
    
    float textureXMax = -INF;
    float textureXMin = INF;
    float textureYMax = -INF;
    float textureYMin = INF;
    GLfloat textureWidth = 0;
    GLfloat textureHeight = 0;
    
    for (int i = 0; i < numTriangles; ++i) {
      
      int triangleID = group->triangles[i];
      
      int indv1 = OBJ->triangles[triangleID].vindices[0];
      int indv2 = OBJ->triangles[triangleID].vindices[1];
      int indv3 = OBJ->triangles[triangleID].vindices[2];
      
      triangle_vertex[9 * i + 0] = OBJ->vertices[indv1 * 3 + 0];
      triangle_vertex[9 * i + 1] = OBJ->vertices[indv1 * 3 + 1];
      triangle_vertex[9 * i + 2] = OBJ->vertices[indv1 * 3 + 2];
      triangle_vertex[9 * i + 3] = OBJ->vertices[indv2 * 3 + 0];
      triangle_vertex[9 * i + 4] = OBJ->vertices[indv2 * 3 + 1];
      triangle_vertex[9 * i + 5] = OBJ->vertices[indv2 * 3 + 2];
      triangle_vertex[9 * i + 6] = OBJ->vertices[indv3 * 3 + 0];
      triangle_vertex[9 * i + 7] = OBJ->vertices[indv3 * 3 + 1];
      triangle_vertex[9 * i + 8] = OBJ->vertices[indv3 * 3 + 2];
      
      int indt1 = OBJ->triangles[triangleID].tindices[0];
      int indt2 = OBJ->triangles[triangleID].tindices[1];
      int indt3 = OBJ->triangles[triangleID].tindices[2];
      
      textureXMax = max(textureXMax, OBJ->texcoords[indt1 * 2]);
      textureXMax = max(textureXMax, OBJ->texcoords[indt2 * 2]);
      textureXMax = max(textureXMax, OBJ->texcoords[indt3 * 2]);
      
      textureXMin = min(textureXMin, OBJ->texcoords[indt1 * 2]);
      textureXMin = min(textureXMin, OBJ->texcoords[indt2 * 2]);
      textureXMin = min(textureXMin, OBJ->texcoords[indt3 * 2]);
      
      textureYMax = max(textureYMax, OBJ->texcoords[indt1 * 2 + 1]);
      textureYMax = max(textureYMax, OBJ->texcoords[indt2 * 2 + 1]);
      textureYMax = max(textureYMax, OBJ->texcoords[indt3 * 2 + 1]);
      
      textureYMin = min(textureYMin, OBJ->texcoords[indt1 * 2 + 1]);
      textureYMin = min(textureYMin, OBJ->texcoords[indt2 * 2 + 1]);
      textureYMin = min(textureYMin, OBJ->texcoords[indt3 * 2 + 1]);
      
      int indn1 = OBJ->triangles[triangleID].nindices[0];
      int indn2 = OBJ->triangles[triangleID].nindices[1];
      int indn3 = OBJ->triangles[triangleID].nindices[2];
      
      triangle_normal[9 * i + 0] = OBJ->normals[indn1 * 3 + 0];
      triangle_normal[9 * i + 1] = OBJ->normals[indn1 * 3 + 1];
      triangle_normal[9 * i + 2] = OBJ->normals[indn1 * 3 + 2];
      triangle_normal[9 * i + 3] = OBJ->normals[indn2 * 3 + 0];
      triangle_normal[9 * i + 4] = OBJ->normals[indn2 * 3 + 1];
      triangle_normal[9 * i + 5] = OBJ->normals[indn2 * 3 + 2];
      triangle_normal[9 * i + 6] = OBJ->normals[indn3 * 3 + 0];
      triangle_normal[9 * i + 7] = OBJ->normals[indn3 * 3 + 1];
      triangle_normal[9 * i + 8] = OBJ->normals[indn3 * 3 + 2];
      
//      int indfn = OBJ->triangles[triangleID].findex;
//      triangle_face_normal[3 * i + 0] = OBJ->facetnorms[indfn * 3 + 0];
//      triangle_face_normal[3 * i + 1] = OBJ->facetnorms[indfn * 3 + 1];
//      triangle_face_normal[3 * i + 2] = OBJ->facetnorms[indfn * 3 + 2];
      
    }
    
    for (int i = 0; i < numTriangles; ++i) {
      
      int triangleID = group->triangles[i];
      int indt1 = OBJ->triangles[triangleID].tindices[0];
      int indt2 = OBJ->triangles[triangleID].tindices[1];
      int indt3 = OBJ->triangles[triangleID].tindices[2];
      
//      textureWidth = textureXMax - textureXMin;
//      textureHeight = textureYMax - textureYMin;
      
//      triangle_texture[6 * i + 0] = OBJ->texcoords[indt1 * 2] / textureWidth - 0.5;
//      triangle_texture[6 * i + 1] = OBJ->texcoords[indt1 * 2 + 1] / textureHeight - 0.5;
//      triangle_texture[6 * i + 2] = OBJ->texcoords[indt2 * 2] / textureWidth - 0.5;
//      triangle_texture[6 * i + 3] = OBJ->texcoords[indt2 * 2 + 1] / textureHeight - 0.5;
//      triangle_texture[6 * i + 4] = OBJ->texcoords[indt3 * 2] / textureWidth - 0.5;
//      triangle_texture[6 * i + 5] = OBJ->texcoords[indt3 * 2 + 1] / textureHeight - 0.5;
      
      triangle_texture[6 * i + 0] = OBJ->texcoords[indt1 * 2];
      triangle_texture[6 * i + 1] = OBJ->texcoords[indt1 * 2 + 1];
      triangle_texture[6 * i + 2] = OBJ->texcoords[indt2 * 2];
      triangle_texture[6 * i + 3] = OBJ->texcoords[indt2 * 2 + 1];
      triangle_texture[6 * i + 4] = OBJ->texcoords[indt3 * 2];
      triangle_texture[6 * i + 5] = OBJ->texcoords[indt3 * 2 + 1];
    }
  }
  
};

class ObjWrapper {

public:
  GLMmodel* OBJ;
  Mat4 modelMat;
  Mat4 viewMat;
  Mat4 projectionMat;
  
  Mat4 normalMat;
  Mat4 itNormalMat;
  
  GLfloat viewTarget[3];
  GLfloat viewEye[3];
  GLfloat viewUp[3];
  GLfloat geoParamScale[3];
  GLfloat geoParamTrans[3];
  GLfloat geoParamRotate[3];
  GLfloat leftV, rightV, bottomV, topV, nearV, farV;
  bool isOrthographic;
  std::vector<TriangleGroup*> triangleGroups;
  std::vector<std::string> textures;
  
  GLuint* textureIDs;
  int curTexture;
  
  void init() {
    viewTarget[0] = viewTarget[1] = viewEye[0] = viewEye[1] = viewUp[0] = viewUp[2] = 0;
    viewTarget[2] = 0;
    viewEye[2] = 2;
    viewUp[1] = 1;
    for (size_t i = 0; i < 3; ++i) {
      geoParamScale[i] = 1;
      geoParamTrans[i] = geoParamRotate[i] = 0;
    }
    leftV = -1;
    rightV = 1;
    bottomV = -1;
    topV = 1;
    nearV = 0.5;
    farV = 5;
    isOrthographic = true;
    curTexture = 0;
  }
  
  ObjWrapper() {
    init();
    OBJ = NULL;
    textureIDs = NULL;
    modelMat.identity();
    viewTransMat();
    orthographicMat();
//    perspectiveMat();
  }
  
  ~ObjWrapper() {
    if (OBJ) {
      glmDelete(OBJ);
    }
    if (textureIDs) {
      delete[] textureIDs;
    }
    for (size_t i = 0; i < triangleGroups.size(); ++i) {
      delete triangleGroups[i];
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
    normalMat = modelMat;
    itNormalMat = normalMat.inverse().transpose();
    
  }
  
  void setTexture() {
    GLsizei nTextures = (GLsizei)triangleGroups.size();
    if (textureIDs != NULL) {
      delete[] textureIDs;
    }
    textureIDs = new GLuint[nTextures];
    glGenTextures(nTextures, textureIDs);
    for (size_t i = 0; i < triangleGroups.size(); ++i) {
      triangleGroups[i]->setTexture(textures[curTexture]);
      glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGB,
                   triangleGroups[i]->imageWidth,
                   triangleGroups[i]->imageHeight,
                   0,
                   GL_RGB,
                   GL_UNSIGNED_BYTE,
                   triangleGroups[i]->image);
      
    }
    std::cout << "current texture : " << textures[curTexture] << std::endl;
    curTexture = (curTexture + 1) % textures.size();
  }
  
  void loadObj(const std::string& file) {
    if (OBJ) {
      glmDelete(OBJ);
    }
    
    OBJ = glmReadOBJ(file.data());
    if (OBJ->numfacetnorms == 0) {
      glmFacetNormals(OBJ);
    }
    if (OBJ->numnormals == 0) {
      glmVertexNormals(OBJ, 90.0);
    }
    
    GLMgroup* group = OBJ->groups;
    
    while (group) {
      triangleGroups.push_back(new TriangleGroup(group, OBJ));
      group = group->next;
    }
    textures.push_back(triangleGroups[0]->textureName);
    setTexture();
    normalizeModel();

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
  }
  
  void localGeoScale() {
    localGeoScale(geoParamScale[0], geoParamScale[1], geoParamScale[2]);
  }
  
  void localGeoRotate(GLfloat p1, GLfloat p2, GLfloat p3) {
    updateObjCenter();
    geoTrans(-OBJ->position[0], -OBJ->position[1], -OBJ->position[2]);
    geoRotate(p1, p2, p3);
    geoTrans(OBJ->position[0], OBJ->position[1], OBJ->position[2]);
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
    projectionMat(0, 0) = 2 / (rightV - leftV);
    projectionMat(1, 1) = 2 / (topV - bottomV);
    projectionMat(2, 2) = -2 / (farV - nearV);
    projectionMat(3, 3) = 1;
    projectionMat(0, 3) = - (rightV + leftV) / (rightV - leftV);
    projectionMat(1, 3) = - (topV + bottomV) / (topV - bottomV);
    projectionMat(2, 3) = - (farV + nearV) / (farV - nearV);
  }
  
  void perspectiveMat() {
    projectionMat.zero();
    projectionMat(0, 0) = 2 * nearV / (rightV - leftV);
    projectionMat(1, 1) = 2 * nearV / (topV - bottomV);
    projectionMat(3, 2) = -1;
    projectionMat(0, 2) = (rightV + leftV) / (rightV - leftV);
    projectionMat(1, 2) = (topV + bottomV) / (topV - bottomV);
    projectionMat(2, 2) = -(farV + nearV) / (farV - nearV);
    projectionMat(2, 3) = -2 * farV * nearV / (farV - nearV);
  }
};