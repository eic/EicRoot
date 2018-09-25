/**
* \file CbmRichTestHits.cxx
*
* \author Claudia Hoehne
* \date 2006
**/

#include "CbmRichTestHits.h"

#include "CbmGeoRichPar.h"
#include "CbmRichPoint.h"
#include "CbmRichHit.h"

#include "FairTask.h"
#include "FairRootManager.h"
#include "CbmMCTrack.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairBaseParSet.h"
#include "FairGeoVolume.h"
#include "FairGeoTransform.h"
#include "FairGeoVector.h"
#include "FairGeoMedium.h"
#include "FairGeoNode.h"

#include "TClonesArray.h"
#include "TGraph.h"
#include "TVector3.h"
#include "TH2D.h"
#include "TH1D.h"

#include <map>
#include <iostream>

using std::cout;
using std::endl;
using std::map;

CbmRichTestHits::CbmRichTestHits():
   FairTask("CbmRichTestHits"),
   fRichHits(NULL),
   fRichPoints(NULL),
   fMcTracks(NULL),

   fNEvents(0),

   fh_Det1ev(NULL),
   fh_Det1ev_zoom(NULL),
   fh_Detall(NULL),
   fh_Detall_zoom(NULL),
   fh_n_vs_p(NULL),

   fh_Nhits(NULL),

   fh_Nall(NULL),
   fh_Nel(NULL),
   fh_Nelprim(NULL),
   fh_Npi(NULL),
   fh_Nk(NULL),
   fh_Nhad(NULL),
   fh_Nnoise(NULL),

   fSensNodes(NULL),
   fPar(NULL),
   fDetZ(0.)
{
   fh_Det1ev = new TH2D("fh_Det1ev","points in detector plane (1 event)",170,-170,170,250,-250,250);
   fh_Det1ev_zoom = new TH2D("fh_Det1ev_zoom","points in detector plane (1 event, zoom in)",100,10,60,100,60,110);
   fh_Detall = new TH2D("fh_Detall","points in detector plane (all events)",170,-170,170,250,-250,250);
   fh_Detall_zoom = new TH2D("fh_Detall_zoom","points in detector plane (all events, zoom in)",150,40,70,150,110,140);
   fh_n_vs_p  = new TH2D("fh_n_vs_p","Npoints versus momentum",150,0,15,70,0,70);

   fh_Nhits = new TH1D("fh_Nhits","Number of hits of e rings (STS>=6)",70,0,70);

   fh_Nall = new TH1D("fh_Nall","Number of all rings in RICH",150,0,150);
   fh_Nel = new TH1D("fh_Nel","Number of electron rings in RICH",150,0,150);
   fh_Nelprim = new TH1D("fh_Nelprim","Number of electron (STS>=6) rings in RICH",150,0,150);
   fh_Npi = new TH1D("fh_Npi","Number of pi rings in RICH",150,0,150);
   fh_Nk = new TH1D("fh_Nk","Number of K rings in RICH",150,0,150);
   fh_Nhad = new TH1D("fh_Nhad","Number of hadrons crossing PMT plane",50,0,50);
   fh_Nnoise = new TH1D("fh_Nnoise","Number of noise hits PMT plane",100,0,2000);
}

CbmRichTestHits::~CbmRichTestHits()
{
   // write histograms to a file
   fh_Det1ev->Write();
   fh_Det1ev_zoom->Write();
   fh_Detall->Write();
   fh_Detall_zoom->Write();
   fh_n_vs_p->Write();

   fh_Nhits-> Write();

   fh_Nall->Write();
   fh_Nel->Write();
   fh_Nelprim->Write();
   fh_Npi->Write();
   fh_Nk->Write();
   fh_Nhad->Write();
   fh_Nnoise->Write();
}

void CbmRichTestHits::SetParContainers()
{
   // Get Base Container
   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb=ana->GetRuntimeDb();
   fPar=(CbmGeoRichPar*)(rtdb->getContainer("CbmGeoRichPar"));
}

