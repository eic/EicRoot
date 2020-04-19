#ifndef CBMLITMATRIXMATH_H_
#define CBMLITMATRIXMATH_H_

#include "base/CbmLitFloat.h"

#include <vector>

//Matrix operations

/* 5x5 symmetric matrix inverse */
bool InvSym15(
   std::vector<litfloat>& a);

/* Multiplication of two 5x5 matrices */
bool Mult25(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* Multiplication of 5x5 matrix and 5x1 vector */
bool Mult25On5(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* Multiplication of 5x5 matrix and 5x1 vector */
bool Mult15On5(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* Transpose of 5x5 matrix */
bool Transpose25(
   std::vector<litfloat>& a);

/* Subtraction of two matrices*/
bool Subtract(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* Addition of two matrices*/
bool Add(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* a*b*a^T */
bool Similarity(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* Multiplication of two 5x5 matrices */
bool Mult15On25(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

/* Multiplication of two 5x5 matrices */
bool Mult25On15(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c);

#endif

