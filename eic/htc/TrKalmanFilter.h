//
// AYK (ayk@bnl.gov)
//
//    Forward detector tracking application of the Kalman filter stuff; 
//  ported from HERMES/OLYMPUS sources; cleaned up 2014/10/17;
//

#include <Mgrid.h>

class MediaBank;
struct t_particle_group;

extern "C" t_particle_group *get_particle_group_by_name(const char *name);

#include <TrKalmanNode.h>
#include <KalmanFilter.h>

#ifndef _TR_KALMAN_FILTER_
#define _TR_KALMAN_FILTER_

class TrKalmanFilter: public KalmanFilter {
 public:
  // The constructor; just call KalmanFilter() constructor;
 TrKalmanFilter(MfieldMode fieldMode = WithField): 
  KalmanFilter(fieldMode == WithField ? 5 : 4), mFieldMode(fieldMode), 
    mLocationHead(0), mLocationTail(0), mAccountEnergyLosses(true), mLocationSeparationDistance(0.0) {};

  t_particle_group *SetParticleGroup(const char *group) { 
    mParticleGroup = get_particle_group_by_name(group); 

    return mParticleGroup;
  };

  MfieldMode GetFieldMode() const { return mFieldMode; };

  int Configure(MediaBank *media_bank, StringList *config);

  virtual MgridSlice *InitializeMgridSlice(double z0) = 0;
  int InitializeRungeKuttaFrames();
  int InitializeMediaSlices(MediaBank *media_bank);
  void SetLocationSeparationDistance(double value) { mLocationSeparationDistance = value; };

  int CalculateHMatrix(KalmanNode *node);
  int Transport(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode);
  // Multiple scattering and dE/dx accounting;
  int TransportExtra(KalmanNode *from, KalmanFilter::Direction fb, unsigned mode);

  // Prepare node to become a starting point of a filter pass (so 
  // basically reset x0[], xp[], CP[][] & mInversedMomentum); FIXME: need to make 
  // cov.matrix parameters configurable as well;
  void ResetNode(TrKalmanNode *node, double S[], int assignmentMode);

  void SelectActiveNodes();
  void SetUpLocations();

  TrKalmanNodeLocation *GetLocationHead()  const { return mLocationHead; };

  // Do this better later (bind FF[]- matrix calculation to locations rather 
  // than nodes);
  void BuildNodeList() {
    KalmanFilter::BuildNodeList();

    CalculateMagnetOffTransportMatrices();
  };

  void AccountEnergyLosses(bool flag) { mAccountEnergyLosses = flag; };

 protected:
  double mLocationSeparationDistance;
  TrKalmanNodeLocation *mLocationHead, *mLocationTail;

 private:
  MfieldMode mFieldMode;

  // It turns out to be very convenient to store particle type
  // in a separate variable instead of passing application-specific 
  // pointers to tfun()/.../xfun() functions;
  t_particle_group *mParticleGroup;

  bool mAccountEnergyLosses;

  // Yes, this is the trick: allocate extra space compared to the 
  // KalmanNode base class; whether it is correct or not I do not 
  // care to the moment; from memory allocation model I'd suppose it is;
  KalmanNode *AllocateNode() { return new TrKalmanNode(); };

  // Yes, for tracking Kalman filter this call checks field/no-field mode; 
  // FIXME: may want to return back the functionality to check for field-free regions;
  bool NeedNonLinearTransport(double z) const { 
    return mFieldMode == WithField ? true : false; 
  };

  int AccountIonizationLosses(TrKalmanNode *from, KalmanFilter::Direction fb);
  int CalculateProcessNoise  (TrKalmanNode *from, KalmanFilter::Direction fb); 

  int CalculateMagnetOffTransportMatrices();
};

#endif
