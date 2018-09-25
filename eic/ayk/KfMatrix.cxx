//
// AYK (ayk@bnl.gov), cleaned up 2014/10/14;
//
//    Yet another matrix package; 
//  

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>

#ifndef _USE_ROOT_TMATRIX_ 
#include <TMatrixD.h>
#include <TArrayD.h>
#endif

#include <ayk.h>
#include <KfMatrix.h>

// Well, this code is explicitely written to be used inside     
// generalized Kalman filter routines; therefore I don't really 
// care how would it work in a different environment; in Kalman 
// filter application there is a very limited set of matrix     
// dimensions used, so it makes perfect sense to create a       
// persistent chain of buffer matrices; in other programs where 
// various dimensions may be used, this can be terribly RAM     
// consuming and inefficient;                                   
struct t_kf_matrix_chain {
  // Pointer to the actual matrix structure;
  KfMatrix *kfm;

  // This buffer matrix may be in use; this flag must be reset       
  // upon every *external* entry; internally if KFM routines call   
  // each other they should pass an extra argument in order to       
  // avoid cleaning up (for instance triple multiplication uses      
  // calls to dual one); cleaning BUSY flags should be avoided in the 
  // latter;                                                          
  int busy_flag;

  t_kf_matrix_chain *next;
};

static t_kf_matrix_chain *buffer, **btail = &buffer;

// Well, for a debugged code when it is known that all matrix    
// dimensions match, makes no sense to spend CPU time on checks; 
// so this flag may be set to 0 (see respective method);            
bool KfMatrix::mDimensionCheckFlag = true;

// =======================================================================================
//  Well, it is nasty to always check return codes of Kfm*() routines; makes 
// sense to check that dimensions match during debugging and then turn        
// checks off; just assert() suffices then; 

int KfMatrix::KfmReturn(int ret)
{
  if (ret && mDimensionCheckFlag) assert(0);

  return ret;
} // KfMatrix::KfmReturn()

// =======================================================================================
// It will also perform intialization to 0.;                                 

//
//  Should later somehow mark success/failure here (can not return values
//  in this - constructor - call);
//

KfMatrix::KfMatrix(unsigned dim1, unsigned dim2)
{
  memset(this, 0x00, sizeof(KfMatrix));

  mDim1 = dim1;
  mDim2 = dim2;
  mDim  = mDim1*mDim2;

#ifdef _USE_ROOT_TMATRIX_
  // CHECK: row/column switch?;
  mMtx = new TMatrixD(mDim1, mDim2);
  // CHECK: really needed?;
  mMtx->Zero();
#else
  mArr  = new double[mDim];
  memset(mArr, 0x00, mDim*sizeof(double));
#endif
} // KfMatrix::KfMatrix()

// ---------------------------------------------------------------------------------------

KfMatrix::~KfMatrix()
{
#ifdef _USE_ROOT_TMATRIX_
  if (mMtx) delete mMtx;
#else
  if (mArr) delete[] mArr;
#endif
} // KfMatrix::~KfMatrix()

// =======================================================================================
//  This is a simple two matrix dimension check for add/subtract/scale ops;   

int KfMatrix::DimensionMatchCheck(KfMatrix *kfm2)
{
  // Sanity check; 
  if (!kfm2) return -1;

  if (GetDim1() != kfm2->GetDim1() || GetDim2() != kfm2->GetDim2()) return -1;

  return 0;
} // KfMatrix::DimensionMatchCheck()

// ---------------------------------------------------------------------------------------

void KfMatrix::AssignTrueDimensions(int trans, int dim[2])
{
  // NB: exact contents of 'trans' bits is of no interest (just either 0 or not);
  if (trans) {
    dim[0] = mDim2;
    dim[1] = mDim1;
  }
  else {
    dim[0] = mDim1;
    dim[1] = mDim2;
  } //if
} // KfMatrix::AssignTrueDimensions()

// ---------------------------------------------------------------------------------------
//  This check is for matrix multiplication; it is more complicated since     
// either of in1/in2 may be transposed;                                       

int KfMatrix::DimensionChainCheck(KfMatrix *in1, KfMatrix *in2, unsigned mode)
{
  int din1[2], din2[2];

  // Sanity check; 
  if (!in1 || !in2) return -1;

  // Assign 'true' dimensions of in1/in2 depending on whether 
  // their transposition will take place;                     
  in1->AssignTrueDimensions(mode & _TRANSPOSE_IN1_, din1);
  in2->AssignTrueDimensions(mode & _TRANSPOSE_IN2_, din2);

  // Clear, neighboring matrices should have matching dimensions; 
  if (mDim1 != din1[0] || din1[1] != din2[0] || din2[1] != mDim2) return -1;

  return 0;
} // KfMatrix::DimensionChainCheck()

