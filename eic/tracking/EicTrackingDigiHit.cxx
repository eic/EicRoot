//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Tracking digi hit class;
//

#include <iostream>

#include <TMath.h>

#include "EicTrackingDigiHit.h"

// ---------------------------------------------------------------------------------------

EicTrackingDigiHit::EicTrackingDigiHit(TString cname, const EicMoCaPoint *point,
				       unsigned kfNodeID, TVector3& global, 
				       const TVector3& local, bool xy_mode) : 
  FairHit(point->GetDetectorID(), global, point->GetPointID()), mLocalCoord(local), mXYmode(xy_mode)
{
  ResetVars();
  
  mKfNodeID = kfNodeID;
  mMultiIndex = point->GetMultiIndex();

  SetLink(FairLink(cname + "MoCaPoint", point->GetPointID()));
} // EicTrackingDigiHit::EicTrackingDigiHit()

// ---------------------------------------------------------------------------------------
#if _OLD_
TVector3 EicTrackingDigiHit::GetLocalCoordinates()
{
  TVector3 local;

  for(unsigned iq=0; iq<GetMdim(); iq++) 
    local[iq] = GetCoord(iq);

  return local;
} // EicTrackingDigiHit::GetLocalCoordinates()
#endif
// ---------------------------------------------------------------------------------------

ClassImp(EicTrackingDigiHit)
ClassImp(EicTrackingDigiHit1D)
ClassImp(EicTrackingDigiHitOrth2D)
ClassImp(EicTrackingDigiHit3D)

