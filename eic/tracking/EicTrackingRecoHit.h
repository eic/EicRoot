//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Tracking reco hit and GenFit interface classes;
//

#ifndef _EIC_TRACKING_RECO_HIT_
#define _EIC_TRACKING_RECO_HIT_

#include "GFRecoHitIfc.h"
#include "GFPlanarHitPolicy.h"
#include "GFSpacepointHitPolicy.h"

#include <ostream> 

#include <PndGeoHandling.h>

#include <EicTrackingDigiHit.h>

class EicTrackingRecoHit {
 public:
  EicTrackingRecoHit() {};
  ~EicTrackingRecoHit() {};

 protected:
  const LogicalVolumeLookupTableEntry *GetLookupTableNode(EicTrackingDigiHit* hit, void *ptr);

  TMatrixT<double> getHMatrix(const GFAbsTrackRep* stateVector);

  virtual unsigned GetMdim() const = 0;
};

class EicPlanarRecoHit : public EicTrackingRecoHit, public GFRecoHitIfc<GFPlanarHitPolicy> {
public:
 EicPlanarRecoHit(): GFRecoHitIfc<GFPlanarHitPolicy>(0), mDim(0) {};
  EicPlanarRecoHit(EicTrackingDigiHit* hit, void *ptr = 0);
  ~EicPlanarRecoHit() {};

 private:
  GFAbsRecoHit* clone() { return new EicPlanarRecoHit(*this); };

  unsigned GetMdim() const { return mDim; };

  UChar_t mDim;

  // Can not inherit this virtual function from EicTrackingRecoHit -> just call it;
  TMatrixT<double> getHMatrix(const GFAbsTrackRep* stateVector) { 
    return EicTrackingRecoHit::getHMatrix(stateVector);
  };

  ClassDef(EicPlanarRecoHit,3);
};

class EicSpaceRecoHit : public EicTrackingRecoHit, public GFRecoHitIfc<GFSpacepointHitPolicy> {
public:
 EicSpaceRecoHit(): GFRecoHitIfc<GFSpacepointHitPolicy>(0) {};
  EicSpaceRecoHit(EicTrackingDigiHit* hit, void *ptr = 0);
  ~EicSpaceRecoHit() {};

 private:
  GFAbsRecoHit* clone() { return new EicSpaceRecoHit(*this); };

  unsigned GetMdim() const { return 2; };

  // Can not inherit this virtual function from EicTrackingRecoHit -> just call it;
  TMatrixT<double> getHMatrix(const GFAbsTrackRep* stateVector) { 
    return EicTrackingRecoHit::getHMatrix(stateVector);
  };

  ClassDef(EicSpaceRecoHit,2);
};

#endif