// =======================================================================================

//
// Do not mind to have these routines (and linked list) to be static; 
// FIXME: may want to do this more elegant later;
//

static void ResetBusyFlags( void )
{
  t_kf_matrix_chain *mring;

  for(mring=buffer; mring; mring=mring->next)
    mring->busy_flag = 0;
} // ResetBusyFlags()

// ---------------------------------------------------------------------------------------

static KfMatrix *GetBufferMatrix(int dim1, int dim2)
{
  t_kf_matrix_chain *mring;

  // Loop through already available matrices;
  for(mring=buffer; mring; mring=mring->next) {
    if (mring->busy_flag) continue;

    // Appropriate matrix found (and is free!) --> break; 
    if (mring->kfm->GetDim1() == dim1 && mring->kfm->GetDim2() == dim2) break;
  } //for

  // No appropriate (and free!) matrix found --> allocate new; 
  if (!mring) {
    mring = *btail = new t_kf_matrix_chain();
    mring->kfm = new KfMatrix(dim1, dim2);

    btail = &mring->next;
  } //if

  mring->busy_flag = 1;

  return mring->kfm;
} // GetBufferMatrix()

// ---------------------------------------------------------------------------------------

KfMatrix *KfMatrix::Bufferize()
{
  KfMatrix *buffer = GetBufferMatrix(GetDim1(), GetDim2());

  if (!buffer) return NULL;
  
  // Since dimensions match, this can not fail; 
  buffer->CopyFrom(this);

  return buffer;
} // KfMatrix::Bufferize()

// =======================================================================================

void KfMatrix::Reset()
{
#ifdef _USE_ROOT_TMATRIX_
  mMtx->Zero();
#else
  for(int ii=0; ii<mDim1; ii++)
    for(int ik=0; ik<mDim2; ik++)
      KFM(ii,ik) = 0.0;
#endif
} // KfMatrix::Reset()

// ---------------------------------------------------------------------------------------

int KfMatrix::Unity()
{
  if (mDim1 != mDim2) return KfmReturn(-1);

  for(int ii=0; ii<mDim1; ii++)
    for(int ik=0; ik<mDim1; ik++)
      KFM(ii,ik) = ii == ik ? 1. : 0.;

  return 0;
} // KfMatrix::Unity()

// =======================================================================================

int KfMatrix::CopyFrom(KfMatrix *in)
{
  if (!in) return KfMatrix::KfmReturn(-1);

  // Well, and no error?; 
  if (in == this) return 0;

  if (GetDimensionCheckFlag() && DimensionMatchCheck(in)) return KfmReturn(-1);
 
#ifdef _USE_ROOT_TMATRIX_                      
  *mMtx = *in->mMtx; 
#else
  for(unsigned ii=0; ii<in->mDim1; ii++)
    for(unsigned ik=0; ik<in->mDim2; ik++) 
      KFM(ii,ik) = in->KFM(ii,ik);
#endif

  return 0;
} // KfMatrix::CopyFrom()

// ---------------------------------------------------------------------------------------
// NB: buffering is not needed here (even if in1=in2==out, will work);    

// These 2 numbers are used in internal operations only; 
#define _KF_MATRIX_ADD_       0
#define _KF_MATRIX_SUBTRACT_  1

int KfMatrix::AddCore(KfMatrix *in1, KfMatrix *in2, int what)
{
  if (GetDimensionCheckFlag() && 
      (DimensionMatchCheck(in1) || DimensionMatchCheck(in2)))
    return KfmReturn(-1);

#ifdef _USE_ROOT_TMATRIX_ 
  if (what == _KF_MATRIX_ADD_)
    *mMtx = *in1->mMtx + *in2->mMtx;
  else
    *mMtx = *in1->mMtx - *in2->mMtx;
#else
  // Avoid additional complication --> two separate branches; 
  if (what == _KF_MATRIX_ADD_)
    for(unsigned ii=0; ii<in1->mDim1; ii++)
      for(unsigned ik=0; ik<in1->mDim2; ik++)
	KFM(ii,ik) = in1->KFM(ii,ik) + in2->KFM(ii,ik);
  else
    for(unsigned ii=0; ii<in1->mDim1; ii++)
      for(unsigned ik=0; ik<in1->mDim2; ik++)
	KFM(ii,ik) = in1->KFM(ii,ik) - in2->KFM(ii,ik);
#endif

  return 0;
} // KfMatrix::AddCore()

