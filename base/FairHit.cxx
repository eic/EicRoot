#include "FairHit.h"


// -----   Default constructor   -------------------------------------------
FairHit::FairHit()
  : FairTimeStamp(),
    fDx(0),
    fDy(0),
    fDz(0),
    fRefIndex(-1),
    fDetectorID(-1),
    fX(0),
    fY(0),
    fZ(0)


{
}

FairHit::FairHit(Int_t detID, TVector3& pos, Int_t index)
  :FairTimeStamp(),
   fDx          (0.0),
   fDy          (0.0),
   fDz          (0.0),
   fRefIndex    (index),
   fDetectorID  (detID),
   fX           (pos.X()),
   fY           (pos.Y()),
   fZ           (pos.Z())
{
}

// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
FairHit::FairHit(Int_t detID, TVector3& pos, TVector3& dpos, Int_t index)
  :FairTimeStamp(),
   fDx          (dpos.X()),
   fDy          (dpos.Y()),
   fDz          (dpos.Z()),
   fRefIndex    (index),
   fDetectorID  (detID),
   fX           (pos.X()),
   fY           (pos.Y()),
   fZ           (pos.Z())
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
FairHit::~FairHit() { }
// -------------------------------------------------------------------------



ClassImp(FairHit)
