#ifndef __MAT_HPP
#define __MAT_HPP

#include "iostream"
#include "string"
#include <GL/glew.h>
#include "math.h"

#define INF (1<<30)
#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

bool gluInvertMatrix(const GLfloat m[16], GLfloat invOut[16]) {
  GLfloat inv[16], det;
  int i;
  
  inv[0] = m[5]  * m[10] * m[15] -
  m[5]  * m[11] * m[14] -
  m[9]  * m[6]  * m[15] +
  m[9]  * m[7]  * m[14] +
  m[13] * m[6]  * m[11] -
  m[13] * m[7]  * m[10];
  
  inv[4] = -m[4]  * m[10] * m[15] +
  m[4]  * m[11] * m[14] +
  m[8]  * m[6]  * m[15] -
  m[8]  * m[7]  * m[14] -
  m[12] * m[6]  * m[11] +
  m[12] * m[7]  * m[10];
  
  inv[8] = m[4]  * m[9] * m[15] -
  m[4]  * m[11] * m[13] -
  m[8]  * m[5] * m[15] +
  m[8]  * m[7] * m[13] +
  m[12] * m[5] * m[11] -
  m[12] * m[7] * m[9];
  
  inv[12] = -m[4]  * m[9] * m[14] +
  m[4]  * m[10] * m[13] +
  m[8]  * m[5] * m[14] -
  m[8]  * m[6] * m[13] -
  m[12] * m[5] * m[10] +
  m[12] * m[6] * m[9];
  
  inv[1] = -m[1]  * m[10] * m[15] +
  m[1]  * m[11] * m[14] +
  m[9]  * m[2] * m[15] -
  m[9]  * m[3] * m[14] -
  m[13] * m[2] * m[11] +
  m[13] * m[3] * m[10];
  
  inv[5] = m[0]  * m[10] * m[15] -
  m[0]  * m[11] * m[14] -
  m[8]  * m[2] * m[15] +
  m[8]  * m[3] * m[14] +
  m[12] * m[2] * m[11] -
  m[12] * m[3] * m[10];
  
  inv[9] = -m[0]  * m[9] * m[15] +
  m[0]  * m[11] * m[13] +
  m[8]  * m[1] * m[15] -
  m[8]  * m[3] * m[13] -
  m[12] * m[1] * m[11] +
  m[12] * m[3] * m[9];
  
  inv[13] = m[0]  * m[9] * m[14] -
  m[0]  * m[10] * m[13] -
  m[8]  * m[1] * m[14] +
  m[8]  * m[2] * m[13] +
  m[12] * m[1] * m[10] -
  m[12] * m[2] * m[9];
  
  inv[2] = m[1]  * m[6] * m[15] -
  m[1]  * m[7] * m[14] -
  m[5]  * m[2] * m[15] +
  m[5]  * m[3] * m[14] +
  m[13] * m[2] * m[7] -
  m[13] * m[3] * m[6];
  
  inv[6] = -m[0]  * m[6] * m[15] +
  m[0]  * m[7] * m[14] +
  m[4]  * m[2] * m[15] -
  m[4]  * m[3] * m[14] -
  m[12] * m[2] * m[7] +
  m[12] * m[3] * m[6];
  
  inv[10] = m[0]  * m[5] * m[15] -
  m[0]  * m[7] * m[13] -
  m[4]  * m[1] * m[15] +
  m[4]  * m[3] * m[13] +
  m[12] * m[1] * m[7] -
  m[12] * m[3] * m[5];
  
  inv[14] = -m[0]  * m[5] * m[14] +
  m[0]  * m[6] * m[13] +
  m[4]  * m[1] * m[14] -
  m[4]  * m[2] * m[13] -
  m[12] * m[1] * m[6] +
  m[12] * m[2] * m[5];
  
  inv[3] = -m[1] * m[6] * m[11] +
  m[1] * m[7] * m[10] +
  m[5] * m[2] * m[11] -
  m[5] * m[3] * m[10] -
  m[9] * m[2] * m[7] +
  m[9] * m[3] * m[6];
  
  inv[7] = m[0] * m[6] * m[11] -
  m[0] * m[7] * m[10] -
  m[4] * m[2] * m[11] +
  m[4] * m[3] * m[10] +
  m[8] * m[2] * m[7] -
  m[8] * m[3] * m[6];
  
  inv[11] = -m[0] * m[5] * m[11] +
  m[0] * m[7] * m[9] +
  m[4] * m[1] * m[11] -
  m[4] * m[3] * m[9] -
  m[8] * m[1] * m[7] +
  m[8] * m[3] * m[5];
  
  inv[15] = m[0] * m[5] * m[10] -
  m[0] * m[6] * m[9] -
  m[4] * m[1] * m[10] +
  m[4] * m[2] * m[9] +
  m[8] * m[1] * m[6] -
  m[8] * m[2] * m[5];
  
  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
  
  if (det == 0)
    return false;
  
  det = 1.0 / det;
  
  for (i = 0; i < 16; i++)
    invOut[i] = inv[i] * det;
  
  return true;
}

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

