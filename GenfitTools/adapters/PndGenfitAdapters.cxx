#include"PndGenfitAdapters.h"

#include <iostream>

#include"GFTrack.h"
#include"GFAbsTrackRep.h"
#include"GFTrackCand.h"
#include"PndTrack.h"
#include"PndTrackCand.h"
#include"GFDetPlane.h"
#include"GFException.h"
#include"TMatrixT.h"
#include"FairTrackParP.h"

#include"GeaneTrackRep.h"
#include"RKTrackRep.h"
#include <cmath>

PndTrackCand* GenfitTrackCand2PndTrackCand(const GFTrackCand* cand){
  PndTrackCand* retVal = new PndTrackCand();
  unsigned int nhits = cand->getNHits();
  unsigned detId,hitId;
  double rho;
  for(unsigned int i=0;i<nhits;++i){
    cand->getHit(i,detId,hitId,rho);
    retVal->AddHit(detId,hitId,rho);
  }
  retVal->setMcTrackId(cand->getMcTrackId());
  //retVal->setTrackSeed(cand->getPosSeed(),
  //		       cand->getDirSeed(),
  //		       cand->getQoverPseed());
  retVal->setTrackSeed(cand->getPosSeed(),
  		       cand->getMomSeed(),
  		       cand->getChargeSeed());

  return retVal;
}

GFTrackCand* PndTrackCand2GenfitTrackCand(PndTrackCand* cand){
  GFTrackCand* retVal = new GFTrackCand();
  unsigned int nhits = cand->GetNHits();
  for(unsigned int i=0;i<nhits;++i){
    PndTrackCandHit candHit = cand->GetSortedHit(i);
    retVal->addHit(candHit.GetDetId(),candHit.GetHitId(),candHit.GetRho(),i);
  }
  retVal->setMcTrackId(cand->getMcTrackId());
  //retVal->setTrackSeed(cand->getPosSeed(),
  //		       cand->getDirSeed(),
  //		       cand->getQoverPseed());
  retVal->setPosMomSeed(cand->getPosSeed(),
  		       cand->getMomSeed(),
  		       cand->getChargeSeed());
#if 0
  // Poor man's choice; do better later; for +/-1 charge should work fine;
  {
    //printf("%f\n", cand->getQoverPseed()); exit(0);
    // Be dumb; regularize in whatever way does not cause core dump;
    double mom = fabs(cand->getQoverPseed()) ? 1./fabs(cand->getQoverPseed()) : 1E6;
    TVector3 pseed = mom * cand->getDirSeed();

    retVal->setPosMomSeed(cand->getPosSeed(), pseed, cand->getQoverPseed() >= 0 ? 1. : -1.);
  }
#endif

  return retVal;
}

PndTrack* GenfitTrack2PndTrack(const GFTrack* tr){
  GFAbsTrackRep* clone = tr->getCardinalRep()->clone();
  TMatrixT<double> firstState = clone->getFirstState();
  TMatrixT<double> lastState = clone->getLastState();
  TMatrixT<double> firstCov = clone->getFirstCov();
  TMatrixT<double> lastCov = clone->getLastCov();
  GFDetPlane firstPlane = clone->getFirstPlane();
  GFDetPlane lastPlane = clone->getLastPlane();
  
  GFAbsTrackRep* gtr;
  if (dynamic_cast<GeaneTrackRep*>(clone)!=NULL)
	  gtr = dynamic_cast<GeaneTrackRep*>(clone);
  else if (dynamic_cast<RKTrackRep*>(clone)!=NULL)
	  gtr = dynamic_cast<RKTrackRep*>(clone);
  else {
    std::cerr << " GenfitGFAbsTrackRep2PndTrack() can currently only handle GeaneTrackRep and RKTrackRep" << std::endl;
    throw;
  }

  //make the FairTrackParP for first and last hit
  double firstCova[15];
  int count=0;;
  for(int i=0; i<5;++i){
     for(int j=i;j<5;++j){
	   firstCova[count++]=firstCov[i][j];
     }
  }
  double lastCova[15];
  count=0;;
  for(int i=0; i<5;++i){
    for(int j=i;j<5;++j){
	  lastCova[count++]=lastCov[i][j];
    }
  }

  //  calculation of spu = sign[p(DJ x DK)]
  double first_pro(0), last_pro(0), first_spu, last_spu;
  bool exc(false);

  try{
    first_pro = gtr->getMom(firstPlane).Dot(firstPlane.getNormal());
    last_pro = gtr->getMom(lastPlane).Dot(lastPlane.getNormal());
  }
  catch (GFException& e){
    exc=true;
    std::cerr<<"could not convert GenfitTrack to PndTrack"<<std::endl;
    e.what();
  }

  first_spu = (first_pro>=0) ? 1 : -1;
  last_spu = (last_pro>=0) ? 1 : -1;
    
  FairTrackParP first(firstState[3][0],firstState[4][0],firstState[1][0],firstState[2][0],firstState[0][0],firstCova,firstPlane.getO(),firstPlane.getU(),firstPlane.getV(),first_spu);
  FairTrackParP last(lastState[3][0],lastState[4][0],lastState[1][0],lastState[2][0],lastState[0][0],lastCova,lastPlane.getO(),lastPlane.getU(),lastPlane.getV(),last_spu);
    
  //copy the trackCand
  GFTrackCand genfitCand = tr->getCand();
  PndTrackCand* pndCand = GenfitTrackCand2PndTrackCand(&genfitCand);
  PndTrack* retVal =  new PndTrack(first,last,*pndCand);
  retVal->SetChi2(tr->getChiSqu());
  retVal->SetNDF(tr->getNDF());
  if (tr->getNDF()==0 || exc) {
    retVal->SetFlag(-1);
  }
  else {
	  retVal->SetFlag(1);
  }

  delete pndCand;
  return retVal;
  
}
