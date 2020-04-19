#include "utils/CbmLitMatrixMath.h"

#include <iostream>

/*
 * Matrix operations
 */

// SMij are indices for a 5x5 matrix.

#define SM00 0
#define SM01 1
#define SM02 2
#define SM03 3
#define SM04 4

#define SM10 1
#define SM11 5
#define SM12 6
#define SM13 7
#define SM14 8

#define SM20 2
#define SM21 6
#define SM22 9
#define SM23 10
#define SM24 11

#define SM30 3
#define SM31 7
#define SM32 10
#define SM33 12
#define SM34 13

#define SM40 4
#define SM41 8
#define SM42 11
#define SM43 13
#define SM44 14

bool InvSym15(
   std::vector<litfloat>& a)
{
   if (a.size() != 15) {
      std::cout << "-E- InvSym15: size is not correct" << std::endl;
      return false;
   }

   litfloat* pM = &a[0];

   // Find all NECESSARY 2x2 dets:  (25 of them)

   const litfloat mDet2_23_01 = pM[SM20]*pM[SM31] - pM[SM21]*pM[SM30];
   const litfloat mDet2_23_02 = pM[SM20]*pM[SM32] - pM[SM22]*pM[SM30];
   const litfloat mDet2_23_03 = pM[SM20]*pM[SM33] - pM[SM23]*pM[SM30];
   const litfloat mDet2_23_12 = pM[SM21]*pM[SM32] - pM[SM22]*pM[SM31];
   const litfloat mDet2_23_13 = pM[SM21]*pM[SM33] - pM[SM23]*pM[SM31];
   const litfloat mDet2_23_23 = pM[SM22]*pM[SM33] - pM[SM23]*pM[SM32];
   const litfloat mDet2_24_01 = pM[SM20]*pM[SM41] - pM[SM21]*pM[SM40];
   const litfloat mDet2_24_02 = pM[SM20]*pM[SM42] - pM[SM22]*pM[SM40];
   const litfloat mDet2_24_03 = pM[SM20]*pM[SM43] - pM[SM23]*pM[SM40];
   const litfloat mDet2_24_04 = pM[SM20]*pM[SM44] - pM[SM24]*pM[SM40];
   const litfloat mDet2_24_12 = pM[SM21]*pM[SM42] - pM[SM22]*pM[SM41];
   const litfloat mDet2_24_13 = pM[SM21]*pM[SM43] - pM[SM23]*pM[SM41];
   const litfloat mDet2_24_14 = pM[SM21]*pM[SM44] - pM[SM24]*pM[SM41];
   const litfloat mDet2_24_23 = pM[SM22]*pM[SM43] - pM[SM23]*pM[SM42];
   const litfloat mDet2_24_24 = pM[SM22]*pM[SM44] - pM[SM24]*pM[SM42];
   const litfloat mDet2_34_01 = pM[SM30]*pM[SM41] - pM[SM31]*pM[SM40];
   const litfloat mDet2_34_02 = pM[SM30]*pM[SM42] - pM[SM32]*pM[SM40];
   const litfloat mDet2_34_03 = pM[SM30]*pM[SM43] - pM[SM33]*pM[SM40];
   const litfloat mDet2_34_04 = pM[SM30]*pM[SM44] - pM[SM34]*pM[SM40];
   const litfloat mDet2_34_12 = pM[SM31]*pM[SM42] - pM[SM32]*pM[SM41];
   const litfloat mDet2_34_13 = pM[SM31]*pM[SM43] - pM[SM33]*pM[SM41];
   const litfloat mDet2_34_14 = pM[SM31]*pM[SM44] - pM[SM34]*pM[SM41];
   const litfloat mDet2_34_23 = pM[SM32]*pM[SM43] - pM[SM33]*pM[SM42];
   const litfloat mDet2_34_24 = pM[SM32]*pM[SM44] - pM[SM34]*pM[SM42];
   const litfloat mDet2_34_34 = pM[SM33]*pM[SM44] - pM[SM34]*pM[SM43];

   // Find all NECESSARY 3x3 dets:   (30 of them)

   const litfloat mDet3_123_012 = pM[SM10]*mDet2_23_12 - pM[SM11]*mDet2_23_02 + pM[SM12]*mDet2_23_01;
   const litfloat mDet3_123_013 = pM[SM10]*mDet2_23_13 - pM[SM11]*mDet2_23_03 + pM[SM13]*mDet2_23_01;
   const litfloat mDet3_123_023 = pM[SM10]*mDet2_23_23 - pM[SM12]*mDet2_23_03 + pM[SM13]*mDet2_23_02;
   const litfloat mDet3_123_123 = pM[SM11]*mDet2_23_23 - pM[SM12]*mDet2_23_13 + pM[SM13]*mDet2_23_12;
   const litfloat mDet3_124_012 = pM[SM10]*mDet2_24_12 - pM[SM11]*mDet2_24_02 + pM[SM12]*mDet2_24_01;
   const litfloat mDet3_124_013 = pM[SM10]*mDet2_24_13 - pM[SM11]*mDet2_24_03 + pM[SM13]*mDet2_24_01;
   const litfloat mDet3_124_014 = pM[SM10]*mDet2_24_14 - pM[SM11]*mDet2_24_04 + pM[SM14]*mDet2_24_01;
   const litfloat mDet3_124_023 = pM[SM10]*mDet2_24_23 - pM[SM12]*mDet2_24_03 + pM[SM13]*mDet2_24_02;
   const litfloat mDet3_124_024 = pM[SM10]*mDet2_24_24 - pM[SM12]*mDet2_24_04 + pM[SM14]*mDet2_24_02;
   const litfloat mDet3_124_123 = pM[SM11]*mDet2_24_23 - pM[SM12]*mDet2_24_13 + pM[SM13]*mDet2_24_12;
   const litfloat mDet3_124_124 = pM[SM11]*mDet2_24_24 - pM[SM12]*mDet2_24_14 + pM[SM14]*mDet2_24_12;
   const litfloat mDet3_134_012 = pM[SM10]*mDet2_34_12 - pM[SM11]*mDet2_34_02 + pM[SM12]*mDet2_34_01;
   const litfloat mDet3_134_013 = pM[SM10]*mDet2_34_13 - pM[SM11]*mDet2_34_03 + pM[SM13]*mDet2_34_01;
   const litfloat mDet3_134_014 = pM[SM10]*mDet2_34_14 - pM[SM11]*mDet2_34_04 + pM[SM14]*mDet2_34_01;
   const litfloat mDet3_134_023 = pM[SM10]*mDet2_34_23 - pM[SM12]*mDet2_34_03 + pM[SM13]*mDet2_34_02;
   const litfloat mDet3_134_024 = pM[SM10]*mDet2_34_24 - pM[SM12]*mDet2_34_04 + pM[SM14]*mDet2_34_02;
   const litfloat mDet3_134_034 = pM[SM10]*mDet2_34_34 - pM[SM13]*mDet2_34_04 + pM[SM14]*mDet2_34_03;
   const litfloat mDet3_134_123 = pM[SM11]*mDet2_34_23 - pM[SM12]*mDet2_34_13 + pM[SM13]*mDet2_34_12;
   const litfloat mDet3_134_124 = pM[SM11]*mDet2_34_24 - pM[SM12]*mDet2_34_14 + pM[SM14]*mDet2_34_12;
   const litfloat mDet3_134_134 = pM[SM11]*mDet2_34_34 - pM[SM13]*mDet2_34_14 + pM[SM14]*mDet2_34_13;
   const litfloat mDet3_234_012 = pM[SM20]*mDet2_34_12 - pM[SM21]*mDet2_34_02 + pM[SM22]*mDet2_34_01;
   const litfloat mDet3_234_013 = pM[SM20]*mDet2_34_13 - pM[SM21]*mDet2_34_03 + pM[SM23]*mDet2_34_01;
   const litfloat mDet3_234_014 = pM[SM20]*mDet2_34_14 - pM[SM21]*mDet2_34_04 + pM[SM24]*mDet2_34_01;
   const litfloat mDet3_234_023 = pM[SM20]*mDet2_34_23 - pM[SM22]*mDet2_34_03 + pM[SM23]*mDet2_34_02;
   const litfloat mDet3_234_024 = pM[SM20]*mDet2_34_24 - pM[SM22]*mDet2_34_04 + pM[SM24]*mDet2_34_02;
   const litfloat mDet3_234_034 = pM[SM20]*mDet2_34_34 - pM[SM23]*mDet2_34_04 + pM[SM24]*mDet2_34_03;
   const litfloat mDet3_234_123 = pM[SM21]*mDet2_34_23 - pM[SM22]*mDet2_34_13 + pM[SM23]*mDet2_34_12;
   const litfloat mDet3_234_124 = pM[SM21]*mDet2_34_24 - pM[SM22]*mDet2_34_14 + pM[SM24]*mDet2_34_12;
   const litfloat mDet3_234_134 = pM[SM21]*mDet2_34_34 - pM[SM23]*mDet2_34_14 + pM[SM24]*mDet2_34_13;
   const litfloat mDet3_234_234 = pM[SM22]*mDet2_34_34 - pM[SM23]*mDet2_34_24 + pM[SM24]*mDet2_34_23;

   // Find all NECESSARY 4x4 dets:   (15 of them)

   const litfloat mDet4_0123_0123 = pM[SM00]*mDet3_123_123 - pM[SM01]*mDet3_123_023
                               + pM[SM02]*mDet3_123_013 - pM[SM03]*mDet3_123_012;
   const litfloat mDet4_0124_0123 = pM[SM00]*mDet3_124_123 - pM[SM01]*mDet3_124_023
                               + pM[SM02]*mDet3_124_013 - pM[SM03]*mDet3_124_012;
   const litfloat mDet4_0124_0124 = pM[SM00]*mDet3_124_124 - pM[SM01]*mDet3_124_024
                               + pM[SM02]*mDet3_124_014 - pM[SM04]*mDet3_124_012;
   const litfloat mDet4_0134_0123 = pM[SM00]*mDet3_134_123 - pM[SM01]*mDet3_134_023
                               + pM[SM02]*mDet3_134_013 - pM[SM03]*mDet3_134_012;
   const litfloat mDet4_0134_0124 = pM[SM00]*mDet3_134_124 - pM[SM01]*mDet3_134_024
                               + pM[SM02]*mDet3_134_014 - pM[SM04]*mDet3_134_012;
   const litfloat mDet4_0134_0134 = pM[SM00]*mDet3_134_134 - pM[SM01]*mDet3_134_034
                               + pM[SM03]*mDet3_134_014 - pM[SM04]*mDet3_134_013;
   const litfloat mDet4_0234_0123 = pM[SM00]*mDet3_234_123 - pM[SM01]*mDet3_234_023
                               + pM[SM02]*mDet3_234_013 - pM[SM03]*mDet3_234_012;
   const litfloat mDet4_0234_0124 = pM[SM00]*mDet3_234_124 - pM[SM01]*mDet3_234_024
                               + pM[SM02]*mDet3_234_014 - pM[SM04]*mDet3_234_012;
   const litfloat mDet4_0234_0134 = pM[SM00]*mDet3_234_134 - pM[SM01]*mDet3_234_034
                               + pM[SM03]*mDet3_234_014 - pM[SM04]*mDet3_234_013;
   const litfloat mDet4_0234_0234 = pM[SM00]*mDet3_234_234 - pM[SM02]*mDet3_234_034
                               + pM[SM03]*mDet3_234_024 - pM[SM04]*mDet3_234_023;
   const litfloat mDet4_1234_0123 = pM[SM10]*mDet3_234_123 - pM[SM11]*mDet3_234_023
                               + pM[SM12]*mDet3_234_013 - pM[SM13]*mDet3_234_012;
   const litfloat mDet4_1234_0124 = pM[SM10]*mDet3_234_124 - pM[SM11]*mDet3_234_024
                               + pM[SM12]*mDet3_234_014 - pM[SM14]*mDet3_234_012;
   const litfloat mDet4_1234_0134 = pM[SM10]*mDet3_234_134 - pM[SM11]*mDet3_234_034
                               + pM[SM13]*mDet3_234_014 - pM[SM14]*mDet3_234_013;
   const litfloat mDet4_1234_0234 = pM[SM10]*mDet3_234_234 - pM[SM12]*mDet3_234_034
                               + pM[SM13]*mDet3_234_024 - pM[SM14]*mDet3_234_023;
   const litfloat mDet4_1234_1234 = pM[SM11]*mDet3_234_234 - pM[SM12]*mDet3_234_134
                               + pM[SM13]*mDet3_234_124 - pM[SM14]*mDet3_234_123;

   // Find the 5x5 det:

   const litfloat det = pM[SM00]*mDet4_1234_1234 - pM[SM01]*mDet4_1234_0234 + pM[SM02]*mDet4_1234_0134
                   - pM[SM03]*mDet4_1234_0124 + pM[SM04]*mDet4_1234_0123;

   if (det == 0) {
      std::cout << "-E- InvSym15: zero determinant" << std::endl;
      return false;
   }

   const litfloat oneOverDet = 1.0/det;
   const litfloat mn1OverDet = - oneOverDet;

   pM[SM00] = mDet4_1234_1234 * oneOverDet;
   pM[SM01] = mDet4_1234_0234 * mn1OverDet;
   pM[SM02] = mDet4_1234_0134 * oneOverDet;
   pM[SM03] = mDet4_1234_0124 * mn1OverDet;
   pM[SM04] = mDet4_1234_0123 * oneOverDet;

   pM[SM11] = mDet4_0234_0234 * oneOverDet;
   pM[SM12] = mDet4_0234_0134 * mn1OverDet;
   pM[SM13] = mDet4_0234_0124 * oneOverDet;
   pM[SM14] = mDet4_0234_0123 * mn1OverDet;

   pM[SM22] = mDet4_0134_0134 * oneOverDet;
   pM[SM23] = mDet4_0134_0124 * mn1OverDet;
   pM[SM24] = mDet4_0134_0123 * oneOverDet;

   pM[SM33] = mDet4_0124_0124 * oneOverDet;
   pM[SM34] = mDet4_0124_0123 * mn1OverDet;

   pM[SM44] = mDet4_0123_0123 * oneOverDet;

   return true;
}


