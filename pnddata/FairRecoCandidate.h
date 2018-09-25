#ifndef FAIRRECOCANDIDATE_H
#define FAIRRECOCANDIDATE_H
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// VAbsMicroCandidate	                                                //
//                                                                      //
// Definition of an abstract interface to a micro candidate.	        //
//                                                                      //
// Author: Sascha Berger and Marcel Kunze, RUB, March 1999		//
// Copyright (C) 1999-2001, Ruhr-University Bochum.			//
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <assert.h>

#include "FairMultiLinkedData.h"
#include "TObject.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"

//class VAbsPidInfo;

//  ========================================================================
//  ===== VAbsMicroCandidate - Abstract Class definig the AOD interface  ====
//  ========================================================================

class FairRecoCandidate : public FairMultiLinkedData
{

 public:

  FairRecoCandidate() {}
  virtual ~FairRecoCandidate() {}

  // ************************
  // basic properties:
  // ************************

  virtual Int_t    GetCharge() const =0;
  virtual TVector3 GetPosition() const =0;
  virtual TVector3 GetMomentum() const =0;
  virtual Double_t GetEnergy() const =0;
  virtual TLorentzVector GetLorentzVector() const =0;
	
  virtual TVector3  GetFirstHit() const=0;
  virtual TVector3  GetLastHit() const=0;
    
  virtual const Float_t* GetErrorP7() const=0;
  virtual const Float_t* GetCov()const =0;
  virtual const Float_t* GetParams()const=0;
	
  virtual TMatrixD& Cov7() const =0;
  virtual TMatrixD& P4Cov() const =0;
  virtual Int_t    GetMcIndex() const =0;

  virtual Int_t     GetTrackIndex() const=0;	
  virtual Int_t     GetTrackBranch() const=0;
  
  // Tracking
  //virtual Float_t		GetTrackLength() const =0;
  virtual Int_t    	GetDegreesOfFreedom() const =0;
  virtual Int_t    	GetFitStatus() const =0;
  //Float_t  	GetProbability() {return TMath::Prob(GetChiSquared(),GetDegreesOfFreedom());};
  virtual Float_t  	GetChiSquared() const =0;
	    
  //PID
  virtual Float_t		GetElectronPidLH() const=0;
  virtual Float_t		GetMuonPidLH() const=0;
  virtual Float_t		GetPionPidLH() const=0;
  virtual Float_t		GetKaonPidLH() const=0;
  virtual Float_t		GetProtonPidLH() const=0;
	
  void     	PrintOn( std::ostream& o=std::cout ) const;

 
	
  ClassDef(FairRecoCandidate,1) // Abstract base class for MicroDST candidates
    };

std::ostream&  operator << (std::ostream& o, const FairRecoCandidate&);

#endif                                           


