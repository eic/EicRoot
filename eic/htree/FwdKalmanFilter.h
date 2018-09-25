//
// AYK (ayk@bnl.gov)
//
//  STAR-related forward tracker Kalman filter;
//

#include <KalmanFilter.h>

#ifndef _FWD_KALMAN_FILTER_
#define _FWD_KALMAN_FILTER_

#if _OFF_
class FwdKalmanFilter: public KalmanFilter 
{
 public:
 FwdKalmanFilter(unsigned sdim): KalmanFilter(sdim) {};
  ~FwdKalmanFilter() {};

  // These two are pure virtual in the base KalmanFilter class;
  int CalculateHMatrix(KalmanNode *node);
  int Transport(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode);

  // Prepare node to become a starting point of a filter pass (so 
  // basically reset x0[], xp[], CP[][] & mInversedMomentum);
  void ResetNode(KalmanNode *node, double S[], int assignmentMode);

 private:
};
#endif

#endif
