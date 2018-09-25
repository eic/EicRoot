//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PndTrackID
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      01/07/09 - Stefano Spataro (Torino)
//
//
//-----------------------------------------------------------

// Panda Headers ----------------------
#include <iostream>

// This Class' Header ------------------
#include "PndTrackID.h"

using std::cout;
using std::endl;

ClassImp(PndTrackID);

PndTrackID::PndTrackID()
{
  fTrackID = -1;
  fCorrTrackIds.Set(0);
  fMultTrackIds.Set(0);
}

PndTrackID::PndTrackID(Int_t id, TArrayI track, TArrayI mult)
{
  fTrackID = id;
  fCorrTrackIds = track;
  fMultTrackIds = mult;
}

PndTrackID::~PndTrackID(){
}

void PndTrackID::Reset()
{
  fCorrTrackIds.Reset();
  fMultTrackIds.Reset();
}

void PndTrackID::Print()
{
  std::cout << "PndTrackID::Print() - PndTrackID: " << fTrackID << "\tNumber of correlated MCTrack ids: " << GetNCorrTrackId() << std::endl;
  for(Int_t ii=0; ii<GetNCorrTrackId(); ++ii)
    {
      std::cout << " *** At: " << ii
		<< "\t MCTrack ID: " << fCorrTrackIds[ii]
		<< "\t Multiplicity: " << fMultTrackIds[ii]
		<< std::endl;
  }
}
