// -------------------------------------------------------------------------
// -----                         CbmHit source file                    -----
// -----                   Created 16/11/07  by V. Friese              -----
// -------------------------------------------------------------------------


#include "CbmHit.h"

#include <iostream>
using namespace std;


// -----   Default constructor   -------------------------------------------
CbmHit::CbmHit() 
  : FairHit(),
    fCovXY(0.)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmHit::CbmHit(Int_t detId, TVector3& pos, TVector3& dpos, 
		     Double_t covXY, Int_t index) 
  : FairHit(detId, pos, dpos, index), 
    fCovXY(covXY)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmHit::~CbmHit() { }
// -------------------------------------------------------------------------




// -----   Public method Print   -------------------------------------------
void CbmHit::Print(const Option_t* opt) const {
  cout.precision(5);
  cout << "Hit at (" << fX << ", " << fY << ", " << fZ << ") cm, "
       << "Detector " << fDetectorID << ", Station " << GetStationNr() 
       << endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmHit)
