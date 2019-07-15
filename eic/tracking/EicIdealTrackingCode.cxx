//
//  //AYK, 2013/02/28:
//
//    Well, there was no easy way to inherit from any generic enough PandaRoot class,
//    so considered to copy over PndFtsTrackerIdeal.h(cxx) and customize/generalize them
//    to my taste; will actually have to look more attentively into these codes later
//    and clean them up; in particular figure out how to get rid of "PndTrack" prefices;
//

#include "PndMCTrack.h"
#include "PndTrackCand.h"
#include "PndTrack.h"

#include "FairTrackParP.h"
#include "FairMCPoint.h"
#include "FairHit.h"
#include "FairTask.h"
#include "FairRootManager.h"

#include "TObjectTable.h"
#include "TClonesArray.h"
#include "TParticlePDG.h"
#include "TRandom3.h"
#include <iostream>

#include <EicDetName.h>
#include "EicIdealTrackingCode.h"

// -----------------------------------------------------------------------------------------------

EicIdealTrackingCode::EicIdealTrackingCode():
  FairTask("EIC Ideal Tracking Code"), fMCTracks(new TClonesArray()), 
  fTrackIds(new TClonesArray()), fMomSigma(0,0,0), fDPoP(0.), fRelative (kFALSE), 
  fVtxSigma(0,0,0), fEfficiency(1.), 
  fTracksArrayName("EicIdealTrack"), pdg(0)  
{
  fTrackCands = new TClonesArray("PndTrackCand");
  fTracks = new TClonesArray("PndTrack");
  fVerbose = 0;
} // EicIdealTrackingCode::EicIdealTrackingCode()

// -----------------------------------------------------------------------------------------------

EicIdealTrackingCode::~EicIdealTrackingCode() 
{  
  FairRootManager *fManager =FairRootManager::Instance();
  fManager->Write();
} // EicIdealTrackingCode::~EicIdealTrackingCode() 

// -----------------------------------------------------------------------------------------------

void EicIdealTrackingCode::Register() 
{
  FairRootManager::Instance()->Register(fTracksArrayName,"EicTrk", fTracks, kTRUE);
  FairRootManager::Instance()->Register(fTracksArrayName+"Cand","EicTrk", fTrackCands, kTRUE);
  if(fVerbose>3) Info("Register","Done.");
} // EicIdealTrackingCode::Register() 

// -----------------------------------------------------------------------------------------------

int EicIdealTrackingCode::AddDetectorGroup(char *name)
{
  EicDetectorGroup group(name); 

  fGroups.push_back(group);

  return 0;
} // EicIdealTrackingCode::AddDetectorGroup()

// -----------------------------------------------------------------------------------------------

InitStatus EicIdealTrackingCode::Init() 
{
  if(fVerbose>3) Info("Init","Start initialisation.");
  
  FairRootManager *fManager = FairRootManager::Instance();
  
  // Get MC track array;
  fMCTracks = (TClonesArray*)fManager->GetObject("MCTrack");
  if ( ! fMCTracks ) {
    std::cout << "-W-  EicIdealTrackingCode::Init: No MCTrack array! Needed for MC Truth" << std::endl;
    return kERROR;
  }

  // Get detector-specific MC & digi arrays;
  for (std::vector<EicDetectorGroup>::iterator it=fGroups.begin(); it!=fGroups.end(); it++)
  { 
    it->_fMCPoints = (TClonesArray*)fManager->GetObject(it->dname->Name() + "MoCaPoint");
    if ( ! it->_fMCPoints ) {
      std::cout << "-W-  EicIdealTrackingCode::Init: No " << it->dname->Name() << "Point array!" << std::endl;
      return kERROR;
    }

    //it->_fHits = (TClonesArray *)fManager->GetObject(it->dname->Name() + "DigiHit");
    it->_fHits = (TClonesArray *)fManager->GetObject(it->dname->Name() + "TrackingDigiHit");
    if ( ! it->_fHits ) {
      std::cout << "-W-  EicIdealTrackingCode::Init: No " << it->dname->Name() << "Hit array!" << std::endl;
      return kERROR;
    }

    //it->_fBranchID = FairRootManager::Instance()->GetBranchId(it->dname->Name() + "DigiHit");
    it->_fBranchID = fManager->GetBranchId(it->dname->Name() + "TrackingDigiHit");

    // And create direct access lookup table entries; yes, all this looks like back 
    // doors into the original FairRoot codes;
    fManager->AddMoCaLookupEntry(it->_fBranchID, it->_fMCPoints);
    fManager->AddDigiLookupEntry(it->_fBranchID, it->_fHits);
  } //for it
  
  if(fVerbose>3) Info("Init","Fetched all arrays.");
  
  Register();
  
  pdg = new TDatabasePDG();
  if(fVerbose>3) Info("Init","End initialisation.");
  
  return kSUCCESS;
} // EicIdealTrackingCode::Init() 

