//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PndRecoKalmanFit
//      see PndRecoKalmanFit.h for details
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Stefano Spataro, UNI Torino
//
//-----------------------------------------------------------

// Panda Headers ----------------------

// This Class' Header ------------------
#include "PndRecoKalmanFit.h"

// C/C++ Headers ----------------------
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <cmath>

// Collaborating Class Headers --------
#include "FairRootManager.h"
#include "FairRuntimeDb.h"
#include "FairRunAna.h"
#include "TClonesArray.h"

#include "GFTrack.h"
//#include "TDatabasePDG.h"

#include "PndGenfitAdapters.h"
#include "PndTrack.h"
#include "PndTrackCand.h"
#include "PndDetectorList.h"
#include "PndGeoHandling.h"
#include "GFRecoHitFactory.h"
#include "GFKalman.h"
#include "GFException.h"
#include "TLorentzVector.h"

#include "FairTrackParH.h"

#include "GeaneTrackRep.h"
#include "RKTrackRep.h"
#include "GFFieldManager.h"
#include "PndGenfitField.h"
#include "FairGeanePro.h"

// Class Member definitions -----------


PndRecoKalmanFit::PndRecoKalmanFit(): TNamed("Genfit", "Fit Tracks"),
                                      fMvdBranchName(""), fCentralTrackerBranchName(""),
				      fUseGeane(kTRUE), fPropagateToIP(kTRUE), fPerpPlane(kFALSE), fNumIt(1), fVerbose(0), fTrackRep(0)
{
  PndGeoHandling::Instance();
}

Bool_t PndRecoKalmanFit::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  if(ioman==0)
    {
      Error("PndRecoKalmanFit::Init","RootManager not instantiated!");
      return kFALSE;
    }

  // STT map loading
  //FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();

  //fVerbose = 2;

  // Build hit factory -----------------------------
  fTheRecoHitFactory = new GFRecoHitFactory();
  if (fVerbose<2) GFException::quiet(true);
  
  if (fUseGeane)
    {
      fPro = new FairGeanePro();
      //if (fVerbose==0) fPro->SetPrintErrors(kFALSE);
    }
  else
    {
      Error("PndRecoKalmanFit::Init","Only GEANE Propagatio available!!!");
      return kFALSE;
    }
  
  fGenFitter.setNumIterations(fNumIt);
  
  if (fTrackRep==1) GFFieldManager::getInstance()->init(new PndGenfitField());

  std::cout << "===PndRecoKalmanFit::Init() finished ===================================================" << std::endl;

  return kTRUE;
}


PndRecoKalmanFit::~PndRecoKalmanFit() { }

#include <FairHit.h>
#include <EicMoCaPoint.h>
#include <GFTools.h>

