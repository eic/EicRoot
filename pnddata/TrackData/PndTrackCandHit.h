//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PndTrackCand
//      see PndTrackCand.hh for details
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Tobias Stockmanns (IKP - Juelich) during the Panda Meeting 03/09
//
//
//-----------------------------------------------------------

#ifndef PNDTRACKCANDHIT_HH
#define PNDTRACKCANDHIT_HH

// Root Class Headers ----------------
#include "TObject.h"

#include "FairLink.h"

#include <iostream>
#include <vector>
#include <map>

class PndTrackCandHit : public FairLink{
public:
  PndTrackCandHit():FairLink(), fRho(0){}
  PndTrackCandHit(Int_t detId, Int_t hitId, Double_t rho):FairLink(detId, hitId), fRho(rho){}
  ~PndTrackCandHit() {}
  bool operator< (const PndTrackCandHit& rhs) const
  {return fRho<rhs.fRho;};
  bool operator> (const PndTrackCandHit& rhs) const
  {return fRho>rhs.fRho;};
  bool operator<= (const PndTrackCandHit& rhs) const
  {return fRho<=rhs.fRho;};
  bool operator>= (const PndTrackCandHit& rhs) const
  {return fRho>=rhs.fRho;};
  bool operator== (const PndTrackCandHit& hit) const {
    return ( FairLink::operator==((FairLink)hit) && fRho == hit.fRho);
  }
  bool operator!= (const PndTrackCandHit& hit) const {
    return (!(FairLink::operator==(hit)));
  }
  Int_t GetHitId()const {return GetIndex();}
  Int_t GetDetId()const {return GetType();}
  Double_t GetRho()const {return fRho;}
  
  void Print();
  
  private :
  Double_t fRho;		///< sorting parameter
  
  ClassDef(PndTrackCandHit,2);
};



#endif

//--------------------------------------------------------------
// $Log$
//--------------------------------------------------------------
