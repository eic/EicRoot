//
// AYK (ayk@bnl.gov)
//
//  STAR-related forward tracker Kalman filter;
//

#include <assert.h>

#include <FwdKalmanFilter.h>

#if _OFF_
// ---------------------------------------------------------------------------------------

int FwdKalmanFilter::CalculateHMatrix(KalmanNode *node)
{
  assert(0);


  //return 0;
} // FwdKalmanFilter::CalculateHMatrix()

// ---------------------------------------------------------------------------------------

int FwdKalmanFilter::Transport(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode)
{
  // For now (magnet off case) is not needed;
  assert(0);
  //return 0;
} // FwdKalmanFilter::Transport()

// ---------------------------------------------------------------------------------------

#if 0
#define _FLYSUB_TYPICAL_XY_COORD_ERROR_  (  0.010)
#define _FLYSUB_TYPICAL_SLOPE_ERROR_     (  0.0001)
//#define _FLYSUB_TYPICAL_XY_COORD_ERROR_  (  10.000)
//#define _FLYSUB_TYPICAL_SLOPE_ERROR_     (  1.)
#define _FLYSUB_TYPICAL_MOMENTUM_ERROR_  (  1.000)
#define _FLYSUB_COVARIANCE_BLOWUP_CFF_        (30)
#endif

void FwdKalmanFilter::ResetNode(KalmanNode *node, double S[], int assignmentMode)
{
  assert(0);

#if _NOW_
  unsigned sdim = mFieldMode == NoField ? 4 : 5;

  if (S)
    for(int ip=_X_; ip<sdim; ip++)
      node->x0->KFV(ip) = S[ip];

  // Initialization depends on whether it's a 0-th iteration or not;
  if (assignmentMode != _USE_00_) {
    // Otherwise a normal Kalman filter iterative update;
    KfVector *add = assignmentMode == _USE_XF_ ? node->xf : node->xs;

    for(int ip=_X_; ip<=_SY_; ip++)
      node->x0->KFV(ip) += add->KFV(ip);

    // Momentum expansion point;
    // NB: head->x0[_QP_] will remain 0. in all cases!;
    if (sdim == 5) node->mInversedMomentum += add->KFV(_QP_);
  } //if

  // Yes, predicted state vector at start of chain is set to 0.0;
  for(int ip=_X_; ip<sdim; ip++)
    node->xp->KFV(ip) = 0.;

  // Cook dummy (diagonal) covariance matrix;
  node->CP->Reset();
  // Just [0..3] components;
  for(int ip=_X_; ip<=_SY_; ip++) {
    double diag;

    switch (ip) {
    case _X_:;
    case _Y_:
      diag = _FLYSUB_TYPICAL_XY_COORD_ERROR_;
      break;
    case _SX_:;
    case _SY_:
      diag = _FLYSUB_TYPICAL_SLOPE_ERROR_;
      break;
    default:
      assert(0);
    } //switch
	
    node->CP->KFM(ip,ip) = SQR(diag*_FLYSUB_COVARIANCE_BLOWUP_CFF_);
  } //for ip

  if (sdim == 5)
    node->CP->KFM(_QP_,_QP_) = 
      SQR(_FLYSUB_TYPICAL_MOMENTUM_ERROR_*(node->mInversedMomentum)*
	  _FLYSUB_COVARIANCE_BLOWUP_CFF_);
#endif
} // FwdKalmanFilter::ResetNode()
#endif
// ---------------------------------------------------------------------------------------