bool Mult25(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != 25 || b.size() != 25 || c.size() != 25) {
      std::cout << "-E- Mult25: size is not correct" << std::endl;
      return false;
   }

   c[0] = a[0] * b[0] + a[1] * b[5] + a[2] * b[10] + a[3] * b[15] + a[4] * b[20];
   c[1] = a[0] * b[1] + a[1] * b[6] + a[2] * b[11] + a[3] * b[16] + a[4] * b[21];
   c[2] = a[0] * b[2] + a[1] * b[7] + a[2] * b[12] + a[3] * b[17] + a[4] * b[22];
   c[3] = a[0] * b[3] + a[1] * b[8] + a[2] * b[13] + a[3] * b[18] + a[4] * b[23];
   c[4] = a[0] * b[4] + a[1] * b[9] + a[2] * b[14] + a[3] * b[19] + a[4] * b[24];
   c[5] = a[5] * b[0] + a[6] * b[5] + a[7] * b[10] + a[8] * b[15] + a[9] * b[20];
   c[6] = a[5] * b[1] + a[6] * b[6] + a[7] * b[11] + a[8] * b[16] + a[9] * b[21];
   c[7] = a[5] * b[2] + a[6] * b[7] + a[7] * b[12] + a[8] * b[17] + a[9] * b[22];
   c[8] = a[5] * b[3] + a[6] * b[8] + a[7] * b[13] + a[8] * b[18] + a[9] * b[23];
   c[9] = a[5] * b[4] + a[6] * b[9] + a[7] * b[14] + a[8] * b[19] + a[9] * b[24];
   c[10] = a[10] * b[0] + a[11] * b[5] + a[12] * b[10] + a[13] * b[15] + a[14] * b[20];
   c[11] = a[10] * b[1] + a[11] * b[6] + a[12] * b[11] + a[13] * b[16] + a[14] * b[21];
   c[12] = a[10] * b[2] + a[11] * b[7] + a[12] * b[12] + a[13] * b[17] + a[14] * b[22];
   c[13] = a[10] * b[3] + a[11] * b[8] + a[12] * b[13] + a[13] * b[18] + a[14] * b[23];
   c[14] = a[10] * b[4] + a[11] * b[9] + a[12] * b[14] + a[13] * b[19] + a[14] * b[24];
   c[15] = a[15] * b[0] + a[16] * b[5] + a[17] * b[10] + a[18] * b[15] + a[19] * b[20];
   c[16] = a[15] * b[1] + a[16] * b[6] + a[17] * b[11] + a[18] * b[16] + a[19] * b[21];
   c[17] = a[15] * b[2] + a[16] * b[7] + a[17] * b[12] + a[18] * b[17] + a[19] * b[22];
   c[18] = a[15] * b[3] + a[16] * b[8] + a[17] * b[13] + a[18] * b[18] + a[19] * b[23];
   c[19] = a[15] * b[4] + a[16] * b[9] + a[17] * b[14] + a[18] * b[19] + a[19] * b[24];
   c[20] = a[20] * b[0] + a[21] * b[5] + a[22] * b[10] + a[23] * b[15] + a[24] * b[20];
   c[21] = a[20] * b[1] + a[21] * b[6] + a[22] * b[11] + a[23] * b[16] + a[24] * b[21];
   c[22] = a[20] * b[2] + a[21] * b[7] + a[22] * b[12] + a[23] * b[17] + a[24] * b[22];
   c[23] = a[20] * b[3] + a[21] * b[8] + a[22] * b[13] + a[23] * b[18] + a[24] * b[23];
   c[24] = a[20] * b[4] + a[21] * b[9] + a[22] * b[14] + a[23] * b[19] + a[24] * b[24];

   return true;
}


