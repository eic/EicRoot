//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PndRecoKalmanTask
//      see PndRecoKalmanTask.hh for details
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//      Stefano Spataro, UNI Torino
//
//-----------------------------------------------------------

// Panda Headers ----------------------

// This Class' Header ------------------
#include "PndRecoKalmanTask.h"

// C/C++ Headers ----------------------
#include <iostream>
#include <cmath>

// Collaborating Class Headers --------
#include "TClonesArray.h"
#include "TParticlePDG.h"
#include "PndTrack.h"
#include "PndTrackID.h"
#include "PndMCTrack.h"
#include "FairRootManager.h"
#include "FairGeanePro.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

PndRecoKalmanTask::PndRecoKalmanTask(const char* name, Int_t iVerbose)
: FairTask(name, iVerbose), fTrackInBranchName(""), fTrackInIDBranchName(""),
fTrackOutBranchName(""), fMvdBranchName(""), fCentralTrackerBranchName(""),
fFitTrackArray(), fFitter(), fDafFitter(), fPDGHyp(-13),
fUseGeane(kTRUE), fIdealHyp(kFALSE), fDaf(kFALSE), fPersistence(kTRUE),
fPropagateToIP(kTRUE), fPerpPlane(kFALSE),
  fNumIt(1), fBusyCut(20), fTrackRep(0)
{
  fFitTrackArray = new TClonesArray("PndTrack");  
  fFitter = new PndRecoKalmanFit(); 
  fDafFitter = new PndRecoDafFit();
}


PndRecoKalmanTask::~PndRecoKalmanTask()
{
}

InitStatus
PndRecoKalmanTask::Init()
{

  switch (fTrackRep)
    {
    case 0:
      std::cout << " -I- PndRecoKalmanTask:Init :: Using GeaneTrackRep" << std::endl;
      break;
    case 1:
      std::cout << " -I- PndRecoKalmanTask:Init :: Using RKTrackRep" << std::endl;
      break;
    default:
      Error("PndRecoKalmanTask::Init","Not existing Track Representation!!");
      return kERROR;   
    }
  
  if (!fDaf)
    {
      fFitter->SetGeane(fUseGeane);
      fFitter->SetPropagateToIP(fPropagateToIP);
      fFitter->SetPerpPlane(fPerpPlane);
      fFitter->SetNumIterations(fNumIt); 
      fFitter->SetMvdBranchName(fMvdBranchName);
      fFitter->SetCentralTrackerBranchName(fCentralTrackerBranchName); 
      fFitter->SetVerbose(fVerbose);
      fFitter->SetTrackRep(fTrackRep);
      if (!fFitter->Init()) return kFATAL;
    }
  else
    {
      fDafFitter->SetGeane(fUseGeane);
      fDafFitter->SetPropagateToIP(fPropagateToIP);
      fDafFitter->SetPerpPlane(fPerpPlane);
      fDafFitter->SetMvdBranchName(fMvdBranchName);
      fDafFitter->SetCentralTrackerBranchName(fCentralTrackerBranchName);
      fDafFitter->SetVerbose(fVerbose);
      fDafFitter->SetTrackRep(fTrackRep);
      if (!fDafFitter->Init()) return kFATAL;
    }
  
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("PndRecoKalmanTask::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  // Get input collection
  fTrackArray=(TClonesArray*) ioman->GetObject(fTrackInBranchName);
  if(fTrackArray==0)
  {
    Error("PndRecoKalmanTask::Init","track-array not found!");
    return kERROR;
  }
  if (fIdealHyp)
    { 
      pdg = new TDatabasePDG();
      fTrackIDArray=(TClonesArray*) ioman->GetObject(fTrackInIDBranchName);
      if(fTrackIDArray==0)
	{
	  Error("PndRecoKalmanTask::Init","track ID array not found! It is not possible to run ideal particle hypothesis");
	  return kERROR;
	} 

      fMCTrackArray=(TClonesArray*) ioman->GetObject("MCTrack");
      if(fMCTrackArray==0)
	{
	  Error("PndRecoKalmanTask::Init","MCTrack array not found! It is not possible to run ideal particle hypothesis");
	  return kERROR;
	}
    }
  
  ioman->Register(fTrackOutBranchName,"Gen", fFitTrackArray, fPersistence);
  
  return kSUCCESS;
}

void PndRecoKalmanTask::SetParContainers() {
  FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();
#if _EIC_OFF_
  rtdb->getContainer("PndGeoSttPar");
  rtdb->getContainer("PndGeoFtsPar");
#endif
}

void PndRecoKalmanTask::Exec(Option_t* opt)
{
  if (fVerbose>0) std::cout<<"PndRecoKalmanTask::Exec"<<std::endl;
  
  fFitTrackArray->Delete();
  
  Int_t ntracks=fTrackArray->GetEntriesFast();
  
  // Detailed output
  if (fVerbose>1) std::cout << " -I- PndRecoKalmanTask: contains " << ntracks << " Tracks."<< std::endl;
  
  // Cut too busy events TODO
  if(ntracks>fBusyCut)
  {
    std::cout<<" -I- PndRecoKalmanTask::Exec: ntracks=" << ntracks << " Evil Event! skipping" << std::endl;
    return;
  }
  
  
  for(Int_t itr=0;itr<ntracks;++itr)
  {
    if (fVerbose>1) std::cout<<"starting track"<<itr<<std::endl;
    
    TClonesArray& trkRef = *fFitTrackArray;
    Int_t size = trkRef.GetEntriesFast();
    
    PndTrack *prefitTrack = (PndTrack*)fTrackArray->At(itr);
    Int_t  fCharge= prefitTrack->GetParamFirst().GetQ();
    Int_t PDGCode = 0;
    if (fIdealHyp)
      {
	PndTrackID *prefitTrackID = (PndTrackID*)fTrackIDArray->At(itr);
	if (prefitTrackID->GetNCorrTrackId()>0)
          {
	    Int_t mcTrackId = prefitTrackID->GetCorrTrackID();
	    if (mcTrackId!=-1)
	      {
		PndMCTrack *mcTrack = (PndMCTrack*)fMCTrackArray->At(mcTrackId);
		if (!mcTrack)
		   {
                    PDGCode = 211*fCharge;
                    std::cout << "-I- PndRecoKalmanTask::Exec: MCTrack #" << mcTrackId << " is not existing!! Trying with pion hyp" << std::endl;
                  }
		else
		  {
		    PDGCode = mcTrack->GetPdgCode();
		  }
                if (PDGCode>=100000000)
                  {
                    PDGCode = 211*fCharge;
                    std::cout << "-I- PndRecoKalmanTask::Exec: Track is an ion (PDGCode>100000000)! Trying with pion hyp" << std::endl;
                  }
                else if ((((TParticlePDG*)pdg->GetParticle(PDGCode))->Charge())==0)
		  {
		    PDGCode = 211*fCharge;
		    std::cout << "-E- PndRecoKalmanTask::Exec: Track MC charge is 0!!!! Trying with pion hyp" << std::endl;
		  }
	      } // end of MCTrack ID != -1
	    else
	      {
		PDGCode = 211*fCharge;
		std::cout << "-E- PndRecoKalmanTask::Exec: No MCTrack index in PndTrackID!! Trying with pion hyp" << std::endl;
	      }
	  } // end of "at least one correlated mc index"
	else
	  {
	    PDGCode = 211*fCharge;
	    std::cout << "-E- PndRecoKalmanTask::Exec: No Correlated MCTrack id in PndTrackID!! Trying with pion hyp" << std::endl;
	  }
      } // end of ideal hyp condition
    else
      {
	PDGCode = fPDGHyp*fCharge;
      }
    
    PndTrack *fitTrack = new PndTrack();
    if (PDGCode!=0)
      {
	if (fDaf) fitTrack = fDafFitter->Fit(prefitTrack, PDGCode);
	else fitTrack = fFitter->Fit(prefitTrack, PDGCode);

#if _OFF_
	{
	  for(unsigned iq=0; iq<fitTrack->mSmoothedValues.size(); iq++) {
	    TVector3 &pos = fitTrack->mSmoothedValues[iq].first;
	    TVector3 &mom = fitTrack->mSmoothedValues[iq].second;

	    printf("%10.4f %10.4f %10.4f -> %10.4f %10.4f %10.4f\n", 
		   pos.X(), pos.Y(), pos.Z(), mom.X(), mom.Y(), mom.Z());
	  } //for iq
	}
#endif 
      }
    else
      {
	fitTrack = prefitTrack;
	fitTrack->SetFlag(22);
	std::cout << "-I- PndRecoKalmanTask::Exec: Kalman cannot run on this track because of the bad MonteCarlo PDC code" << std::endl;
      }
    
    PndTrack* pndTrack = new(trkRef[size]) PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
                                                    fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), itr, FairRootManager::Instance()->GetBranchId(fTrackInBranchName));

    for(unsigned iq=0; iq<fitTrack->mSmoothedValues.size(); iq++)
      pndTrack->mSmoothedValues.push_back(fitTrack->mSmoothedValues[iq]);
  }
  
  if (fVerbose>0) std::cout<<"Fitting done"<<std::endl;
  
  return;
}

