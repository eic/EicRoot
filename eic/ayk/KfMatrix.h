//
// AYK (ayk@bnl.gov), cleaned up 2014/10/14;
//
//    Yet another matrix package; using ROOT matrices does not help
//  to easily improve performance (FIXME: should try to replace 
//  per-element access by hopefully optimized ROOT matrix operations);
//  

//#define _USE_ROOT_TMATRIX_

#ifdef _USE_ROOT_TMATRIX_
#include <TMatrixD.h>
#endif

#ifndef _KF_MATRIX_H
#define _KF_MATRIX_H

#define _PRINTOUT_FORMAT_DEFAULT_ ("%12.8f ")

// General matrix structure; 
class KfMatrix {
 public:
  // Will work for both "matrix" and "vector" types;
  KfMatrix(unsigned dim1, unsigned dim2 = 1);
  ~KfMatrix();

  static void SetDimensionCheckFlag(bool what) { mDimensionCheckFlag = what; };
  static bool GetDimensionCheckFlag()          { return mDimensionCheckFlag; };

  // Trivial routines to access matrix dimension values;
  int GetDim1() const { return mDim1; };
  int GetDim2() const { return mDim2; };

  // Matrix element access; try to minimize changes to the original C code
  // where KFV() and KFM() were preprocessor macros which allowed to be
  // used both as lvalue and rvalue -> use references here; this indeed 
  // compromises encapsulation, however allows to keep access under certain
  // control, and also sould with minimal changes work for other matrix 
  // implementation types;
#ifdef _USE_ROOT_TMATRIX_
  double & KFV(int ip)                { return  (*mMtx)[ip][ 0]; };
  double & KFM(int ip, int iq)        { return  (*mMtx)[ip][iq]; };
  double * ARR()                      { return &(*mMtx)[ 0][ 0]; };
  TMatrixD &Vec()                     { return *mMtx; }
  TMatrixD &Mtx()                     { return *mMtx; }
#else
  double & KFV(int ip)                { return mArr[ip]; };
  double & KFM(int ip, int iq)        { return mArr[ip*mDim2+iq]; };
  // Leave this separate instead of using &KFV(0), just as a sign, that 
  // sequential access to data array is in fact needed in a few places;
  // can be cured if eg GSL matrix library is used;
  double * ARR()                      { return mArr; };
#endif
  
  // Reset to a unity (square) matrix; 
  int Unity();
  // Reset all matrix elements to 0.0;
  void Reset();

  enum MtxType {Symmetric, Arbitrary};
  // Square matrix invertion ('type' is either _SYMMETRIC_ or _ARBITRARY_);
  int Invert(MtxType type = Arbitrary);

  // Forces square matrix symmetrization (just fills out both lower an upper 
  // triangles with half-sum);
  int ForceSymmetric();

  // Returns '-1' if a square matrix does not pass positivity check;
  int CheckPositivity();
  int FixPositivity(double maxFixablePositivityScrewup, double positivityCorrelationFix);

  // Print out matrix contents in a human-readable format; NB: if format pointer
  // is NULL, _PRINTOUT_FORMAT_DEFAULT_ is used;
  void Print(const char *fmt = _PRINTOUT_FORMAT_DEFAULT_);
  // Prints out matrix correlation coefficients; 
  int CorrelationPrint(const char *fmt = _PRINTOUT_FORMAT_DEFAULT_);

  int CopyFrom       (KfMatrix *in);
  int Add            (KfMatrix *in);
  int Subtract       (KfMatrix *in);

  int SetToSum       (KfMatrix *in1, KfMatrix *in2);
  int SetToDifference(KfMatrix *in1, KfMatrix *in2);
  int SetToProduct   (double cff,    KfMatrix *in);
  int SetToProduct   (KfMatrix *in1, KfMatrix *in2,                unsigned mode = 0x0000);
  int SetToProduct   (KfMatrix *in1, KfMatrix *in2, KfMatrix *in3, unsigned mode = 0x0000);

  // This routine works only for KfVector objects of course;
  int VectorLengthSquare(KfMatrix *metric, double *lsqr); 

 private:
  // 1-st & 2-d dimensions; mDim = mDim1 * mDim2;
  int mDim1, mDim2, mDim;

#ifdef _USE_ROOT_TMATRIX_
  TMatrixD *mMtx;
#else
  // Data array;
  double *mArr;
#endif

  static bool mDimensionCheckFlag;

  static int KfmReturn(int ret);

  // NB: dim[] array is filled by dim1 & dim2 values depending on whether
  // transposition is needed or not;
  void AssignTrueDimensions(int trans, int dim[2]);

  int AddCore         (KfMatrix *in1, KfMatrix *in2, int what);
  int SetToProductCore(KfMatrix *in1, KfMatrix *in2, KfMatrix *in3, unsigned mode = 0x0000);

  int DimensionMatchCheck(KfMatrix *kfm2);
  int DimensionChainCheck(KfMatrix *in1, KfMatrix *in2, unsigned mode);

  KfMatrix *Bufferize();
};

// 'mode' bitwise pattern; NB: never change these values (bitwise shifts 
// _TRANSPOSE_IN3_ -> _TRANSPOSE_IN2_ used, etc);
#define _TRANSPOSE_IN1_        0x001
#define _TRANSPOSE_IN2_        0x002
#define _TRANSPOSE_IN3_        0x004

// Yes, it is easier to work always with 2-dim stuff; if dim2=1 it is 
// however more legible to emphasise on this fact;
typedef KfMatrix KfVector;

#endif
