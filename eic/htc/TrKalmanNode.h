//
// AYK (ayk@bnl.gov)
//
//    Forward detector tracking application of the Kalman filter stuff; 
//  ported from HERMES/OLYMPUS sources; cleaned up 2014/10/17;
//

#include <EicTrackingDigiHit.h>

#include <KalmanFilter.h>
#include <TrKalmanNodeLocation.h>

class SensitiveVolume;
class EicTrackingDigiHit;
struct t_particle_group;

#ifndef _TR_KALMAN_NODE_
#define _TR_KALMAN_NODE_

enum MfieldMode {WithField, NoField};

class TrKalmanNode: public KalmanNode {
  // NB: in particular want TrKalmanFilter class to be able to use 
  // protected members of KalmanNode class;
  friend class TrKalmanFilter;

public:
 TrKalmanNode(): mInversedMomentum(0.0), mHit(0), mLocation(0), mActive(true), mSensitiveVolume(0) {};

  // No validity check?;
  void SetMomentum(double value)                   { mInversedMomentum = 1./value; }; 
  double GetInversedMomentum()               const { return mInversedMomentum; }; 
  void UpdateInversedMomentum(double value)        { mInversedMomentum += value; };

  int SetMeasurementNoise(KfMatrix *W)             { V->CopyFrom(W); return 0; };
  void InflateMeasurementNoise(double scale);
  
  // Get magnetic field on an XY-plane of this node (orthogonal to KF Z axis);
  int GetMagneticField(double xy[2], TVector3 &B);

  // Assign hit pointer for this node;
  void SetHit(EicTrackingDigiHit *hit)             { mHit = hit; };
  bool HasHit()                              const { return mHit; };

  double GetZ()                              const { return mZ; };

  void SetLocation(TrKalmanNodeLocation *location) { mLocation = location; };
  TrKalmanNodeLocation *GetLocation()        const { return mLocation; };

  void SetActiveFlag(bool flag)                    { mActive = flag; };
  bool IsActive()                            const { return mActive; };

  void SetSensitiveVolume(SensitiveVolume *sv)     { mSensitiveVolume = sv; };
  SensitiveVolume *GetSensitiveVolume()      const { return mSensitiveVolume; };

  //private:
  // Inversed momentum;
  double mInversedMomentum;

  EicTrackingDigiHit *mHit;

  TrKalmanNodeLocation *mLocation;

  SensitiveVolume *mSensitiveVolume;

  // If 'true', this node will be included in the linked list in kf->BuildNodeList();
  bool mActive;

  TrKalmanNode *GetPrev(KalmanFilter::Direction fb) { 
    return static_cast <TrKalmanNode *> (KalmanNode::GetPrev(fb));
  };
  TrKalmanNode *GetNext(KalmanFilter::Direction fb) {
    return static_cast <TrKalmanNode *> (KalmanNode::GetNext(fb));
  };

  int PerformRungeKuttaStep(KalmanFilter::Direction fb, double pout[], double rkd[5][5]);
};

#endif
