//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Implementation of class PrimSelector
//      see PrimSelector.hh for details
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
//
//-----------------------------------------------------------

// Panda Headers ----------------------

// This Class' Header ------------------
#include "PrimSelector.h"

// C/C++ Headers ----------------------
#include <vector>
#include <map>
#include <iostream>

// Collaborating Class Headers --------
#include "FairRootManager.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TLorentzVector.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TFile.h"
#include "GFTrack.h"
#include "GFTrackCand.h"
#include "GFException.h"
#include "GFAbsTrackRep.h"
//#include "trackProximity.h"

// Class Member definitions -----------

using namespace::std;

PrimSelector::PrimSelector()
  : FairTask("PrimSelector"), _persistence(kFALSE), _trackBranchName("TrackPreFit")
{
}

PrimSelector::~PrimSelector()
{}


InitStatus
PrimSelector::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
    {
      Error("PrimSelector::Init","RootManager not instantiated!");
      return kERROR;
    }
  
  // Get input collection
  _trackArray=(TClonesArray*) ioman->GetObject(_trackBranchName);
  
  if(_trackArray==0)
    {
      Error("PrimSelector::Init","Track-array not found!");
      return kERROR;
    }

  _pocaArray=new TClonesArray("TVector3");
  ioman->Register("PrimPoca","PrimSelector",_pocaArray,_persistence);

  // missuse of TVector3 to store the charge....
  _chargeArray=new TClonesArray("TVector3");
  ioman->Register("PrimCharge","PrimSelector",_chargeArray,_persistence);



  _primArray=new TClonesArray("TLorentzVector");
  ioman->Register("Mom","PrimSelector",_primArray,_persistence);

  fEventCounter=0;
  fNFullEvents=0;
  fNPhysTracks=0;
  fNBkgTracks=0;
  

  return kSUCCESS;
}

void
PrimSelector::Exec(Option_t* opt)
{
  std::cout << "PrimSelector::Exec" << std::endl;

  // Reset output Arrays
  if(_pocaArray==0) Fatal("PrimSelector::Exec)","No pocaArray");
  _pocaArray->Delete();
  if(_chargeArray==0) Fatal("PrimSelector::Exec)","No chargeArray");
  _chargeArray->Delete();
  if(_primArray==0) Fatal("PrimSelector::Exec)","No primArray");
  _primArray->Delete();
  int ntrks=_trackArray->GetEntriesFast();
  
  map<unsigned int, int> trackIds; 

  TVector3 IP(0,0,0);
  for(int itrk=0;itrk<ntrks;++itrk){
    GFTrack* trk=(GFTrack*)_trackArray->At(itrk);
    if(trk->getMom().Mag()<0.1) continue;
  
    GFAbsTrackRep* rep = trk->getCardinalRep();
    TVector3 poca, dirInPoca;
    try {
      rep->extrapolateToPoint(IP, poca, dirInPoca);
    }
    catch(GFException& ex) {
      if(fVerbose)std::cout<<ex.what()<<std::endl;
      //continue;
    }
    
    // record mctruth info
    if(trk->getCand().getMcTrackId()<10000){
      ++fNPhysTracks;
      trackIds[trk->getCand().getMcTrackId()]+=1;
    }
    else ++fNBkgTracks;
   


    new((*_chargeArray)[_chargeArray->GetEntriesFast()]) TVector3(trk->getCharge(),0,0);
    // store positions to cut on in analysis
    Int_t size=_pocaArray->GetEntriesFast();
    new((*_pocaArray)[size]) TVector3(poca);

    // build 4-vector
    TVector3 p3=rep->getMom();
    TLorentzVector pion; 
    // make mass hypothesis here
    pion.SetXYZM(p3.X(),p3.Y(),p3.Z(),0.13957018);
    
    Int_t size2=_primArray->GetEntriesFast();
    new((*_primArray)[size2]) TLorentzVector(pion);
  
  } // end loop over tracks

  if(trackIds.size()>=fNExpectedTracks)++fNFullEvents;

  ++fEventCounter;
} 


void 
PrimSelector::FinishTask(){
  TFile* file=FairRootManager::Instance()->GetOutFile();
  file->mkdir("PrimSelector");
  file->cd("PrimSelector");
   double singleTrackEff=(double)fNPhysTracks/double(fNExpectedTracks*fEventCounter);
  double singleTrackPurity=1. -  (double)fNBkgTracks/(double)(fNBkgTracks+fNPhysTracks);
 
  double fullEvtEff= (double)fNFullEvents/(double)fEventCounter;

   TH1D* h=new TH1D("hEffPrim","Prim: nbkg nphys nfullevents nevent",4,0,4);
   h->Fill(0.5,fNBkgTracks);
   h->Fill(1.5,fNPhysTracks);
   h->Fill(2.5,fNFullEvents);
   h->Fill(3.5,fEventCounter);
   h->Write();

  std::cout << "PrimSelector::FinishTask \n" 
	    << "    SingleTrackEfficiency: " << singleTrackEff << std::endl
	    << "    SingleTrackPurity: " << singleTrackPurity<< std::endl
	    << "    FullEventEfficiency: " << fullEvtEff<< std::endl;
    
  TGraph* gresults=new TGraph(3);
  gresults->SetName("hPrimMC");
  gresults->SetTitle("Efficiency Statistics by PrimSelector");

  gresults->SetPoint(0,0,singleTrackEff);
  gresults->SetPoint(1,1,singleTrackPurity);
  gresults->SetPoint(2,2,fullEvtEff);
  
  gresults->Write();
}

ClassImp(PrimSelector)
