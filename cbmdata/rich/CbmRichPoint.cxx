// -------------------------------------------------------------------------
// -----                      CbmRichPoint source file                 -----
// -----               Created 28/04/04  by B. Polichtchouk            -----
// -------------------------------------------------------------------------

#include "CbmRichPoint.h"

#include <iostream>

using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmRichPoint::CbmRichPoint() : FairMCPoint(), fPDG(0) {}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmRichPoint::CbmRichPoint(Int_t trackID, Int_t pdg, Int_t detID, TVector3 pos, 
			   TVector3 mom, Double_t tof, Double_t length, 
			   Double_t eLoss)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss), fPDG(pdg) { }


// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmRichPoint::~CbmRichPoint() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmRichPoint::Print(const Option_t* opt) const {
  cout << "-I- CbmRichPoint: RICH Point for track " << fTrackID 
       << " in detector " << fDetectorID << endl;
  cout << "    Position (" << fX << ", " << fY << ", " << fZ
       << ") cm" << endl;
  cout << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
       << ") GeV" << endl;
  cout << "    Time " << fTime << " ns,  Length " << fLength 
       << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmRichPoint)