bool Transpose25(
   std::vector<litfloat>& a)
{
   if (a.size() != 25) {
      std::cout << "-E- Transpose25: size is not correct" << std::endl;
      return true;
   }
   std::vector<litfloat> b(a);
   a[0] = b[0];
   a[1] = b[5];
   a[2] = b[10];
   a[3] = b[15];
   a[4] = b[20];
   a[5] = b[1];
   a[6] = b[6];
   a[7] = b[11];
   a[8] = b[16];
   a[9] = b[21];
   a[10] = b[2];
   a[11] = b[7];
   a[12] = b[12];
   a[13] = b[17];
   a[14] = b[22];
   a[15] = b[3];
   a[16] = b[8];
   a[17] = b[13];
   a[18] = b[18];
   a[19] = b[23];
   a[20] = b[4];
   a[21] = b[9];
   a[22] = b[14];
   a[23] = b[19];
   a[24] = b[24];
   return true;
}


bool Mult25On5(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != 25 || b.size() != 5 || c.size() != 5) {
      std::cout << "-E- Mult25On5: size is not correct" << std::endl;
      return false;
   }
   c[0] = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3] + a[4] * b[4];
   c[1] = a[5] * b[0] + a[6] * b[1] + a[7] * b[2] + a[8] * b[3] + a[9] * b[4];
   c[2] = a[10] * b[0] + a[11] * b[1] + a[12] * b[2] + a[13] * b[3] + a[14] * b[4];
   c[3] = a[15] * b[0] + a[16] * b[1] + a[17] * b[2] + a[18] * b[3] + a[19] * b[4];
   c[4] = a[20] * b[0] + a[21] * b[1] + a[22] * b[2] + a[23] * b[3] + a[24] * b[4];
   return true;
}

