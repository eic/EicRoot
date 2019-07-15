//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Kalman filter task wrapper; since it just inherits everything from 
// PndRecoKalmanTask class, prefer to use a similar name for clarity;
//

#ifndef _EIC_RECO_KALMAN_TASK_
#define _EIC_RECO_KALMAN_TASK_

#include <PndRecoKalmanTask.h>
#include <EicIdealTrackingCode.h>

class EicRecoKalmanTask:  public PndRecoKalmanTask {
 public:
  EicRecoKalmanTask() {};
  // May want to set pion hypothesis by default?; otherwise use ideal hypo for now;
  //EicRecoKalmanTask(EicIdealTracker *ideal): PndRecoKalmanTask() { SetParticleHypo(211);};
  EicRecoKalmanTask(EicIdealTrackingCode *ideal);

  ~EicRecoKalmanTask() {};

  // Want to propagate detector group names to the Kalman filter initialization;
  InitStatus Init();
  // This one is not really needed;
  void Exec(Option_t* opt);

  void SetNumIterations(Int_t numIt) { fNumIt = numIt;};

private:
  EicIdealTrackingCode *fIdeal;

  ClassDef(EicRecoKalmanTask,2);
};

#endif