template<int T>
class Vec {
public:
  size_t length;
  void normalize() {
    GLfloat l;
    for (size_t i = 0; i < length; ++i) {
      l += data[i] * data[i];
    }
    l = (GLfloat)(sqrt(l));
    for (size_t i = 0; i < length; ++i) {
      data[i] /= l;
    }
  }
  
  Vec() :
  length(T) {
    data = new GLfloat[length];
    for (size_t i = 0; i < length; ++i) {
      data[i] = 0;
    }
  }
  
  Vec(GLfloat* d, int n):
  length(T) {
    data = new GLfloat[length];
    zero();
    for (size_t i = 0; i < n; ++i) {
      data[i] = d[i];
    }
  }
  
  Vec(GLfloat f1, GLfloat f2, GLfloat f3, GLfloat f4) :
  length(T){
    if (T != 4) {
      std::cout << "dim error!" << std::endl;
    }
    data = new GLfloat[length];
    data[0] = f1;
    data[1] = f2;
    data[2] = f3;
    data[3] = f4;
  }
  
  Vec(GLfloat f1, GLfloat f2, GLfloat f3) :
  length(T){
    if (T != 3) {
      std::cout << "dim error!" << std::endl;
    }
    data = new GLfloat[length];
    data[0] = f1;
    data[1] = f2;
    data[2] = f3;
  }
  
  void zero() {
    for (size_t i = 0; i < length; ++i) {
      data[i] = 0;
    }
  }
  
  
  Vec(const Vec& v) {
    data = new GLfloat[length];
    for (size_t i = 0; i < length; ++i) {
      data[i] = v.data[i];
    }
  }
  
  Vec& operator=(const Vec& v) {
    if (this == &v) {
      return *this;
    }
    if (data) {
      delete[] data;
    }
    data = new GLfloat[length];
    for (size_t i = 0; i < length; ++i) {
      data[i] = v.data[i];
    }
    return *this;
  }
  
  ~Vec() {
    if (data)
      delete[] data;
  }
  
  GLfloat& operator()(size_t i) {
    return data[i];
  }
  
  GLfloat operator()(size_t i) const {
    return data[i];
  }
  
  GLfloat* data;
};

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
  
  Mat4 inverse() {
    Mat4 result;
    if (gluInvertMatrix(data, result.data)) {
      return result;
    } else {
      std::cout << "inverse error!" << std::endl;
      return Mat4();
    }
  }
  
  Mat4 transpose() {
    Mat4 result;
    for (size_t i = 0; i < 4; ++i) {
      for (size_t j = 0; j < 4; ++j) {
        result(i, j) = (*this)(j, i);
      }
    }
    return result;
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