bool Mult15On5(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != 15 || b.size() != 5 || c.size() != 5) {
      std::cout << "-E- Mult15On5: size is not correct" << std::endl;
      return false;
   }
   c[0] = a[0] * b[0] + a[1] * b[1] + a[2]  * b[2] + a[3]  * b[3] + a[4]  * b[4];
   c[1] = a[1] * b[0] + a[5] * b[1] + a[6]  * b[2] + a[7]  * b[3] + a[8]  * b[4];
   c[2] = a[2] * b[0] + a[6] * b[1] + a[9]  * b[2] + a[10] * b[3] + a[11] * b[4];
   c[3] = a[3] * b[0] + a[7] * b[1] + a[10] * b[2] + a[12] * b[3] + a[13] * b[4];
   c[4] = a[4] * b[0] + a[8] * b[1] + a[11] * b[2] + a[13] * b[3] + a[14] * b[4];
   return true;
}

bool Subtract(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != b.size() || a.size() != c.size()) {
      std::cout << "-E- Subtract: size is not correct" << std::endl;
      return false;
   }
   for (unsigned int i = 0; i < a.size(); ++i) { c[i] = a[i] - b[i]; }
   return true;
}



bool Add(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != b.size() || a.size() != c.size()) {
      std::cout << "-E- Add: size is not correct" << std::endl;
      return false;
   }
   for (unsigned int i = 0; i < a.size(); ++i) { c[i] = a[i] + b[i]; }
   return true;
}