// ---------------------------------------------------------------------------------------

int KfMatrix::SetToSum(KfMatrix *in1, KfMatrix *in2)
{
  return AddCore(in1, in2, _KF_MATRIX_ADD_);
} // KfMatrix::SetToSum()

// ---------------------------------------------------------------------------------------

int KfMatrix::SetToDifference(KfMatrix *in1, KfMatrix *in2)
{
  return AddCore(in1, in2, _KF_MATRIX_SUBTRACT_);
} // KfMatrix::SetToDifference()

// ---------------------------------------------------------------------------------------

//
// It seems there is no real need to optimize these two?;
//

int KfMatrix::Add(KfMatrix *in)
{
  return AddCore(this, in, _KF_MATRIX_ADD_);
} // KfMatrix::Add()

// ---------------------------------------------------------------------------------------

int KfMatrix::Subtract(KfMatrix *in)
{
  return AddCore(this, in, _KF_MATRIX_SUBTRACT_);
} // KfMatrix::Subtract()

// =======================================================================================
// NB: buffering is not needed here (even if in=out, will work);          

int KfMatrix::SetToProduct(double cff, KfMatrix *in)
{
  if (GetDimensionCheckFlag() && DimensionMatchCheck(in)) return KfmReturn(-1);

  // Perform scalar multiplication; 
#ifdef _USE_ROOT_TMATRIX_ 
  // Has not been checked;
  assert(0);
  *mMtx = cff * *in->mMtx;
#else
  for(unsigned ii=0; ii<in->mDim1; ii++)
    for(unsigned ik=0; ik<in->mDim2; ik++)
      KFM(ii,ik) = cff*in->KFM(ii,ik);
#endif

  return 0;
} // KfMatrix::SetToProduct()

// ---------------------------------------------------------------------------------------

int KfMatrix::SetToProduct(KfMatrix *in1, KfMatrix *in2, unsigned mode)
{
  // Reset BUSY flags on buffer matrices (user entry point!); 
  ResetBusyFlags();

  unsigned trans1 = mode & _TRANSPOSE_IN1_, trans2 = mode & _TRANSPOSE_IN2_;
  // Since buffering of either in1 or in2 may be possible, introduce safe 
  // source pointers, which are just in1/2 if buffering is not needed;    
  KfMatrix *tin1 = in1, *tin2 = in2;
  
  // Do the check this way even if 'out' may coincide with either 'in1' or 'in2'; 
  if (KfMatrix::GetDimensionCheckFlag() && DimensionChainCheck(in1, in2, mode)) 
    KfMatrix::KfmReturn(-1);

#ifdef _USE_ROOT_TMATRIX_ 
  assert(!(trans1 && trans2));
  switch (mode) {
  case 0:
    *mMtx = *in1->mMtx * *in2->mMtx;
    break;
  case _TRANSPOSE_IN1_:
    *mMtx = TMatrixD(TMatrixD::kTransposed, *in1->mMtx) * *in2->mMtx;
    break;
  case _TRANSPOSE_IN2_:
    *mMtx = *in1->mMtx * TMatrixD(TMatrixD::kTransposed, *in2->mMtx);
    break;
  } //switch
#else
  // Figure out whether buffering is needed; 
  if (this == in1) tin1 = in1->Bufferize();
  // This is also more or less clear (avoid double buffering); 
  if (this == in2) tin2 = (in2 == in1) ? tin1 : in2->Bufferize();

  // Set output matrix to 0.; 
  Reset();
  // And eventually calculate it; 'qdim' is a bit of a subtlety; 
  int qdim = trans1 ? tin1->mDim1 : tin1->mDim2;

  for(int ii=0; ii<mDim1; ii++)
    for(int ip=0; ip<qdim; ip++)
      for(int ik=0; ik<mDim2; ik++)
      {
	double val1 = trans1 ? tin1->KFM(ip,ii) : tin1->KFM(ii,ip);
	double val2 = trans2 ? tin2->KFM(ik,ip) : tin2->KFM(ip,ik);

	KFM(ii,ik) += val1*val2;
      } /*for..for*/
#endif

  return 0;
} // KfMatrix::SetToProduct()

// ---------------------------------------------------------------------------------------

