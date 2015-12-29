#ifndef __MAT4_HPP
#define __MAT4_HPP

#include "iostream"
#include "string"
#include <GL/glew.h>
#include <GLUT/GLUT.h>
#include "textfile.h"
#include "glm.h"
#include "math.h"

GLvoid glmCross(GLfloat* u, GLfloat* v, GLfloat* n) {
  n[0] = u[1]*v[2] - u[2]*v[1];
  n[1] = u[2]*v[0] - u[0]*v[2];
  n[2] = u[0]*v[1] - u[1]*v[0];
}

/* glmNormalize: normalize a vector
 *
 * v - array of 3 GLfloats (GLfloat v[3]) to be normalized
 */
GLvoid glmNormalize(GLfloat* v)
{
  GLfloat l;
  
  l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  v[0] /= l;
  v[1] /= l;
  v[2] /= l;
}

class Mat4 {
public:
  
  Mat4() {
    data = new GLfloat[16];
    for (size_t i = 0; i < 16; ++i) {
      data[i] = 0;
    }
  }
  
  void zero() {
    for (size_t i = 0; i < 16; ++i) {
      data[i] = 0;
    }
  }
  
  void identity() {
    zero();
    for (size_t i = 0; i < 4; ++i) {
      (*this)(i, i) = 1;
    }
  }
  
  Mat4(const Mat4& m) {
    data = new GLfloat[16];
    for (size_t i = 0; i < 16; ++i) {
      data[i] = m.data[i];
    }
  }
  
  Mat4& operator=(const Mat4& m) {
    if (this == &m) {
      return *this;
    }
    if (data) {
      delete[] data;
    }
    data = new GLfloat[16];
    for (size_t i = 0; i < 16; ++i) {
      data[i] = m.data[i];
    }
    return *this;
  }
  
  ~Mat4() {
    if (data)
      delete[] data;
  }
  
  GLfloat& operator()(size_t r, size_t c) {
    return data[c * 4 + r];
  }
  
  GLfloat operator()(size_t r, size_t c) const {
    return data[c * 4 + r];
  }
  
  GLfloat* data;
};

std::ostream& operator<<(std::ostream& out, const Mat4& m) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      out << m(i, j) << " ";
    }
    out << std::endl;
  }
  out << std::endl;
  return out;
}

Mat4 operator*(const Mat4& m1, const Mat4& m2) {
  Mat4 m;
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      for (size_t k = 0; k < 4; ++k) {
        m(i, j) += m1(i, k) * m2(k, j);
      }
    }
  }
  return m;
}

#endif