PndTrack* PndRecoKalmanFit::Fit(PndTrack *tBefore, Int_t PDG, bool store_track_parameterization)
{
  PndTrack* tAfter = NULL;
  if (fVerbose>0) std::cout<<"PndRecoKalmanFit::Fit"<<std::endl;
  if (fabs(tBefore->GetParamFirst().GetPz())<1e-9) 
    {
      tAfter = tBefore;
      tAfter->SetFlag(-10);
      return tAfter; // flag -10 : pz==0
    }
  
  Int_t  fCharge= tBefore->GetParamFirst().GetQ();
  Int_t PDGCode= PDG;
  TVector3 StartPos(tBefore->GetParamFirst().GetX(),tBefore->GetParamFirst().GetY(),tBefore->GetParamFirst().GetZ()); 
  TVector3 StartMom(tBefore->GetParamFirst().GetPx(),tBefore->GetParamFirst().GetPy(),tBefore->GetParamFirst().GetPz());
  TVector3 StartPosErr(tBefore->GetParamFirst().GetDX(),tBefore->GetParamFirst().GetDY(),tBefore->GetParamFirst().GetDZ()); 
  TVector3 StartMomErr(tBefore->GetParamFirst().GetDPx(),tBefore->GetParamFirst().GetDPy(),tBefore->GetParamFirst().GetDPz());
  
  GFAbsTrackRep* rep = 0;

  if (fPropagateToIP)
    { 
      // Calculating params at PCA to Origin
      FairTrackParP par = tBefore->GetParamFirst();
      Int_t ierr = 0;
      FairTrackParH *helix = new FairTrackParH(&par, ierr);
      FairGeanePro *fPro0 = new FairGeanePro();
      //if (fVerbose==0) fPro0->SetPrintErrors(kFALSE);
      FairTrackParH *fRes= new FairTrackParH();
      fPro0->SetPoint(TVector3(0,0,0));
      fPro0->PropagateToPCA(1, -1);
      Bool_t rc =  fPro0->Propagate(helix, fRes, PDGCode);
      if (rc)
        {
          StartPos.SetXYZ(fRes->GetX(), fRes->GetY(), fRes->GetZ());
          StartMom.SetXYZ(fRes->GetPx(), fRes->GetPy(), fRes->GetPz());
          StartPosErr.SetXYZ(fRes->GetDX(), fRes->GetDY(), fRes->GetDZ());
          StartMomErr.SetXYZ(fRes->GetDPx(), fRes->GetDPy(), fRes->GetDPz());
        }
    }
  
  TVector3 plane_v1, plane_v2;
  if (fPerpPlane)
    {
      plane_v1 = StartMom.Orthogonal();
      plane_v2 = StartPos.Cross(plane_v1);
    }
  else
    {
      plane_v1.SetXYZ(1.,0.,0.);
      plane_v2.SetXYZ(0.,1.,0.);
    }
  GFDetPlane start_pl(StartPos, plane_v1, plane_v2);
  GFTrack* trk;
  if (fTrackRep==0)
    {
      GeaneTrackRep *grep = new GeaneTrackRep(fPro,
					   start_pl,StartMom,
					   StartPosErr,StartMomErr,
					   fCharge,PDGCode);
      grep->setPropDir(1);
      rep = grep;
    }
  else if (fTrackRep==1)
    {
      RKTrackRep *grep = new RKTrackRep(StartPos, StartMom,
					   StartPosErr, StartMomErr,
					   PDGCode);
      rep = grep;
    }
  else
    {
      std::cout << "*** PndRecoKalmanFit::Exec" << "\t" << "Not existing Track Representation " << fTrackRep << std::endl;
      return NULL; // any smarted ideas?
    }

  trk= new GFTrack(rep);

  PndTrackCand trackCand = tBefore->GetTrackCand();
  trk->setCandidate(*PndTrackCand2GenfitTrackCand(&trackCand));
  
  // Load RecoHits
  try 
    {
      trk->addHitVector(fTheRecoHitFactory->createMany(trk->getCand()));
      if (fVerbose>0) std::cout<<trk->getNumHits()<<" hits in track " << std::endl;
    }
  catch(GFException& e)
    {
      std::cout << "*** PndRecoKalmanFit::Exec" << "\t" << "Genfit Exception: trk->addHitVector " << e.what() << std::endl;
      //throw e;
    }
  // Start Fitter
  try
    {
      // NB: want fast smoothing turned on;
      if (store_track_parameterization) trk->setSmoothing(true, true);

      fGenFitter.processTrack(trk);
    }
  catch (GFException e)
    {
      std::cout<<"*** FITTER EXCEPTION ***"<<std::endl;
      std::cout<<e.what()<<std::endl;
    }
  if (fVerbose>0) std::cout<<"SUCCESSFULL FIT!"<<std::endl;
  
  try
    { 
      tAfter = (PndTrack*)GenfitTrack2PndTrack(trk);
    }
  catch (GFException e)
    {
      std::cout<<"*** PndGenfitAdapters EXCEPTION ***"<<std::endl;
      std::cout<<e.what()<<std::endl;
      tAfter = tBefore;
      tAfter->SetFlag(-2); // flag -2: conversion failed
    } 

  if (fVerbose>0) std::cout<<"Fitting done"<<std::endl;
  if (store_track_parameterization) {    
    PndTrackCand *cand = tAfter->GetTrackCandPtr(); 
    std::vector<PndTrackCandHit> &ptchits = cand->_GetSortedHits();

    //printf("%2d hits total\n", trk->getNumHits());
    for(unsigned ih=0; ih<trk->getNumHits(); ih++) {
      bool ret;

      TVector3 rcpos = GFTools::getBiasedSmoothedPosXYZ(trk, 0, ih, &ret);
      if (ret) {
	TVector3 rcmom = GFTools::getBiasedSmoothedMomXYZ(trk, 0, ih, &ret);

	if (ret) {
	  NaiveTrackParameterization param(true);
	  param.SetRecoPosition(rcpos); param.SetRecoMomentum(rcmom); 

	  // Pull out the original {pos,mom} values at this location;
	  {
	    PndTrackCandHit *ptchit = &ptchits[ih];

	    TClonesArray *dghits = FairRootManager::Instance()->GetDigiLookup(ptchit->GetDetId());
	    FairHit *dghit = (FairHit*) dghits->At(ptchit->GetHitId()); assert(dghit);

	    TClonesArray *mchits = FairRootManager::Instance()->GetMoCaLookup(ptchit->GetDetId());
	    EicMoCaPoint *mchit = (EicMoCaPoint*) mchits->At(dghit->GetRefIndex()); assert(mchit);
	    TVector3 mcpos = mchit->GetPosAvg(), mcmom = mchit->GetMomAvg();

	    param.SetMoCaPosition(mcpos); param.SetMoCaMomentum(mcmom); 

	    // Perform some massaging, per default; have no idea why momentum 
	    // direction gets flipped at a fraction of nodes; have no desire to 
	    // debug this s*it;
	    if (mcmom.Dot(rcmom) < 0.0) param.SetRecoMomentum(-rcmom);

	    // Eventually push back;
	    tAfter->mParameterizations.push_back(param);
	  }
	} //if
      } //if
    } //for ih
  } //if

  return tAfter;
}

ClassImp(PndRecoKalmanFit);
