//-----------------------------------------------------------
// Task which coverts TClonesArray of GFTrack to TClonesArray of PndTrack 
//-----------------------------------------------------------

// Panda Headers ----------------------

// This Class' Header ------------------
#include "PndGFTrackToPndTrackConvertorTask.h"

// C/C++ Headers ----------------------
#include <iostream>
#include <cmath>

// Collaborating Class Headers --------
#include "TClonesArray.h"
#include "FairRootManager.h"
#include "PndTrack.h"
#include "GFTrack.h"
#include "GFException.h"
#include "PndGenfitAdapters.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

PndGFTrackToPndTrackConvertorTask::PndGFTrackToPndTrackConvertorTask(const char* name, Int_t iVerbose)
: FairTask(name, iVerbose)
{
  fTrackInBranchName  = "TrackPostFitComplete"; 
  fTrackOutBranchName = "PndTrackPostFitComplete"; 
  fOutTrackArray = new TClonesArray("PndTrack");  
}


PndGFTrackToPndTrackConvertorTask::~PndGFTrackToPndTrackConvertorTask()
{
}

InitStatus
PndGFTrackToPndTrackConvertorTask::Init()
{
  
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("PndGFTrackToPndTrackConvertorTask::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  // Get input collection
  fInTrackArray=(TClonesArray*) ioman->GetObject(fTrackInBranchName);
  if(fInTrackArray==0)
  {
    Error("PndGFTrackToPndTrackConvertorTask::Init","track-array not found!");
    return kERROR;
  }
  
  ioman->Register(fTrackOutBranchName,"Gen", fOutTrackArray, kTRUE);
  
  return kSUCCESS;
}

void PndGFTrackToPndTrackConvertorTask::Exec(Option_t* opt)
{
  if (fVerbose>0) std::cout<<"PndGFTrackToPndTrackConvertorTask::Exec"<<std::endl;
  
  fOutTrackArray->Delete();
  
  Int_t ntracks=fInTrackArray->GetEntriesFast();
  
  // Detailed output
  if (fVerbose>1) std::cout << " -I- PndGFTrackToPndTrackConvertorTask: contains " << ntracks << " Tracks."<< std::endl;
  
  
  for(Int_t itr=0;itr<ntracks;++itr)
  {
    if (fVerbose>1) std::cout<<"starting track"<<itr<<std::endl;
    
    TClonesArray& trkRef = *fOutTrackArray;
    Int_t size = trkRef.GetEntriesFast();

	GFTrack *inTrack = (GFTrack*) fInTrackArray->At(itr);
  try 
    {
		PndTrack* outTrack = GenfitTrack2PndTrack(inTrack);
		PndTrack* pndTrack = new(trkRef[size]) PndTrack(outTrack->GetParamFirst(), outTrack->GetParamLast(), outTrack->GetTrackCand(),
                                                    outTrack->GetFlag(), outTrack->GetChi2(), outTrack->GetNDF(), outTrack->GetPidHypo(), itr, FairRootManager::Instance()->GetBranchId(fTrackInBranchName));
    }
  catch(GFException& e)
    {
      std::cout << "*** PndGFTrackToPndTrackConvertorTask::Exec" << "\t" << "Genfit Exception: " << e.what() << std::endl;
	  continue;
      //throw e;
    }

  }
  
  return;
}

ClassImp(PndGFTrackToPndTrackConvertorTask);