void PndRecoKalmanTask::SetParticleHypo(TString h)
{
  // Set the hypothesis for the fit, charge will be applied later
  if(h.BeginsWith("e") || h.BeginsWith("E")){
    fPDGHyp=-11; //electrons
  }else if(h.BeginsWith("m") || h.BeginsWith("M")){
    fPDGHyp=-13; //muons
  }else if(h.BeginsWith("pi") || h.BeginsWith("Pi") || h.BeginsWith("PI")){
    fPDGHyp=211; //pions
  }else if(h.BeginsWith("K") || h.BeginsWith("K")){
    fPDGHyp=321; //kaons
  }else if(h.BeginsWith("p") || h.BeginsWith("P") || h.BeginsWith("antip")){
    fPDGHyp=2212; //protons/antiprotons
  }else{
    std::cout << "-I- PndRecoKalmanTask::SetParticleHypo: Not recognised PID set -> Using default MUON hypothesis" << std::endl;
    fPDGHyp=-13; // Muon is default.
  }
}

void PndRecoKalmanTask::SetParticleHypo(Int_t h)
{  
  switch (abs(h))
  {
    case 11:
      fPDGHyp = -11;
      break;
    case 13:
      fPDGHyp = -13;
      break;
    case 211:
      fPDGHyp = 211;
      break;
    case 321:
      fPDGHyp = 321;
      break;
    case 2212:
      fPDGHyp = 2212;
      break;
    default:
      std::cout << "-I- PndRecoKalmanTask::SetParticleHypo: Not recognised PID set -> Using default MUON hypothesis" << std::endl;
      fPDGHyp = -13;
      break;
  }
}
  ClassImp(PndRecoKalmanTask);
