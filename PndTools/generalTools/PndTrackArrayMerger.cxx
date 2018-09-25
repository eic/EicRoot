//
//  This class COPIES PndTrack objects from all specified input branches
//  into one output branch. Linking is still to be done...
//
//  Aurthor: R.Kliemt, May 2011

#include "PndTrackArrayMerger.h"
#include "PndTrackCand.h"
#include "PndTrack.h"
#include <iostream>


PndTrackArrayMerger::PndTrackArrayMerger()
:fPersistance(kTRUE), fOutputBranch("ALLTracks"), fOutputArray(), fInputArrayList(), fInputBranchList()
{}

PndTrackArrayMerger::PndTrackArrayMerger(TString s)
:fPersistance(kTRUE), fOutputBranch(s), fOutputArray(), fInputArrayList(), fInputBranchList()
{}

PndTrackArrayMerger::~PndTrackArrayMerger()
{}

void PndTrackArrayMerger::SetParContainers()
{return;}

InitStatus PndTrackArrayMerger::ReInit()
{return kSUCCESS;}


InitStatus PndTrackArrayMerger::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman )
  {
    std::cout << "-E- PndSdsStripHitProducer::Init: "
    << "RootManager not instantiated!" << std::endl;
    return kFATAL;
  }
  
  //setup input arrays
  TClonesArray* tmparray;
  for(std::vector<TString>::iterator iter = fInputBranchList.begin(); iter!=fInputBranchList.end();++iter)
  {
    tmparray = (TClonesArray*) ioman->GetObject((*iter).Data());
    if ( ! tmparray )
    {
      Error("Init()","No %s array! Skipping that name.",(*iter).Data());
      continue;
    }
    TString namebuff = (TString) tmparray->GetClass()->GetName();
    if(namebuff == "PndTrack"){
      fInputArrayList.push_back(tmparray);    
    }
  }
  
  //setup output array
  fOutputArray = ioman->Register(fOutputBranch, "PndTrack", "AllTracks", fPersistance);
 //fOutputArray = new TClonesArray("PndTrack");
 // ioman->Register(fOutputBranch, "AllTracks", fOutputArray, fPersistance);
  
  return kSUCCESS;
}

void PndTrackArrayMerger::Exec(Option_t* opt)
{
  fOutputArray->Clear();
  // copy data from input arrays to output array
  TClonesArray* tmparray;
  PndTrack* tmptrk;
  Int_t namenum=0;
  TString brname;
  Int_t entries=0;
  
  
  for(std::vector<TClonesArray*>::iterator iter = fInputArrayList.begin(); iter!=fInputArrayList.end();++iter)
  {
    tmparray=*iter;
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,29,1)
  // MOVES References into a new TCA
  // Just from root 5.29.02
  if (tmparray != 0) {
    fOutputArray->AbsorbObjects(tmparray, 0, tmparray->GetEntries() - 1);
  }
#else
    brname=fInputBranchList[namenum];
    for ( Int_t i=0;i<tmparray->GetEntriesFast();i++)
    {
      tmptrk=(PndTrack*)tmparray->At(i);
      entries=fOutputArray->GetEntriesFast();
      PndTrack* mynewtrack = new ((*fOutputArray)[entries]) 
           PndTrack(tmptrk->GetParamFirst(),tmptrk->GetParamLast(),*(tmptrk->GetTrackCandPtr()),
                    tmptrk->GetFlag(), tmptrk->GetChi2(), tmptrk->GetNDF(), 
                    tmptrk->GetPidHypo(),-1,-1);
      mynewtrack->Reset(); //resetting links 
      for(int nlin=0;nlin<tmptrk->GetNLinks();nlin++)
      {
        mynewtrack->AddLink(tmptrk->GetLink(nlin));
      }
    }
    namenum++;
#endif
  }

  return;
}

void PndTrackArrayMerger::FinishEvent()
{
  // called after all Tasks did their Exex() and the data is copied to the file
//  fOutputArray->Clear();
  FinishEvents();
}

ClassImp(PndTrackArrayMerger);

