//
// AYK (ayk@bnl.gov)
//
//    Forward detector tracking application of the Kalman filter stuff; 
//  ported from HERMES/OLYMPUS sources; cleaned up 2014/10/17;
//

#include <cassert>
#include <cstdlib>

#include <htclib.h>

#include <MediaBank.h>
#include <MediaSliceArray.h>
#include <TrKalmanNode.h>

// 1/p will be varied by this fraction; FIXME: do it better later;
#define _PSTEP_ (0.001)

// =======================================================================================

int TrKalmanNode::PerformRungeKuttaStep(KalmanFilter::Direction fb, double pout[], double rkd[5][5])
{
  // Shape up request frame;
  RungeKuttaRequest rq;
  KalmanNode *to = GetNext(fb);

  // These parameters will not change;
  rq.z0 = GetZ();
  rq.h  = to->GetZ() - GetZ();
  rq.rk = mLocation->mRungeKutta + fb;

  {
    double in[5], q0 = mInversedMomentum;
    int ret = rq.serveRequest(x0->ARR(), q0, pout, 0);

    // Otherwise derivatives are of no interest;
    if (rkd)
    {
      // Calculate drv_steps[4] by hand;
      double drv_steps_qp = fabs(q0)*_PSTEP_;

      // Calculate derivatives vs <1/p>;
      if (rq.serveRequest(x0->ARR(), q0 + drv_steps_qp, rkd[4], 1)) ret = -1;

      for(int ip=0; ip<4; ip++)
      {
	for(int iq=0; iq<4; iq++)
	  in[iq] = GetX0(iq) + (ip == iq ? _drv_steps[iq] : 0.0);

	if (rq.serveRequest(in, q0, rkd[ip], 1)) ret = -1;
      } /*for iq*/
      
      for(int ip=0; ip<5; ip++)
	for(int iq=0; iq<4; iq++)
	  {
	    rkd[ip][iq] -= pout     [iq];
	    rkd[ip][iq] /= (ip == 4 ? drv_steps_qp : _drv_steps[ip]);
	  } /*for ip..iq*/
    } /*if*/

    // Yes, return 0 in all cases; just set field_missing_flags[] in case
    // of problems;
    return 0;
  }
} // TrKalmanNode::PerformRungeKuttaStep()

// ---------------------------------------------------------------------------------------

int TrKalmanNode::GetMagneticField(double xy[2], TVector3 &B)
{
  TVector3 xx(xy[0], xy[1], GetZ());
  // Does not matter which one; take KalmanFilter::Forward;
  RungeKutta *rk = mLocation->mRungeKutta + KalmanFilter::Forward;

  // Put a lousy fix here; figure out what's wrong later;
  if (rk && rk->m1)
    return rk->m1->mGrid->getCartesianFieldValue(xx, B);
  else {
    VZERO(B);
    return -1;
  } //if
} // TrKalmanNode::GetMagneticField()
// ---------------------------------------------------------------------------------------

void TrKalmanNode::InflateMeasurementNoise(double scale)
{
  for(unsigned iq=0; iq<mDim; iq++)
    V->KFM(iq, iq) *= pow(scale, 2);
} // TrKalmanNode::InflateMeasurementNoise()

// =======================================================================================
