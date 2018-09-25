//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PndRecoMultiKalmanTask
//      see PndRecoMultiKalmanTask.h for details
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
#include "PndRecoMultiKalmanTask.h"

// C/C++ Headers ----------------------
#include <iostream>
#include <cmath>

// Collaborating Class Headers --------
#include "TClonesArray.h"
#include "PndTrack.h"
#include "FairRootManager.h"
#include "FairGeanePro.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

PndRecoMultiKalmanTask::PndRecoMultiKalmanTask(const char* name, Int_t iVerbose)
  : FairTask(name, iVerbose), fPersistence(kFALSE)
{
  fTrackInBranchName  = "LheTrack"; 
  fTrackOutBranchName = "LheGenTrack";
  fMvdBranchName = "";
  fCentralTrackerBranchName = "";
  fFitTrackArrayElectron = new TClonesArray("PndTrack");
  fFitTrackArrayMuon     = new TClonesArray("PndTrack");
  fFitTrackArrayPion     = new TClonesArray("PndTrack");
  fFitTrackArrayKaon     = new TClonesArray("PndTrack");
  fFitTrackArrayProton   = new TClonesArray("PndTrack");
  fUseGeane = kTRUE;
  fNumIt = 1;
  fFitter = new PndRecoKalmanFit();
}


PndRecoMultiKalmanTask::~PndRecoMultiKalmanTask()
{
}

InitStatus
PndRecoMultiKalmanTask::Init()
{
 
  fFitter->SetGeane(fUseGeane);
  fFitter->SetNumIterations(fNumIt);
  fFitter->SetMvdBranchName(fMvdBranchName);
  fFitter->SetCentralTrackerBranchName(fCentralTrackerBranchName);
  if (!fFitter->Init()) return kFATAL;
  
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
    {
      Error("PndRecoMultiKalmanTask::Init","RootManager not instantiated!");
      return kERROR;
    }
  
  // Get input collection
  fTrackArray=(TClonesArray*) ioman->GetObject(fTrackInBranchName);
  if(fTrackArray==0)
    {
      Error("PndRecoMultiKalmanTask::Init","track-array not found!");
      return kERROR;
    }
  
  ioman->Register(fTrackOutBranchName+"Electron","Gen", fFitTrackArrayElectron, kTRUE); 
  ioman->Register(fTrackOutBranchName+"Muon",    "Gen", fFitTrackArrayMuon,     kTRUE); 
  ioman->Register(fTrackOutBranchName+"Pion",    "Gen", fFitTrackArrayPion,     kTRUE); 
  ioman->Register(fTrackOutBranchName+"Kaon",    "Gen", fFitTrackArrayKaon,     kTRUE); 
  ioman->Register(fTrackOutBranchName+"Proton",  "Gen", fFitTrackArrayProton,   kTRUE); 
 	return kSUCCESS;
}

void PndRecoMultiKalmanTask::SetParContainers() 
{
  FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();
#if _EIC_OFF_
  fSttParameters = (PndGeoSttPar*) rtdb->getContainer("PndGeoSttPar");
#endif
}

void PndRecoMultiKalmanTask::Exec(Option_t* opt)
{
  if (fVerbose>0) std::cout<<"PndRecoMultiKalmanTask::Exec"<<std::endl;
  
  fFitTrackArrayElectron->Clear();
  fFitTrackArrayMuon->Clear();
  fFitTrackArrayPion->Clear();
  fFitTrackArrayKaon->Clear();
  fFitTrackArrayProton->Clear();
  
  Int_t ntracks=fTrackArray->GetEntriesFast();

  // Detailed output
  if (fVerbose>1) std::cout << " -I- PndRecoMultiKalmanTask: contains " << ntracks << " Tracks."<< std::endl;
  
  // Cut too busy events TODO
  if(ntracks>20)
    {
      std::cout<<" -I- PndRecoMultiKalmanTask::Exec: ntracks=" << ntracks << " Evil Event! skipping" << std::endl;
      return;
    }
  
  
  for(Int_t itr=0;itr<ntracks;++itr)
    {
      if (fVerbose>1) std::cout<<"starting track"<<itr<<std::endl;
      PndTrack *prefitTrack = (PndTrack*)fTrackArray->At(itr);
      Int_t  fCharge= prefitTrack->GetParamFirst().GetQ();
   
      { // Electron
	Int_t PDGCode = -11*fCharge;
	PndTrack *fitTrack = new PndTrack();
	fitTrack = fFitter->Fit(prefitTrack, PDGCode);
	
	TClonesArray& trkRef = *fFitTrackArrayElectron;
	Int_t size = trkRef.GetEntriesFast();
	PndTrack* pndTrack = new(trkRef[size]) PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
							fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), itr, kLheTrack);
      } // end of electron
      
      { // Muon	
	Int_t PDGCode = -13*fCharge;
	PndTrack *fitTrack = new PndTrack();
	fitTrack = fFitter->Fit(prefitTrack, PDGCode);
	
	TClonesArray& trkRef = *fFitTrackArrayMuon;
	Int_t size = trkRef.GetEntriesFast();
	PndTrack* pndTrack = new(trkRef[size]) PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
							fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), itr, kLheTrack);
      } // end of Muon
      
      { // Pion
	Int_t PDGCode = 211*fCharge;
	PndTrack *fitTrack = new PndTrack();
	fitTrack = fFitter->Fit(prefitTrack, PDGCode);
	
	TClonesArray& trkRef = *fFitTrackArrayPion;
	Int_t size = trkRef.GetEntriesFast();
	PndTrack* pndTrack = new(trkRef[size]) PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
							fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), itr, kLheTrack);
      } // end of Pion
      
      { // Kaon	
	Int_t PDGCode = 321*fCharge;
	PndTrack *fitTrack = new PndTrack();
	fitTrack = fFitter->Fit(prefitTrack, PDGCode);
	
	TClonesArray& trkRef = *fFitTrackArrayKaon;
	Int_t size = trkRef.GetEntriesFast();
	PndTrack* pndTrack = new(trkRef[size]) PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
							fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), itr, kLheTrack);
      } // end of Kaon
      
      { // Proton
	Int_t PDGCode = 2212*fCharge;
	PndTrack *fitTrack = new PndTrack();
	fitTrack = fFitter->Fit(prefitTrack, PDGCode);
	
	TClonesArray& trkRef = *fFitTrackArrayProton;
	Int_t size = trkRef.GetEntriesFast();
	PndTrack* pndTrack = new(trkRef[size]) PndTrack(fitTrack->GetParamFirst(), fitTrack->GetParamLast(), fitTrack->GetTrackCand(),
							fitTrack->GetFlag(), fitTrack->GetChi2(), fitTrack->GetNDF(), fitTrack->GetPidHypo(), itr, kLheTrack);
      } // end of Proton
      
    } // end of track loop
  
  if (fVerbose>0) std::cout<<"Fitting done"<<std::endl;
  
  return;
}

ClassImp(PndRecoMultiKalmanTask);