InitStatus CbmRichTestHits::Init()
{
   fSensNodes = fPar->GetGeoSensitiveNodes();

   // get detector position (z):
   FairGeoNode *det= dynamic_cast<FairGeoNode*> (fSensNodes->FindObject("rich1d#1"));
   if (! det) cout << " -I no RICH Geo Node  found !!!!!  " << endl;
   FairGeoTransform* detTr=det->getLabTransform();  // detector position in labsystem
   FairGeoVector detPosLab=detTr->getTranslation(); // ... in cm
   FairGeoTransform detCen=det->getCenterPosition();  // center in Detector system
   FairGeoVector detPosCen=detCen.getTranslation();
   fDetZ = detPosLab.Z() + detPosCen.Z();   /** z coordinate of photodetector (Labsystem, cm) */

   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmRichTestHits::Init", "RootManager not instantised!");}

   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) { Fatal("CbmRichTestHits::Init", "No MCTrack array!");}

   fRichPoints  = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( NULL == fRichPoints) { Fatal("CbmRichTestHits::Init", "No RichPoint array!");}

   fRichHits  = (TClonesArray*) ioman->GetObject("RichHit");
   if ( NULL == fRichHits) { Fatal("CbmRichTestHits::Init", "No RichHit array!");}

   return kSUCCESS;
}

void CbmRichTestHits::Exec(
      Option_t* option)
{
   fNEvents++;

   TVector3 position;
   TVector3 momentum;
   TVector3 vertex;

   Int_t Nhad, Nring, Nel, Nel_prim, Npi, Nk, Nnoise;
   Nhad = Nring = Nel = Nel_prim = Npi = Nk = Nnoise = 0;

   map<Int_t,Int_t> hitMap;

   Int_t nHits = fRichHits->GetEntriesFast();
   Int_t nMCTracks = fMcTracks->GetEntriesFast();

   for(Int_t iHit=0; iHit<nHits; iHit++){
      CbmRichHit* pHit= (CbmRichHit*) fRichHits->At(iHit);
      if( NULL == pHit ) continue;

      pHit->Position(position);
      fh_Detall->Fill(position.X(),position.Y());
      fh_Detall_zoom->Fill(position.X(),position.Y());
      if (fNEvents == 1) {
         fh_Det1ev->Fill(position.X(),position.Y());
         fh_Det1ev_zoom->Fill(position.X(),position.Y());
      }

      // Noise hit
      if ((pHit->GetRefId()) < 0) {
         Nnoise++;
         continue;
      }
      CbmRichPoint* pPoint = (CbmRichPoint*) fRichPoints->At(pHit->GetRefId());
      CbmMCTrack* pTrack = (CbmMCTrack*)fMcTracks->At(pPoint->GetTrackID());
      Int_t gcode = pTrack->GetPdgCode();
      if (gcode != 50000050) Nhad++;
      if (gcode == 50000050){
         pTrack->GetStartVertex(vertex);
         Int_t motherID = pTrack->GetMotherId();
         if (motherID == -1) continue;

         pTrack = (CbmMCTrack*) fMcTracks->At(motherID);
         hitMap[motherID]++;
      } //select cherenkov photons
   } // point loop

   for (Int_t iMCTrack=0; iMCTrack<nMCTracks; iMCTrack++) {
      CbmMCTrack* pTrack = (CbmMCTrack*) fMcTracks->At(iMCTrack);
      if ( NULL == pTrack ) continue;
      Int_t Nhits = hitMap[iMCTrack];
      if (Nhits){
         Nring++;
         pTrack->GetMomentum(momentum);
         fh_n_vs_p->Fill(momentum.Mag(),Nhits);
         Int_t gcode = pTrack->GetPdgCode();
         if (TMath::Abs(gcode) == 11) {
            Nel++;
            if (pTrack->GetNPoints(kSTS) > 5) {
               Nel_prim++;
               fh_Nhits->Fill(Nhits);
            }
         } //electrons
         if (TMath::Abs(gcode) == 211) Npi++;
         if (TMath::Abs(gcode) == 321) Nk++;
      } // tracks with points in Rich
   } // track loop

   fh_Nall->Fill(Nring);
   fh_Nel->Fill(Nel);
   fh_Nelprim->Fill(Nel_prim);
   fh_Npi->Fill(Npi);
   fh_Nk->Fill(Nk);
   fh_Nhad->Fill(Nhad);
   fh_Nnoise->Fill(Nnoise);

   if (fVerbose){
   cout << "--------------------------------------------------------------------------" << endl;
   cout << "-----------               Test Rich HitProducer                -----------" << endl;
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

void CbmRichTestHits::Finish()
{
   fMcTracks->Clear();
   fRichPoints->Clear();
   fRichHits->Clear();
}

ClassImp(CbmRichTestHits)

