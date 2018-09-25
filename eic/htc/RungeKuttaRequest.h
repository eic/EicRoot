
#ifndef _RUNGE_KUTTA_REQUEST_H
#define _RUNGE_KUTTA_REQUEST_H

class RungeKutta {
  friend class RungeKuttaRequest;
  friend class TrKalmanNode;
  friend class TrKalmanFilter;

 private:
  // 2-d, 4-th or 5-th order;
  int _order;

  // Will be set to '1' only if all the necessary m_i are Ok;
  //int initialization_flag;

  // Actual usage of these pointers depends on required number of 
  // slices in Z (which is 2,3,6 for Runge-Kutta 2-d,4-th & 5-th
  // order, respectively); the whole idea is that Runge-Kutta 
  // tracking requires field knowledge on a predefined set of 
  // XY-planes; once a grid is calculated there interpolation 
  // goes noticeably faster; mgrid usage is also required in 
  // general (even if official HRC map used), since it allows 
  // to fix used cells easily and thus guarantee continuity 
  // for F-matrix calculation procedure;
  MgridSlice *m1, *m2, *m3, *m4, *m5, *m6;
} ;

// It turns out to be fairly inconvenient to shuffle several parameters
// around; so split off code part which does not know anything about 
// Kalman filter, HERMES, etc and pass this frame as a general "request" 
// to perform Runge-Kutta step; remaining parameters to 
// serve_runge_kutta_request() are really the ones which differ when 
// calculating derivatives;
class RungeKuttaRequest {
  friend class TrKalmanNode;

 private:
  int kk(MgridSlice *slice, double z, double x[4], double k[4]);
  int serveRequest(double in[], double _q, double out[], int _repetition_flag);

  // Starting Z coordinate; step in Z; frame pointer to magnetic field slices;
  // these parameters are set by perform_runge_kutta_step() and do not change;
  double z0, h;
  RungeKutta *rk;

  // No sense to change self cell and cube during derivative calculation;
  int repetition_flag;
  // Q/|p|; both parameters are set by serve_runge_kutta_request() and do 
  // not change between numerous calls to kk() routine;
  double q;
} ;

#endif