/* a*b*a^T */
bool Similarity(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != 25 || b.size() != 15 || c.size() != 15) {
      std::cout << "-E- Similarity: size is not correct" << std::endl;
      return false;
   }

   litfloat A = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3] + a[4] * b[4];
   litfloat B = a[0] * b[1] + a[1] * b[5] + a[2] * b[6] + a[3] * b[7] + a[4] * b[8];
   litfloat C = a[0] * b[2] + a[1] * b[6] + a[2] * b[9] + a[3] * b[10] + a[4] * b[11];
   litfloat D = a[0] * b[3] + a[1] * b[7] + a[2] * b[10] + a[3] * b[12] + a[4] * b[13];
   litfloat E = a[0] * b[4] + a[1] * b[8] + a[2] * b[11] + a[3] * b[13] + a[4] * b[14];

   litfloat F = a[5] * b[0] + a[6] * b[1] + a[7] * b[2] + a[8] * b[3] + a[9] * b[4];
   litfloat G = a[5] * b[1] + a[6] * b[5] + a[7] * b[6] + a[8] * b[7] + a[9] * b[8];
   litfloat H = a[5] * b[2] + a[6] * b[6] + a[7] * b[9] + a[8] * b[10] + a[9] * b[11];
   litfloat I = a[5] * b[3] + a[6] * b[7] + a[7] * b[10] + a[8] * b[12] + a[9] * b[13];
   litfloat J = a[5] * b[4] + a[6] * b[8] + a[7] * b[11] + a[8] * b[13] + a[9] * b[14];

   litfloat K = a[10] * b[0] + a[11] * b[1] + a[12] * b[2] + a[13] * b[3] + a[14] * b[4];
   litfloat L = a[10] * b[1] + a[11] * b[5] + a[12] * b[6] + a[13] * b[7] + a[14] * b[8];
   litfloat M = a[10] * b[2] + a[11] * b[6] + a[12] * b[9] + a[13] * b[10] + a[14] * b[11];
   litfloat N = a[10] * b[3] + a[11] * b[7] + a[12] * b[10] + a[13] * b[12] + a[14] * b[13];
   litfloat O = a[10] * b[4] + a[11] * b[8] + a[12] * b[11] + a[13] * b[13] + a[14] * b[14];

   litfloat P = a[15] * b[0] + a[16] * b[1] + a[17] * b[2] + a[18] * b[3] + a[19] * b[4];
   litfloat Q = a[15] * b[1] + a[16] * b[5] + a[17] * b[6] + a[18] * b[7] + a[19] * b[8];
   litfloat R = a[15] * b[2] + a[16] * b[6] + a[17] * b[9] + a[18] * b[10] + a[19] * b[11];
   litfloat S = a[15] * b[3] + a[16] * b[7] + a[17] * b[10] + a[18] * b[12] + a[19] * b[13];
   litfloat T = a[15] * b[4] + a[16] * b[8] + a[17] * b[11] + a[18] * b[13] + a[19] * b[14];

   c[0] = A * a[0]  + B * a[1]  + C * a[2]  + D * a[3]  + E * a[4];
   c[1] = A * a[5]  + B * a[6]  + C * a[7]  + D * a[8]  + E * a[9];
   c[2] = A * a[10] + B * a[11] + C * a[12] + D * a[13] + E * a[14];
   c[3] = A * a[15] + B * a[16] + C * a[17] + D * a[18] + E * a[19];
   c[4] = A * a[20] + B * a[21] + C * a[22] + D * a[23] + E * a[24];

   c[5] = F * a[5]  + G * a[6]  + H * a[7]  + I * a[8]  + J * a[9];
   c[6] = F * a[10] + G * a[11] + H * a[12] + I * a[13] + J * a[14];
   c[7] = F * a[15] + G * a[16] + H * a[17] + I * a[18] + J * a[19];
   c[8] = F * a[20] + G * a[21] + H * a[22] + I * a[23] + J * a[24];

   c[9]  = K * a[10] + L * a[11] + M * a[12] + N * a[13] + O * a[14];
   c[10] = K * a[15] + L * a[16] + M * a[17] + N * a[18] + O * a[19];
   c[11] = K * a[20] + L * a[21] + M * a[22] + N * a[23] + O * a[24];

   c[12] = P * a[15] + Q * a[16] + R * a[17] + S * a[18] + T * a[19];
   c[13] = P * a[20] + Q * a[21] + R * a[22] + S * a[23] + T * a[24];

   c[14] = (a[20] * b[0] + a[21] * b[1] + a[22] * b[2] + a[23] * b[3] + a[24] * b[4]) * a[20] +
           (a[20] * b[1] + a[21] * b[5] + a[22] * b[6] + a[23] * b[7] + a[24] * b[8]) * a[21] +
           (a[20] * b[2] + a[21] * b[6] + a[22] * b[9] + a[23] * b[10] + a[24] * b[11]) * a[22] +
           (a[20] * b[3] + a[21] * b[7] + a[22] * b[10] + a[23] * b[12] + a[24] * b[13]) * a[23] +
           (a[20] * b[4] + a[21] * b[8] + a[22] * b[11] + a[23] * b[13] + a[24] * b[14]) * a[24];
   return true;
}