// -----------------------------------------------------------------------------------------------

void EicIdealTrackingCode::Exec(Option_t * option) 
{
  Reset();
  if(fVerbose>3) Info("Exec","Start eventloop.");

  if(fVerbose>4) Info("Exec","Print some array properties");

  FairHit* ghit = NULL;
  std::map<Int_t, FairHit*> firstHit;
  std::map<Int_t, FairHit*> lastHit;
  FairMCPoint* myPoint=NULL;
  std::map<Int_t, FairMCPoint*> firstPoint;
  std::map<Int_t, FairMCPoint*> lastPoint;
  std::map<Int_t, PndTrackCand*> candlist;

  // Loop through all detector groups (TRS, FGT, etc);
  for (std::vector<EicDetectorGroup>::iterator it=fGroups.begin(); it!=fGroups.end(); it++)
  { 
    for (Int_t ih = 0; ih < it->_fHits->GetEntriesFast(); ih++) {
      ghit = (FairHit*) it->_fHits->At(ih);
      if(!ghit) {
        if(fVerbose>3) Error("Exec","Have no ghit %i, array size: %i",ih, it->_fHits->GetEntriesFast());
        continue;
      }
      Int_t mchitid=ghit->GetRefIndex();
      //printf("%d\n", mchitid);
      if(mchitid<0) {
        if(fVerbose>3) Error("Exec","Have a mcHit %i",mchitid);
        continue;
      }
      myPoint = (FairMCPoint*)(it->_fMCPoints->At(mchitid));
      if(!myPoint) continue;
      Int_t trackID = myPoint->GetTrackID();
      if(trackID<0) continue;
      
      if(fVerbose>5) Info("Exec","Have a Hit %i at Track index %i",ih,trackID);
      if(fVerbose>5) Info("Exec","  --> mchitid %i",mchitid);
      
      PndTrackCand* cand=candlist[trackID];
      if(NULL==cand){
        if(fVerbose>5) Info("Exec","Create new PndTrack object %i",trackID);
        cand=new PndTrackCand();
        cand->setMcTrackId(trackID);
        if(fVerbose>5) Info("Exec","Create new PndTrack object finished %i",trackID);
      }
      if(fVerbose>5) Info("Exec","add the hit %i to trackcand %i",ih,trackID);
#if _THINK_
      // Well, it looks like distance from the IP should be a good guess on hit 
      // ordering parameter; not the best idea -> THINK later, need much better algorithm 
      // (order along the trajectory, whatever this means);
      //cand->AddHit(it->_fBranchID,ih,ghit->GetZ());
#endif
      //printf("%3d -> %7.2f -> %7.2f %7.2f %7.2f\n", ih, myPoint->GetTime(), 
      //     ghit->GetX(), ghit->GetY(), ghit->GetZ());
      cand->AddHit(it->_fBranchID,ih,sqrt(ghit->GetX()*ghit->GetX() + 
					  ghit->GetY()*ghit->GetY() + ghit->GetZ()*ghit->GetZ()));
      //cand->AddHit(it->_fBranchID, ih, myPoint->GetTime());
      //cand->AddHit(it->_fBranchID,ih,sqrt(ghit->GetX()*ghit->GetX() + ghit->GetY()*ghit->GetY()));
      if(!firstHit[trackID] || firstHit[trackID]->GetZ() > ghit->GetZ()) {
        firstHit[trackID]=ghit;
        firstPoint[trackID]=myPoint;
      }
      if(!lastHit[trackID] || lastHit[trackID]->GetZ() < ghit->GetZ()) {
        lastHit[trackID]=ghit;
        lastPoint[trackID]=myPoint;
      }
      
      candlist[trackID] = cand; // set
    }
  }
  if(fVerbose>3) Info("Exec","Insert to TCA (depending on efficiency)");
  
  // re-iterate and select by efficiency & number of hits
  std::map<Int_t, PndTrackCand*>::iterator  candit;
  PndMCTrack *mc=NULL;
  TVector3 svtx, smom;
  Int_t charge=0, trackID=-1;
  
  for(candit=candlist.begin(); candit!=candlist.end(); ++candit) {
    PndTrackCand* tcand=candit->second; 
    trackID=candit->first;
    if(!tcand) {
      if(fVerbose>3) Warning("Exec","Have no candidate at %i",trackID);
      continue;
    }
    if( tcand->GetNHits() < 3 ) continue;
    if(0 < fEfficiency && fEfficiency < 1){
      if(gRandom->Rndm() > fEfficiency) continue;
    }
    tcand->Sort();
    mc = (PndMCTrack*)fMCTracks->At(trackID);
    if (mc->GetPdgCode()<100000000) 
      charge = (Int_t)TMath::Sign(1.0, ((TParticlePDG*)pdg->GetParticle(mc->GetPdgCode()))->Charge());
    else charge = 1;
    tcand->setMcTrackId(trackID);
    // prepare track parameters
    firstHit[trackID]->Position(svtx); // set position to first hit
    SmearFWD(svtx, fVtxSigma);
    firstPoint[trackID]->Momentum(smom);
    if (fRelative) fMomSigma=fDPoP*smom;
    SmearFWD(smom, fMomSigma);
    //FairTrackParP* firstPar=new FairTrackParP(svtx, smom,
    //                                      fVtxSigma, fMomSigma,
    //                                      charge, svtx,
    //                                      TVector3(1.,0.,0.), TVector3(0.,1.,0.));
    FairTrackParP firstPar(svtx, smom,
    			   fVtxSigma, fMomSigma,
    			   charge, svtx,
    			   TVector3(1.,0.,0.), TVector3(0.,1.,0.));					 
        
    lastHit[trackID]->Position(svtx);
    SmearFWD(svtx, fVtxSigma);
    lastPoint[trackID]->Momentum(smom);
    SmearFWD(smom, fMomSigma);
    //FairTrackParP* lastPar=new FairTrackParP(svtx, smom,
    //                                     fVtxSigma, fMomSigma,
    //                                     charge, svtx,
    //                                     TVector3(1.,0.,0.), TVector3(0.,1.,0.));	
    FairTrackParP lastPar(svtx, smom,
    			  fVtxSigma, fMomSigma,
    			  charge, svtx,
    			  TVector3(1.,0.,0.), TVector3(0.,1.,0.));	    
    
    if(fVerbose>3) Info("Exec","Store candidate at %i",trackID);
    // Creates a new hit in the TClonesArray.
    if(fVerbose>3) Info("AddTrack","Adding a Track.");
    TClonesArray &pndtracks = *fTracks;
    TClonesArray &pndtrackcands = *fTrackCands;
    //  TClonesArray &pndtrackids = *fTrackIds;
    Int_t size = pndtrackcands.GetEntriesFast();
    if(pndtracks.GetEntriesFast() != size) {
      Error("Exec","Arrays out of synchronisation: %i tracks, %i cands. Abort event."
            ,pndtracks.GetEntriesFast(),pndtrackcands.GetEntriesFast());
      return;
    }
    
#if 1
    PndTrackCand* pndTrackCand = new(pndtrackcands[size]) PndTrackCand(*tcand);
    PndTrack* pndTrack = new(pndtracks[size]) 
      PndTrack(firstPar, lastPar, *tcand,0,0,1,mc->GetPdgCode(),trackID,
             FairRootManager::Instance()->GetBranchId("MCTrack"));
    //PndTrack(firstPar, lastPar, *tcand,0,0,1,mc->GetPdgCode(),trackID,
    //	       FairRootManager::Instance()->GetBranchId("MCTrack"));
#endif
  }

  // Clean up candidate list;
  for(candit=candlist.begin(); candit!=candlist.end(); ++candit) 
    delete candit->second;
  {
    //static unsigned counter;
    //printf("%6d\n", counter++);
  } 
  
  if(fVerbose>3) Info("Exec","End eventloop.");
} // EicIdealTrackingCode::Exec()

