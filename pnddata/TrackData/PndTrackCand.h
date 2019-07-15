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

#ifndef PNDTRACKCAND_HH
#define PNDTRACKCAND_HH

// Root Class Headers ----------------
#include "PndTrackCandHit.h"
#include "FairTimeStamp.h"

#include "TObject.h"
#include "TVector3.h"
#include "TString.h"

#include <iostream>
#include <vector>
#include <map>

typedef std::multimap<Double_t, std::pair<Int_t, Int_t> >::const_iterator mapIter;

class PndTrackCand : public FairTimeStamp {
public:

  // Constructors/Destructors ---------
  PndTrackCand();
  ~PndTrackCand();

  // operators
  bool operator== (const PndTrackCand& rhs);

  // Accessors -----------------------
  PndTrackCandHit GetSortedHit(UInt_t i){
	  if (sorted == false)
		  Sort();
	  return fHitId.at(i);
  }
  UInt_t GetNHits() const {return fHitId.size();}
  int getMcTrackId() const {return fMcTrackId;}
  TVector3 getPosSeed() const {return fPosSeed;}
  //TVector3 getDirSeed() const {return fDirSeed;}
  //double getQoverPseed() const {return fQoverPseed;}
  TVector3 getMomSeed() const {return fMomSeed;}
  double getChargeSeed() const {return fChargeSeed;}
  
  UInt_t GetNHitsDet(UInt_t detId);
  std::vector<PndTrackCandHit> &_GetSortedHits();
  void Sort();
  
  // Modifiers -----------------------
  void AddHit(UInt_t detId, UInt_t hitId, Double_t rho);
  void AddHit(TString branchName, UInt_t hitId, Double_t rho);
  void AddHit(FairLink link, Double_t rho);
  void DeleteHit(UInt_t detId, UInt_t hitId);
  Int_t HitInTrack(UInt_t detId, UInt_t hitId);
  void setMcTrackId(int i){fMcTrackId=i;}
  //void setTrackSeed(const TVector3& p,const TVector3& d,double qop){
  //fPosSeed=p;fDirSeed=d;fQoverPseed=qop;
  //}
  void setTrackSeed(const TVector3& p,const TVector3& m,double charge){
    fPosSeed=p; fMomSeed=m; fChargeSeed=charge;
  }

  void CalcTimeStamp();

  void ResetLinks();

  void Print();

private:
  // Private Data Members ------------
	std::vector<PndTrackCandHit> fHitId;  ///< first index is detId, second index is hit Id
	bool sorted;
	int fMcTrackId; //track id for MC simulation
	TVector3 fPosSeed;
	TVector3 fMomSeed;//fDirSeed;
	double fChargeSeed;//fQoverPseed;
	Int_t fVerbose;
public:
  ClassDef(PndTrackCand,3);
};

#endif

//--------------------------------------------------------------
// $Log$
//--------------------------------------------------------------