bool Mult15On25(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != 15 || b.size() != 25 || c.size() != 25) {
      std::cout << "-E- Mult15On25: size is not correct" << std::endl;
      return false;
   }
   c[0] = a[0] * b[0] + a[1] * b[5] + a[2] * b[10] + a[3] * b[15] + a[4] * b[20];
   c[1] = a[0] * b[1] + a[1] * b[6] + a[2] * b[11] + a[3] * b[16] + a[4] * b[21];
   c[2] = a[0] * b[2] + a[1] * b[7] + a[2] * b[12] + a[3] * b[17] + a[4] * b[22];
   c[3] = a[0] * b[3] + a[1] * b[8] + a[2] * b[13] + a[3] * b[18] + a[4] * b[23];
   c[4] = a[0] * b[4] + a[1] * b[9] + a[2] * b[14] + a[3] * b[19] + a[4] * b[24];
   c[5] = a[1] * b[0] + a[5] * b[5] + a[6] * b[10] + a[7] * b[15] + a[8] * b[20];
   c[6] = a[1] * b[1] + a[5] * b[6] + a[6] * b[11] + a[7] * b[16] + a[8] * b[21];
   c[7] = a[1] * b[2] + a[5] * b[7] + a[6] * b[12] + a[7] * b[17] + a[8] * b[22];
   c[8] = a[1] * b[3] + a[5] * b[8] + a[6] * b[13] + a[7] * b[18] + a[8] * b[23];
   c[9] = a[1] * b[4] + a[5] * b[9] + a[6] * b[14] + a[7] * b[19] + a[8] * b[24];
   c[10] = a[2] * b[0] + a[6] * b[5] + a[9] * b[10] + a[10] * b[15] + a[11] * b[20];
   c[11] = a[2] * b[1] + a[6] * b[6] + a[9] * b[11] + a[10] * b[16] + a[11] * b[21];
   c[12] = a[2] * b[2] + a[6] * b[7] + a[9] * b[12] + a[10] * b[17] + a[11] * b[22];
   c[13] = a[2] * b[3] + a[6] * b[8] + a[9] * b[13] + a[10] * b[18] + a[11] * b[23];
   c[14] = a[2] * b[4] + a[6] * b[9] + a[9] * b[14] + a[10] * b[19] + a[11] * b[24];
   c[15] = a[3] * b[0] + a[7] * b[5] + a[10] * b[10] + a[12] * b[15] + a[13] * b[20];
   c[16] = a[3] * b[1] + a[7] * b[6] + a[10] * b[11] + a[12] * b[16] + a[13] * b[21];
   c[17] = a[3] * b[2] + a[7] * b[7] + a[10] * b[12] + a[12] * b[17] + a[13] * b[22];
   c[18] = a[3] * b[3] + a[7] * b[8] + a[10] * b[13] + a[12] * b[18] + a[13] * b[23];
   c[19] = a[3] * b[4] + a[7] * b[9] + a[10] * b[14] + a[12] * b[19] + a[13] * b[24];
   c[20] = a[4] * b[0] + a[8] * b[5] + a[11] * b[10] + a[13] * b[15] + a[14] * b[20];
   c[21] = a[4] * b[1] + a[8] * b[6] + a[11] * b[11] + a[13] * b[16] + a[14] * b[21];
   c[22] = a[4] * b[2] + a[8] * b[7] + a[11] * b[12] + a[13] * b[17] + a[14] * b[22];
   c[23] = a[4] * b[3] + a[8] * b[8] + a[11] * b[13] + a[13] * b[18] + a[14] * b[23];
   c[24] = a[4] * b[4] + a[8] * b[9] + a[11] * b[14] + a[13] * b[19] + a[14] * b[24];

   return true;
}