// -----------------------------------------------------------------------------------------------

void EicIdealTrackingCode::Finish() 
{
  std::cout << " Found  "<< fTracks->GetEntriesFast() << " tracks\n";
} // EicIdealTrackingCode::Finish()

// -----------------------------------------------------------------------------------------------

void EicIdealTrackingCode::Reset() 
{
  //for(unsigned iq=0; iq<fTracks->GetEntriesFast(); iq++) {
  //delete (PndTrack*)(fTracks->At(iq));
  //fTracks[iq] = 0;
  //}
  //for(unsigned iq=0; iq<fTrackCands->GetEntriesFast(); iq++)
  //delete fTrackCands->At(iq);

  //if (fTracks->GetEntriesFast() != 0)  fTracks->Clear();
  if (fTracks->GetEntriesFast() != 0)  fTracks->Delete();
  //if (fTrackCands->GetEntriesFast() != 0)  fTrackCands->Clear();
  if (fTrackCands->GetEntriesFast() != 0)  fTrackCands->Delete();
} // EicIdealTrackingCode::Reset()

// -----------------------------------------------------------------------------------------------

void EicIdealTrackingCode::SmearFWD(TVector3 &vec, const TVector3 &sigma)
{
  // gaussian smearing
  Double_t rannn=0.;
  rannn = gRandom->Gaus(vec.X(),sigma.X());
  vec.SetX(rannn);
  
  rannn = gRandom->Gaus(vec.Y(),sigma.Y());
  vec.SetY(rannn);
  
  rannn = gRandom->Gaus(vec.Z(),sigma.Z());
  vec.SetZ(rannn);
  
  return;
} // EicIdealTrackingCode::SmearFWD()

// -----------------------------------------------------------------------------------------------

ClassImp(EicIdealTrackingCode)