int KfMatrix::SetToProductCore(KfMatrix *in1, KfMatrix *in2, KfMatrix *in3, unsigned mode)
{
  int ret, nl, nr, dim1, dim2, dim3, dim4;
  unsigned trans1 = mode & _TRANSPOSE_IN1_;
  unsigned trans2 = mode & _TRANSPOSE_IN2_;
  unsigned trans3 = mode & _TRANSPOSE_IN3_;
  KfMatrix *buffer;

  // Estimate number of operations needed to calculate ABC as (AB)C 
  // or A(BC) in order to minimize CPU losses; assume dimensions   
  // do match (otherwise multiplication will fail later anyway);     
  // a proper choice can drastically improve performance;           
  dim1 = mDim1; dim2 = trans1 ? in1->mDim1 : in1->mDim2; 
  dim3 = trans2 ? in2->mDim1 : in2->mDim2; dim4 = mDim2;
  nl = dim1*dim2*dim3 + dim1*dim3*dim4;
  nr = dim2*dim3*dim4 + dim1*dim2*dim4;

  if (nl < nr) {
    // Request buffer matrix;
    buffer = GetBufferMatrix(trans1 ? in1->mDim2 : in1->mDim1, 
				    trans2 ? in2->mDim1 : in2->mDim2);
    if (!buffer) return KfMatrix::KfmReturn(-1);

    // Perform first multiplication; 
    ret = buffer->SetToProduct(in1, in2, trans1 | trans2);
    if (ret) return ret;

    // Perform second multiplication and return; 
    // NB: indeed 'buffer' is not transposed;    
    return SetToProduct(buffer, in3, trans3 >> 1);
  }
  else {
    // Request buffer matrix; 
    buffer = GetBufferMatrix(trans2 ? in2->mDim2 : in2->mDim1, 
				    trans3 ? in3->mDim1 : in3->mDim2);
    if (!buffer) return KfMatrix::KfmReturn(-1);

    // Perform first multiplication; 
    ret = buffer->SetToProduct(in2, in3, (trans2 | trans3) >> 1);
    if (ret) return ret;

    // Perform second multiplication and return; 
    return SetToProduct(in1, buffer, trans1);
  } //if
} // KfMatrix::SetToProductCore()

// ---------------------------------------------------------------------------------------

//
// NB: both SetToProduct() and SetToProductCore() are needed, since otherwise
// VectorLengthSquare() will call ResetBusyFlags() twice;
//

int KfMatrix::SetToProduct(KfMatrix *in1, KfMatrix *in2, KfMatrix *in3, unsigned mode)
{
  // Reset BUSY flags on buffer matrices (user entry point!); 
  ResetBusyFlags();

  return SetToProductCore(in1, in2, in3, mode);
} // KfMatrix::SetToProduct()

// ---------------------------------------------------------------------------------------
//  This routine  is not too much efficient in the easiest case of MDIM=1;    

int KfMatrix::VectorLengthSquare(KfMatrix *metric, double *lsqr) 
{
  // Reset BUSY flags on buffer matrices (user entry point!); 
  ResetBusyFlags();

  KfMatrix *value = GetBufferMatrix(1, 1);

  int ret = value->SetToProductCore(this, metric, this, _TRANSPOSE_IN1_);
  if (ret) return ret;
  
  if (lsqr) *lsqr = value->KFM(0,0);
  
  return 0;
} // KfMatrix::VectorLengthSquare()

// =======================================================================================

int KfMatrix::Invert(KfMatrix::MtxType type) 
{   
  if (mDim1 != mDim2) return KfmReturn(-1);

  // If mdim=1, take an easy way;
  if (mDim1 == 1) {
    // Well, symmetric actually means also "positively definite";
    //NB: return code of this routine is always checked in my code;
    // so no kfm_return() needed;
    if (type == KfMatrix::Symmetric && KFM(0,0) <= 0.) return -1;

    KFM(0,0) = 1./KFM(0,0);

    return 0;
  }
  else
    // Well, for mdim=2 also do not need to introduce extra complications;
    // NB: this code has not yet been checked!;
  if (mDim1 == 2)
  {
    // Order 0..1 does not matter here, but should be in sync with 
    // assignment back few lines below;
    double a[2][2] = {{KFM(0,0), KFM(0,1)}, {KFM(1,0), KFM(1,1)}};
    double det = a[0][0]*a[1][1] - a[0][1]*a[1][0];

    if (!det) return -1;

    // No positivity check?;
    if (type == KfMatrix::Symmetric && a[0][1] != a[1][0]) return -1;

    KFM(0,0) =  a[1][1]/det;
    KFM(0,1) = -a[0][1]/det;
    KFM(1,0) = -a[1][0]/det;
    KFM(1,1) =  a[0][0]/det;
    
    return 0;
  }
  else {
    // FIXME: why does not it work for dim=1,2?!;
#ifdef _USE_ROOT_TMATRIX_ 
    mMtx->Invert();
#else
    // Well, want to avoid CERNLIB linkage alltogether; standard 
    // KF usage does not require high dimension matrix inversion, 
    // so do not care much about performance here;
    TArrayD arr(mDim1*mDim1);

    // Think about the [ip,iq] order here!;
    for(int ip=0; ip<mDim1*mDim1; ip++)
      arr[ip] = ARR()[ip];

    TMatrixD bff; bff.Use(mDim1, mDim1, arr.GetArray());
    
    bff.Invert();
    
    for(int ip=0; ip<mDim1*mDim1; ip++)
      ARR()[ip] = arr[ip];
#endif

    return 0;
  } //if
} // KfMatrix::Invert()

