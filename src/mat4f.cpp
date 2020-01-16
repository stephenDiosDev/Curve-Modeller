#include "mat4f.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <functional>
#include <iostream>

namespace math {

Mat4f::Mat4f(float fillValue) { m_values.fill(fillValue); }

Mat4f::Mat4f(Mat4f::array16f values) : m_values(values) {}

Mat4f::Mat4f(std::initializer_list<float> list) {
  assert(list.size() == NUMBER_ELEMENTS);
  std::copy_n(list.begin(), NUMBER_ELEMENTS, m_values.begin());
}

void Mat4f::fill(float t) { m_values.fill(t); }

float &Mat4f::operator()(int row, int column) {
  return m_values[rowMajorIndex(row, column)];
}

float &Mat4f::operator[](int element) { return m_values[element]; }

float &Mat4f::at(int row, int column) {
  return m_values.at(rowMajorIndex(row, column));
}

float &Mat4f::at(int element) { return m_values.at(element); }

float *Mat4f::data() { return m_values.data(); }

float Mat4f::operator()(int row, int column) const {
  return m_values[rowMajorIndex(row, column)];
}

float Mat4f::operator[](int element) const { return m_values[element]; }

float Mat4f::at(int row, int column) const {
  return m_values.at(rowMajorIndex(row, column));
}

float Mat4f::at(int element) const { return m_values.at(element); }

float const *Mat4f::data() const { return m_values.data(); }

int Mat4f::rowMajorIndex(int row, int column) {
  return row * DIMENSION + column;
}

Mat4f Mat4f::identity() {
  return {1.f, 0.f, 0.f, 0.f, // row 0
          0.f, 1.f, 0.f, 0.f, // row 1
          0.f, 0.f, 1.f, 0.f, // row 2
          0.f, 0.f, 0.f, 1.f};
}

Mat4f transposed(Mat4f mat) {
  using std::swap;
  //    0 1 2 3
  // ----------------
  // 0|	0  1  2  3
  // 1| 4  5  6  7
  // 2| 8  9  10 11
  // 3| 12 13 14 15

  swap(mat[1], mat[4]);
  swap(mat[2], mat[8]);
  swap(mat[3], mat[12]);
  swap(mat[6], mat[9]);
  swap(mat[7], mat[13]);
  swap(mat[11], mat[14]);

  return mat;
}

float determinant(Mat4f const &m) {
  float r0 = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - //
             m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +   //
             m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

  float r4 = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + //
             m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -    //
             m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

  float r8 = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - //
             m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +  //
             m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

  float r12 = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + //
              m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -   //
              m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

  return m[0] * r0 + m[1] * r4 + m[2] * r8 + m[3] * r12;
}

Mat4f inverse(Mat4f const &m) {

  //	0	1	2	3
  // ----------------
  // 0|	0	1	2	3
  // 1| 4	5	6	7
  // 2| 8	9	10	11
  // 3| 12	13	14	15

  Mat4f result;

  // hela ugly and explicit (see mesa code)
  result[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - //
              m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +   //
              m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

  result[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + //
              m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -    //
              m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

  result[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - //
              m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +  //
              m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

  result[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + //
               m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -   //
               m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

  result[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + //
              m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -    //
              m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

  result[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - //
              m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +   //
              m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

  result[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + //
              m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -   //
              m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

  result[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - //
               m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +  //
               m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

  result[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - //
              m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + //
              m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

  result[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + //
              m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -  //
              m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

  result[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - //
               m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + //
               m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

  result[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + //
               m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -  //
               m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

  result[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + //
              m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -  //
              m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

  result[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - //
              m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + //
              m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

  result[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + //
               m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -  //
               m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

  result[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - //
               m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + //
               m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

  // determinant(m) same
  float det = m[0] * result[0] + m[1] * result[4] + m[2] * result[8] +
              m[3] * result[12];

  if (det == 0.)         // condition number ? error close to?
    return result * 0.f; // just null it out

  return result * (1.f / det);
}

Mat4f operator+(Mat4f const &lhs, Mat4f rhs) {
  using std::begin;
  using std::end;

  std::transform(begin(lhs), end(lhs), begin(rhs), begin(rhs),
                 std::plus<float>());
  return rhs;
}

Mat4f operator*(Mat4f const &lhs, Mat4f const &rhs) {
  Mat4f result;

  float element = 0.f;
  for (int i = 0; i < Mat4f::DIMENSION; ++i) {
    for (int j = 0; j < Mat4f::DIMENSION; ++j) {
      element = 0.f;
      for (int k = 0; k < Mat4f::DIMENSION; ++k) {
        element += lhs(i, k) * rhs(k, j);
      }
      result(i, j) = element;
    }
  }

  return result;
}

Mat4f operator*(float s, Mat4f rhs) {
  using std::begin;
  using std::end;
  std::transform(begin(rhs), end(rhs), begin(rhs),
                 [=](float f) { return s * f; });
  return rhs;
}

Mat4f operator*(Mat4f lhs, float s) {
  using std::begin;
  using std::end;
  std::transform(begin(lhs), end(lhs), begin(lhs),
                 [=](float f) { return s * f; });
  return lhs;
}

Mat4f::array16f::iterator Mat4f::begin() { return m_values.begin(); }

Mat4f::array16f::iterator Mat4f::end() { return m_values.end(); }

Mat4f::array16f::const_iterator Mat4f::begin() const {
  return m_values.begin();
}

Mat4f::array16f::const_iterator Mat4f::end() const { return m_values.end(); }

std::ostream &operator<<(std::ostream &out, Mat4f const &mat) {
  using std::begin;
  using std::end;

  std::ostream_iterator<float> outIter(out, " ");
  std::copy(begin(mat), end(mat), outIter);

  return out;
}

} // namespace math
