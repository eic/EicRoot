/**
* \file CbmRichTestSim.cxx
*
* \author Claudia Hoehne
* \date 2006
**/

#include "CbmRichTestSim.h"

#include "CbmRichPoint.h"
#include "CbmGeoRichPar.h"

#include "FairTask.h"
#include "FairRootManager.h"
#include "CbmMCTrack.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairBaseParSet.h"
#include "FairGeoNode.h"

#include "TClonesArray.h"
#include "TVector3.h"
#include "TH2D.h"
#include "TH1D.h"

#include <map>
#include <iostream>

using std::cout;
using std::endl;
using std::map;


CbmRichTestSim::CbmRichTestSim():
   FairTask("CbmRichTestSim"),
   fNEvents(0),
   fMCRichPointArray(NULL),
   fMCTrackArray(NULL),

   fh_Det1ev(NULL),
   fh_Det1ev_zoom(NULL),
   fh_Detall(NULL),
   fh_n_vs_p(NULL),
   fh_v_el(NULL),

   fh_Nall(NULL),
   fh_Nel(NULL),
   fh_Nelprim(NULL),
   fh_Npi(NULL),
   fh_Nk(NULL),
   fh_Nhad(NULL),

   fSensNodes(NULL),
   fPar(NULL),
   fDetZ(0.)
{
   fh_Det1ev = new TH2D("fh_Det1ev","points in detector plane (1 event)",170,-170,170,250,-250,250);
   fh_Det1ev_zoom = new TH2D("fh_Det1ev_zoom","points in detector plane (1 event, zoom in)",100,10,60,100,100,150);
   fh_Detall = new TH2D("fh_Detall","points in detector plane (all events)",170,-170,170,250,-250,250);
   fh_n_vs_p  = new TH2D("fh_n_vs_p","Npoints versus momentum",150,0,15,100,0,400);
   fh_v_el = new TH2D("fh_v_el","(y,z) of production vertex of electrons",230,0,460,300,-300,300);

   fh_Nall = new TH1D("fh_Nall","Number of all rings in RICH",150,0,150);
   fh_Nel = new TH1D("fh_Nel","Number of electron rings in RICH",150,0,150);
   fh_Nelprim = new TH1D("fh_Nelprim","Number of electron (STS>6) rings in RICH",150,0,150);
   fh_Npi = new TH1D("fh_Npi","Number of pi rings in RICH",150,0,150);
   fh_Nk = new TH1D("fh_Nk","Number of K rings in RICH",150,0,150);
   fh_Nhad = new TH1D("fh_Nhad","Number of hadrons crossing PMT plane",50,0,50);
}

CbmRichTestSim::~CbmRichTestSim()
{
   // write histograms to a file
   fh_Det1ev->Write();
   fh_Det1ev_zoom->Write();
   fh_Detall->Write();
   fh_n_vs_p->Write();
   fh_v_el->Write();

   fh_Nall->Write();
   fh_Nel->Write();
   fh_Nelprim->Write();
   fh_Npi->Write();
   fh_Nk->Write();
   fh_Nhad->Write();
}

void CbmRichTestSim::SetParContainers()
{
   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();
   fPar=(CbmGeoRichPar*)(rtdb->getContainer("CbmGeoRichPar"));
}

InitStatus CbmRichTestSim::Init()
{
   fSensNodes = fPar->GetGeoSensitiveNodes();
   // get detector position (z):
   FairGeoNode *det= dynamic_cast<FairGeoNode*> (fSensNodes->FindObject("rich1d#1"));
   if ( NULL == det) Fatal("CbmRichTestSim::Init", "no RICH Geo Node  found!");
   FairGeoTransform* detTr = det->getLabTransform(); // detector position in labsystem
   FairGeoVector detPosLab = detTr->getTranslation(); // ... in cm
   FairGeoTransform detCen = det->getCenterPosition(); // center in Detector system
   FairGeoVector detPosCen = detCen.getTranslation();
   fDetZ = detPosLab.Z() + detPosCen.Z(); // z coordinate of photodetector (Labsystem, cm)

   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmRichTestSim::Init()", "RootManager not instantised!");}

   fMCTrackArray = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMCTrackArray) { Fatal("CbmRichTestSim::Init()", "No MCTrack array!");}

   fMCRichPointArray  = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( NULL == fMCRichPointArray) { Fatal("CbmRichTestSim::Init()", "No RichPoint array!");}

   return kSUCCESS;
}