// =======================================================================================

void KfMatrix::Print(const char *fmt)
{
  printf("\n");
  for(int ip=0; ip<mDim1; ip++) {
    for(int iq=0; iq<mDim2; iq++)
      printf(fmt, KFM(ip,iq));
    printf("\n");
  } //for ip
} // KfMatrix::Print()

// ---------------------------------------------------------------------------------------

int KfMatrix::CorrelationPrint(const char *fmt)
{
  if (mDim1 != mDim2) return KfmReturn(-1);

  // Ok, just "return -1" here?;
  for(int ip=0; ip<mDim1; ip++)
    if (KFM(ip,ip) <= 0.) {
      printf("Non-positive diagonal element: %20.15f\n", KFM(ip,ip));
      return -1;
    } //for ip .. if

  printf("\n");
  for(int ip=0; ip<mDim1; ip++) {
    for(int iq=0; iq<mDim2; iq++)
      printf(fmt, KFM(ip,iq)/sqrt(KFM(ip,ip)*KFM(iq,iq)));
    printf("\n");
  } //for ip

  return 0;
} // KfMatrix::CorrelationPrint()

// =======================================================================================

int KfMatrix::ForceSymmetric()
{
  if (mDim1 != mDim2) return KfmReturn(-1);

  for(int ip=0; ip<mDim1; ip++)
    for(int iq=0; iq<mDim1; iq++) {
      if (ip == iq) continue;

      KFM(ip,iq) = KFM(iq,ip) = (KFM(ip,iq) + KFM(iq,ip))/2.;
    } //for ip..iq

  return 0;
} // KfMatrix::ForceSymmetric()

// ---------------------------------------------------------------------------------------
//  Well, this is a "naive" positivity check indeed;                                 

int KfMatrix::CheckPositivity()
{
  if (mDim1 != mDim2) return KfmReturn(-1);

  for(int ip=0; ip<mDim1; ip++)
    if (KFM(ip,ip) <= 0.) return -1;

  for(int ip=0; ip<mDim1; ip++)
    for(int iq=0; iq<mDim1; iq++) {
      if (ip == iq) continue;

      if (SQR(KFM(ip,iq)) > KFM(ip,ip)*KFM(iq,iq)) return -1;
    } //for ip..iq

  return 0;
} // KfMatrix::CheckPositivity()

// ---------------------------------------------------------------------------------------

int KfMatrix::FixPositivity(double maxFixablePositivityScrewup, 
			    double positivityCorrelationFix)
{
  // Well, cases with negative diagonal elements are fatal I guess;
  for(int ip=0; ip<GetDim1(); ip++)
    if (KFM(ip,ip) <= 0.) return -1;

  // Just rescale "too big" off-diagonal elements;
  for(int ip=0; ip<GetDim1(); ip++)
    for(int iq=0; iq<GetDim1(); iq++) {
      if (ip == iq) continue;

      double corr  = SQR(KFM(ip,iq));
      double limit = KFM(ip,ip)*KFM(iq,iq);

      // Then this coefficient requires a fix;
      if (corr > limit) {
	// Too off; something is terribly wrong; give up;
	if (corr > maxFixablePositivityScrewup) {
	  //if (_verbosityLevel >= 3)
	  //printf("Necessary correction (%7.2f) is higher than alllowed limit (%7.2f)\n",  
	  //	     corr, limit);
	  return -1;
	} //if
	
	  // Othewise apply a fix; 
	KFM(ip,iq) = (KFM(ip,iq) > 0. ? 1. : -1.)*
	  sqrt(limit*positivityCorrelationFix);
      } //if
    } //for ip..iq
 
  // Be on a safe side; but this call should now really return 0;
  return CheckPositivity();
} // FixPositivity()

// =======================================================================================