bool Mult25On15(
   const std::vector<litfloat>& a,
   const std::vector<litfloat>& b,
   std::vector<litfloat>& c)
{
   if (a.size() != 25 || b.size() != 15 || c.size() != 25) {
      std::cout << "-E- Mult15On25: size is not correct" << std::endl;
      return false;
   }
   c[0] = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3] + a[4] * b[4];
   c[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[6] + a[3] * b[7] + a[4] * b[8];
   c[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[9] + a[3] * b[10] + a[4] * b[11];
   c[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[10] + a[3] * b[12] + a[4] * b[13];
   c[4] = a[0] * b[4] + a[1] * b[8] + a[2] * b[11] + a[3] * b[13] + a[4] * b[14];
   c[5] = a[5] * b[0] + a[6] * b[1] + a[7] * b[2] + a[8] * b[3] + a[9] * b[4];
   c[6] = a[5] * b[1] + a[6] * b[5] + a[7] * b[6] + a[8] * b[7] + a[9] * b[8];
   c[7] = a[5] * b[2] + a[6] * b[6] + a[7] * b[9] + a[8] * b[10] + a[9] * b[11];
   c[8] = a[5] * b[3] + a[6] * b[7] + a[7] * b[10] + a[8] * b[12] + a[9] * b[13];
   c[9] = a[5] * b[4] + a[6] * b[8] + a[7] * b[11] + a[8] * b[13] + a[9] * b[14];
   c[10] = a[10] * b[0] + a[11] * b[1] + a[12] * b[2] + a[13] * b[3] + a[14] * b[4];
   c[11] = a[10] * b[1] + a[11] * b[5] + a[12] * b[6] + a[13] * b[7] + a[14] * b[8];
   c[12] = a[10] * b[2] + a[11] * b[6] + a[12] * b[9] + a[13] * b[10] + a[14] * b[11];
   c[13] = a[10] * b[3] + a[11] * b[7] + a[12] * b[10] + a[13] * b[12] + a[14] * b[13];
   c[14] = a[10] * b[4] + a[11] * b[8] + a[12] * b[11] + a[13] * b[13] + a[14] * b[14];
   c[15] = a[15] * b[0] + a[16] * b[1] + a[17] * b[2] + a[18] * b[3] + a[19] * b[4];
   c[16] = a[15] * b[1] + a[16] * b[5] + a[17] * b[6] + a[18] * b[7] + a[19] * b[8];
   c[17] = a[15] * b[2] + a[16] * b[6] + a[17] * b[9] + a[18] * b[10] + a[19] * b[11];
   c[18] = a[15] * b[3] + a[16] * b[7] + a[17] * b[10] + a[18] * b[12] + a[19] * b[13];
   c[19] = a[15] * b[4] + a[16] * b[8] + a[17] * b[11] + a[18] * b[13] + a[19] * b[14];
   c[20] = a[20] * b[0] + a[21] * b[1] + a[22] * b[2] + a[23] * b[3] + a[24] * b[4];
   c[21] = a[20] * b[1] + a[21] * b[5] + a[22] * b[6] + a[23] * b[7] + a[24] * b[8];
   c[22] = a[20] * b[2] + a[21] * b[6] + a[22] * b[9] + a[23] * b[10] + a[24] * b[11];
   c[23] = a[20] * b[3] + a[21] * b[7] + a[22] * b[10] + a[23] * b[12] + a[24] * b[13];
   c[24] = a[20] * b[4] + a[21] * b[8] + a[22] * b[11] + a[23] * b[13] + a[24] * b[14];

   return true;
}