void CbmRichTestSim::Exec(
      Option_t* option)
{
   fNEvents++;

   // define some variables
   TVector3 position;
   TVector3 momentum;
   TVector3 vertex;

   Int_t Nhad, Nring, Nel, Nel_prim, Npi, Nk;
   Nhad = Nring = Nel = Nel_prim = Npi = Nk = 0;

   map<Int_t,Int_t> pointMap;

   Int_t nPoints = fMCRichPointArray->GetEntriesFast();
   Int_t nMCTracks = fMCTrackArray->GetEntriesFast();
   for(Int_t iPoint=0; iPoint<nPoints; iPoint++){
      CbmRichPoint* pPoint= (CbmRichPoint*)fMCRichPointArray->At(iPoint);
      if( NULL == pPoint ) continue;
      pPoint->Position(position);
      fh_Detall->Fill(position.X(),position.Y());

      if (fNEvents == 1) {
         fh_Det1ev->Fill(position.X(),position.Y());
         fh_Det1ev_zoom->Fill(position.X(),position.Y());
      }
      if (pPoint->GetTrackID() < 0 ) continue;
      CbmMCTrack* pTrack = (CbmMCTrack*)fMCTrackArray->At(pPoint->GetTrackID());
      Int_t gcode = pTrack->GetPdgCode();
      if (gcode != 50000050) Nhad++;

      if (gcode == 50000050){
         pTrack->GetStartVertex(vertex);
         Int_t motherID = pTrack->GetMotherId();
         if (motherID == -1) continue;
         pTrack = (CbmMCTrack*)fMCTrackArray->At(motherID);
         pointMap[motherID]++;
       } // select cherenkov photons
   } // iPoint

   for (Int_t iMCTrack=0; iMCTrack<nMCTracks; iMCTrack++) {
      CbmMCTrack* pTrack = (CbmMCTrack*) fMCTrackArray->At(iMCTrack);
      if ( NULL == pTrack ) continue;
      Int_t Npoints = pointMap[iMCTrack];
      if (Npoints){
         Nring++;
         pTrack->GetMomentum(momentum);
         fh_n_vs_p->Fill(momentum.Mag(),Npoints);
         pTrack->GetStartVertex(vertex);
         Int_t gcode = pTrack->GetPdgCode();
         if (TMath::Abs(gcode) == 11) {
            Nel++;
            fh_v_el->Fill(vertex.Z(),vertex.Y());
            if (pTrack->GetNPoints(kSTS) > 5) Nel_prim++;
         } // electrons
         if (TMath::Abs(gcode) == 211) Npi++;
         if (TMath::Abs(gcode) == 321) Nk++;
      } // tracks with points in Rich
   } // iMCTrack

   fh_Nall->Fill(Nring);
   fh_Nel->Fill(Nel);
   fh_Nelprim->Fill(Nel_prim);
   fh_Npi->Fill(Npi);
   fh_Nk->Fill(Nk);
   fh_Nhad->Fill(Nhad);

   if (fVerbose){
   cout << "--------------------------------------------------------------------------" << endl;
   cout << "-----------               Test Rich Simulation                 -----------" << endl;
   cout << endl;
   cout << " Number of particles in RICH detector  ----- event number  " << fNEvents << endl;
   cout << " hadrons in RICH (no Cherenkov photons) = " << Nhad << endl;
   cout << " total rings        = " << Nring << endl;
   cout << " electrons          = " << Nel << endl;
   cout << " electrons (STS>=6) = " << Nel_prim << endl;
   cout << " pions              = " << Npi << endl;
   cout << " Kaons              = " << Nk << endl;
   cout << endl;
   cout << "--------------------------------------------------------------------------" << endl;
   }
}

void CbmRichTestSim::Finish()
{
   fMCTrackArray->Clear();
   fMCRichPointArray->Clear();
}

ClassImp(CbmRichTestSim)

