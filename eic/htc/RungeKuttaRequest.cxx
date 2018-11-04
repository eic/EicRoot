
#include <cassert>

#include <htclib.h>

#include <MediaBank.h>
#include <MediaSliceArray.h>

#include <KalmanNode.h>
#include <RungeKuttaRequest.h>

// Cash-Karp coefficients for Runge-Kutta 5-th order embedded algorithm; 
// well, prefer to avoid array creation since numerical recipes use 1-based 
// indices and eventually code would loose readability completely; 
       double a2 = 1./5., a3 = 3./10., a4 = 3./5., a5 = 1., a6 = 7./8.;

static double b21 =   1./5.;
static double b31 =   3./40., b32 =  9./40.;
static double b41 =   3./10., b42 = -9./10., b43 =   6./5.;
static double b51 = -11./54., b52 =  5./2.,  b53 = -70./27., b54 = 35./27.;

static double b61 =  1631./55296.,  b62 = 175./512., b63 = 575./13824.;
static double b64 = 44275./110592., b65 = 253./4096.;

static double c1 =  37./378.,     c2 = 0.,          c3 =   250./621.;
static double c4 = 125./594.,     c5 = 0.,          c6 =   512./1771.;

static double s1 = 2825./27648.,  s2 = 0.,          s3 = 18575./48384.;
static double s4 = 13525./55296., s5 = 277./14336., s6 =     1./4.;

// A hack variable to easily switch Runge-Kutta algorithm order; do it better later;
//unsigned forced_order;

/* ========================================================================== */

int RungeKuttaRequest::kk(MgridSlice *slice, double z, double x[4], double k[4])
{
  int ret = 0;
  //TVector3 xx = TVector3(x[0], x[1], z), BB;
  TVector3 xx(x[0], x[1], z), BB;
  double tx = x[2], ty = x[3], slope = sqrt(1.+SQR(tx)+SQR(ty));
  double qv = q*_LIGHT_SPEED_*1E-14;

  assert(slice);

  if (slice->mGrid)
  {
    // Repetition mode (derivatives) makes no sense if a former call 
    // at "nominal" values failed --> check on that; caused a day of 
    // debugging in Jan'2009;
    if (repetition_flag && slice->mLastUnboundCallStatus)
      ret = slice->mLastUnboundCallStatus;
    else
    {
      slice->mGrid->repetition_flag = repetition_flag;

      ret = slice->mGrid->getCartesianFieldValue(xx, BB);

      // Record last repetition_flag=0 call status at this Z;
      if (!repetition_flag) slice->mLastUnboundCallStatus = ret;
    } /*if*/
  } /*if*/

  // Yes, if no field or field routine failed, continue 
  // execution (show must go on!), but set a bit indicating that 
  // something bad happened;
  if (!slice->mGrid || ret) VZERO(BB);

  k[0] = h*tx;
  k[1] = h*ty;
  k[2] = h*qv*slope*( ty*(tx*BB[_X_]+BB[_Z_]) - (1.+SQR(tx))*BB[_Y_]);
  k[3] = h*qv*slope*(-tx*(ty*BB[_Y_]+BB[_Z_]) + (1.+SQR(ty))*BB[_X_]);

  return ret;
} /* RungeKuttaRequest::kk */

/* -------------------------------------------------------------------------- */

int RungeKuttaRequest::serveRequest(double in[], double _q, double out[], 
				    int _repetition_flag)
{
  int ret = 0;
  //RungeKutta *rk = rq->rk;
  double k1[4], k2[4], k3[4], k4[4], k5[4], k6[4], yy[4], bff[4];

  q               = _q;
  repetition_flag = _repetition_flag;

  // Start with k1[]; same for all 3 Runge-Kutta cases;
  if (kk(rk->m1, z0, in, k1))              ret = -1;

  // A hack for now; fix later;
  //unsigned order = forced_order ? forced_order : rk->_order;
  // Well, at least some safety check;
  //if (forced_order) assert(forced_order <= rk->_order);

  //switch (order)
  switch (rk->_order)
  {
  case _RK_ORDER_2_:
    // Then k2[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + k1[iq]/2.;
    if (kk(rk->m2, z0 + h/2., yy, k2)) ret = -1;

    // Sum up and obtain out[];
    for(int iq=0; iq<4; iq++)
      out[iq] = in[iq] + k2[iq];

    break;
  case _RK_ORDER_4_:
    // Then k2[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + k1[iq]/2.;
    if (kk(rk->m2, z0 + h/2., yy, k2)) ret = -1;

    // Then k3[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + k2[iq]/2.;
    if (kk(rk->m3, z0 + h/2., yy, k3)) ret = -1;

    // Then k4[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + k3[iq];
    if (kk(rk->m4, z0 + h,    yy, k4)) ret = -1;

    // Sum up and obtain out[];
    for(int iq=0; iq<4; iq++)
      out[iq] = in[iq] + k1[iq]/6. + k2[iq]/3. + k3[iq]/3. + k4[iq]/6.;

    break;
  case _RK_ORDER_5_:
    // Then k2[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + b21*k1[iq];
    if (kk(rk->m2, z0 + a2*h, yy, k2)) ret = -1;

    // Then k3[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + b31*k1[iq] + b32*k2[iq];
    if (kk(rk->m3, z0 + a3*h, yy, k3)) ret = -1;
    
    // Then k4[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + b41*k1[iq] + b42*k2[iq] + b43*k3[iq];
    if (kk(rk->m4, z0 + a4*h, yy, k4)) ret = -1;

    // Then k5[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + b51*k1[iq] + b52*k2[iq] + b53*k3[iq] + b54*k4[iq];
    if (kk(rk->m5, z0 + a5*h, yy, k5)) ret = -1;

    // Eventually k6[];
    for(int iq=0; iq<4; iq++)
      yy[iq] = in[iq] + b61*k1[iq] + b62*k2[iq] + b63*k3[iq] + b64*k4[iq] 
	+ b65*k5[iq];
    if (kk(rk->m6, z0 + a6*h, yy, k6)) ret = -1;
  
    // Sum up and obtain out[];
    for(int iq=0; iq<4; iq++)
      out[iq] = in[iq] + c1*k1[iq] + c2*k2[iq] + c3*k3[iq] + c4*k4[iq] + 
	c5*k5[iq] + c6*k6[iq];

#if _LATER_    
    // Same way get 4-th order estimate and consequently truncation error estimate;
    for(int iq=0; iq<4; iq++)
      bff[iq] = in[iq] + s1*k1[iq] + s2*k2[iq] + s3*k3[iq] + s4*k4[iq] + 
	s5*k5[iq] + s6*k6[iq];
    for(int iq=0; iq<4; iq++)
      rq->dlt[iq] = out[iq] - bff[iq];
#endif

    break;
  } /*switch*/

  return ret;
} /* RungeKuttaRequest::serveRequest */

/* ========================================================================== */

