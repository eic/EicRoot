//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC Monte-Carlo point (hit); 
//

#include "EicMoCaPoint.h"

#include <iostream>

// ---------------------------------------------------------------------------------------

EicMoCaPoint::EicMoCaPoint(Int_t trackID, Int_t primaryMotherID, Int_t secondaryMotherID, 
			   Int_t detID,
			   //const TString &volumePath, 
			   ULong64_t multiIndex,
			   const TVector3 &PosIn, const TVector3 &PosOut, 
			   const TVector3 &MomIn, const TVector3 &MomOut,
			   Double_t tof, Double_t length, Double_t eLoss, Double_t step)
  : FairMCPoint(trackID, detID, PosIn, MomIn, tof, length, eLoss)
{
  //mVolumePath        = volumePath;
  mMultiIndex        = multiIndex;

  mPrimaryMotherID   = primaryMotherID;
  mSecondaryMotherID = secondaryMotherID;

  mPosOut            = PosOut;
  mMomOut            = MomOut;

  mStep              = step;
} // EicMoCaPoint::EicMoCaPoint()

// ---------------------------------------------------------------------------------------

/// Test
void EicMoCaPoint::Print(const Option_t* opt) const {
  // It looks I can use 'opt' pointer as a name here?; do it better later;
  std::cout<< opt << std::endl 
    //std::cout<<"QpcPoint\n"
	   <<" Pos("<<fX<<","<<fY<<","<<fZ<<")\n"
	   <<" dE="<< fELoss << fTrackID
	   <<std::endl;	   
} // EicMoCaPoint::Print()

// ---------------------------------------------------------------------------------------

ClassImp(EicMoCaPoint)
