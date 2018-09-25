/**
 * \file CbmRichUrqmdTest.cxx
 *
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2012
 **/

#include "CbmRichUrqmdTest.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"

#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatch.h"
#include "CbmRichRing.h"
#include "CbmRichHit.h"

#include "std/utils/CbmLitUtils.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>

using namespace std;
using boost::assign::list_of;

CbmRichUrqmdTest::CbmRichUrqmdTest()
  : FairTask("CbmRichUrqmdTest"),
    fOutputDir(""),
    fRichHits(NULL),
    fRichRings(NULL),
    fRichPoints(NULL),
    fMcTracks(NULL),
    fRichRingMatches(NULL),
    fRichProjections(NULL),
    fCanvas(),
    fEventNum(0),
    fMinNofHits(7),
    fNofHitsInRingMap(),
    fh_vertex_z(NULL),
    fh_nof_rings_1hit(NULL),		      
    fh_nof_rings_7hits(NULL),		    
    fh_nof_rings_prim_1hit(NULL),		    
    fh_nof_rings_prim_7hits(NULL),	    
    fh_nof_rings_target_1hit(NULL), 	    
    fh_nof_rings_target_7hits(NULL),	    
    fh_secel_mom(NULL),			    
    fh_gamma_target_mom(NULL),
    fh_gamma_nontarget_mom(NULL),
    fh_pi_mom(NULL),			    
    fh_kaon_mom(NULL),              	    
    fh_mu_mom(NULL),			    
    fh_nof_hits_per_event(NULL),              
    fh_hits_xy_u(NULL),		
    fh_hits_xy_d(NULL),		
    fh_hitrate_xy_u(NULL),	
    fh_hitrate_xy_d(NULL),	
    fh_nof_proj_per_event(NULL),	
    fh_proj_xy_u(NULL),		
    fh_proj_xy_d(NULL),          
    fHists()
{
}

CbmRichUrqmdTest::~CbmRichUrqmdTest()
{

}

InitStatus CbmRichUrqmdTest::Init()
{
   cout << "CbmRichUrqmdTest::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmRichUrqmdTest::Init","RootManager not instantised!"); }

   fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
   if ( NULL == fRichHits) { Fatal("CbmRichUrqmdTest::Init","No RichHit array!"); }

   fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
   if ( NULL == fRichRings) { Fatal("CbmRichUrqmdTest::Init","No RichRing array!"); }

   fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( NULL == fRichPoints) { Fatal("CbmRichUrqmdTest::Init","No RichPoint array!"); }

   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) { Fatal("CbmRichUrqmdTest::Init","No MCTrack array!"); }

   fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
   if ( NULL == fRichRingMatches) { Fatal("CbmRichUrqmdTest::Init","No RichRingMatch array!"); }

   fRichProjections = (TClonesArray*) ioman->GetObject("RichProjection");
   if ( NULL == fRichProjections) { Fatal("CbmRichUrqmdTest::Init","No fRichProjections array!"); }

   InitHistograms();

   return kSUCCESS;
}

void CbmRichUrqmdTest::Exec(
      Option_t* option)
{
   fEventNum++;

   cout << "CbmRichUrqmdTest, event No. " <<  fEventNum << endl;

   FillRichRingNofHits();
   NofRings();
   NofHits();
   NofProjections();
   Vertex();


}

void CbmRichUrqmdTest::InitHistograms()
{
   fh_vertex_z = new TH1D("fh_vertex_z", "fh_vertex_z;z [cm];Number of vertices per event", 200, -1., 200);
   fh_nof_rings_1hit = new TH1D("fh_nof_rings_1hit", "fh_nof_rings_1hit;Number of detected particles/event;Yield", 100, -.5, 99.5);
   fh_nof_rings_7hits = new TH1D("fh_nof_rings_7hits", "fh_nof_rings_7hits;Number of detected particles/event;Yield", 100, -.5, 99.5 );
   fh_nof_rings_prim_1hit = new TH1D("fh_nof_rings_prim_1hit", "fh_nof_rings_prim_1hit;Number of detected particles/event;Yield", 50, -.5, 49.5);
   fh_nof_rings_prim_7hits = new TH1D("fh_nof_rings_prim_7hits", "fh_nof_rings_prim_7hits;Number of detected particles/event;Yield", 50, -.5, 49.5 );
   fh_nof_rings_target_1hit = new TH1D("fh_nof_rings_target_1hit", "fh_nof_rings_target_1hit;Number of detected particles/event;Yield", 60, -.5, 59.5);
   fh_nof_rings_target_7hits = new TH1D("fh_nof_rings_target_7hits", "fh_nof_rings_target_7hits;Number of detected particles/event;Yield", 60, -.5, 59.5 );

   fh_secel_mom = new TH1D("fh_secel_mom", "fh_secel_mom;p [GeV/c];Number per event", 100, 0., 20);
   fh_gamma_target_mom = new TH1D("fh_gamma_target_mom", "fh_gamma_target_mom;p [GeV/c];Number per event", 100, 0., 20);
   fh_gamma_nontarget_mom = new TH1D("fh_gamma_nontarget_mom", "fh_gamma_nontarget_mom;p [GeV/c];Number per event", 100, 0., 20);
   fh_pi_mom = new TH1D("fh_pi_mom", "fh_pi_mom;p [GeV/c];Number per event", 100, 0., 20);
   fh_kaon_mom = new TH1D("fh_pi_mom", "fh_pi_mom;p [GeV/c];Number per event", 100, 0., 20);
   fh_mu_mom = new TH1D("fh_mu_mom", "fh_mu_mom;p [GeV/c];Number per event", 100, 0., 20);

   fh_nof_hits_per_event = new TH1D("fh_nof_hits_per_event", "fh_nof_hits_per_event;Number of hits per event;Yield", 50, 0, 1500);
   fh_hits_xy_u = new TH2D("fh_hits_xy_u", "fh_hits_xy_u;x [cm];y [cm];Number of hits/cm^{2}/event", 110, -110, 110, 45, 90, 180);
   fh_hits_xy_d = new TH2D("fh_hits_xy_d", "fh_hits_xy_d;x [cm];y [cm];Number of hits/cm^{2}/event", 110, -110, 110, 45, -180, -90);

   // bin size is set to 1.2 cm in order to cover 4 pixels, before drawing must be normalized by 1/4
   fh_hitrate_xy_u = new TH2D("fh_hitrate_xy_u", "fh_hitrate_xy_u;x [cm];y [cm];Number of hits/pixel/s", 184, -110, 110, 75, 90, 180);
   fh_hitrate_xy_d = new TH2D("fh_hitrate_xy_d", "fh_hitrate_xy_d;x [cm];y [cm];Number of hits/pixel/s", 184, -110, 110, 75, -180, -90);

   fh_nof_proj_per_event = new TH1D("fh_nof_proj_per_event", "fh_nof_proj_per_event;Number of tracks per event;Yield", 50, 200, 600);
   fh_proj_xy_u = new TH2D("fh_proj_xy_u", "fh_proj_xy_u;x [cm];y [cm];Number of tracks/cm^{2}/event", 220, -110, 110, 90, 90, 180);
   fh_proj_xy_d = new TH2D("fh_proj_xy_d", "fh_proj_xy_d;x [cm];y [cm];Number of tracks/cm^{2}/event", 220, -110, 110, 90, -180, -90);
}

void CbmRichUrqmdTest::FillRichRingNofHits()
{
   fNofHitsInRingMap.clear();
    Int_t nofRichHits = fRichHits->GetEntriesFast();
    for (Int_t iHit=0; iHit < nofRichHits; iHit++) {
        CbmRichHit* hit = static_cast<CbmRichHit*>(fRichHits->At(iHit));
        if (NULL == hit) continue;

        Int_t iPoint = hit->GetRefId();
        if (iPoint < 0) continue;

        FairMCPoint* point = static_cast<FairMCPoint*>(fRichPoints->At(iPoint));
        if (NULL == point) continue;

        Int_t iMCTrack = point->GetTrackID();
        CbmMCTrack* track = static_cast<CbmMCTrack*>(fMcTracks->At(iMCTrack));
        if (NULL == track) continue;

        Int_t iMother = track->GetMotherId();
        if (iMother == -1) continue;

        fNofHitsInRingMap[iMother]++;
    }
}

void CbmRichUrqmdTest::NofRings()
{
   Int_t nofRings = fRichRings->GetEntriesFast();
   int nRings1hit = 0, nRings7hits = 0;
   int nRingsPrim1hit = 0, nRingsPrim7hits = 0;
   int nRingsTarget1hit = 0, nRingsTarget7hits = 0;
   for (Int_t iR = 0; iR < nofRings; iR++){
      CbmRichRing *ring = (CbmRichRing*) fRichRings->At(iR);
      if (NULL == ring) continue;
      CbmTrackMatch* ringMatch = (CbmTrackMatch*) fRichRingMatches->At(iR);
      if (NULL == ringMatch) continue;
      Int_t mcTrackId = ringMatch->GetMCTrackId();
      if (mcTrackId < 0) continue;
      CbmMCTrack* mcTrack = (CbmMCTrack*)fMcTracks->At(mcTrackId);
      if (NULL == mcTrack) continue;
      Int_t motherId = mcTrack->GetMotherId();
      Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
      double mom = mcTrack->GetP();
      TVector3 vert;
      mcTrack->GetStartVertex(vert);
      double dZ = vert.Z();

      if (motherId == -1 && pdg == 11) continue; // do not calculate embedded electrons

      int nofHits = ring->GetNofHits();
      if (nofHits >= 1) nRings1hit++;
      if (nofHits >= fMinNofHits) nRings7hits++;

      if (motherId == -1 && nofHits >= 1) nRingsPrim1hit++;
      if (motherId == -1 && nofHits >= fMinNofHits) nRingsPrim7hits++;

      if (dZ < 0.1 && nofHits >= 1) nRingsTarget1hit++;
      if (dZ < 0.1 && nofHits >= fMinNofHits) nRingsTarget7hits++;

      if (nofHits >= 1) {
         if (motherId != -1) {
            int motherPdg;
            CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
            if (NULL != mother) motherPdg = mother->GetPdgCode();
            if (motherId != -1 && pdg == 11 && motherPdg != 22) fh_secel_mom->Fill(mom);

            if (motherId != -1 && pdg == 11 && motherPdg == 22){
               if (dZ < 0.1){
                  fh_gamma_target_mom->Fill(mom);
               } else {
                  fh_gamma_nontarget_mom->Fill(mom);
               }
            }

         }
         if (pdg == 211) fh_pi_mom->Fill(mom);
         if (pdg == 321) fh_kaon_mom->Fill(mom);
         if (pdg == 13) fh_mu_mom->Fill(mom);
      }
   }
   fh_nof_rings_1hit->Fill(nRings1hit);
   fh_nof_rings_7hits->Fill(nRings7hits);

   fh_nof_rings_prim_1hit->Fill(nRingsPrim1hit);
   fh_nof_rings_prim_7hits->Fill(nRingsPrim7hits);

   fh_nof_rings_target_1hit->Fill(nRingsTarget1hit);
   fh_nof_rings_target_7hits->Fill(nRingsTarget7hits);
}

void CbmRichUrqmdTest::NofHits()
{
   int nofHits = fRichHits->GetEntriesFast();
   int nofNoiseHits = 0;
   int nofHitsUrqmd = 0;
   int nofHitsEl = 0;
   for (int i = 0; i < nofHits; i++) {
      CbmRichHit* hit = (CbmRichHit*) fRichHits->At(i);
      if (NULL == hit) continue;

      Int_t iPoint = hit->GetRefId();
      if (iPoint < 0){
         nofNoiseHits++;
         continue;
      }

      FairMCPoint* point = static_cast<FairMCPoint*>(fRichPoints->At(iPoint));
      if (NULL == point) continue;

      Int_t iMCTrack = point->GetTrackID();
      CbmMCTrack* track = static_cast<CbmMCTrack*>(fMcTracks->At(iMCTrack));
      if (NULL == track) continue;

      Int_t iMother = track->GetMotherId();
      if (iMother == -1) continue;

      CbmMCTrack* track2 = static_cast<CbmMCTrack*>(fMcTracks->At(iMother));
      if (NULL == track2) continue;
      int pdg = TMath::Abs(track2->GetPdgCode());
      int motherId = track2->GetMotherId();
      if (motherId == -1 && pdg == 11){
         nofHitsEl++;
         continue;
      }
      nofHitsUrqmd++;
      double x = hit->GetX();
      double y = hit->GetY();
      if (y > 0) {
        // cout << x << " " <<  y << endl;
         fh_hits_xy_u->Fill(x, y);
         fh_hitrate_xy_u->Fill(x, y);
      } else {
         fh_hits_xy_d->Fill(x, y);
         fh_hitrate_xy_d->Fill(x, y);
      }
   }

   fh_nof_hits_per_event->Fill(nofHitsUrqmd);

   cout << "nofHits = " << nofHits << endl;
   cout << "nofNoiseHits = " << nofNoiseHits << endl;
   cout << "nofHitsUrqmd = " << nofHitsUrqmd << endl;
   cout << "nofHitsEl = " << nofHitsEl << endl;
}

void CbmRichUrqmdTest::NofProjections()
{
   int nofProj = fRichProjections->GetEntriesFast();
   int nofGoodProj = 0;
   for (int i = 0; i < nofProj; i++){
      FairTrackParam* proj = (FairTrackParam*) fRichProjections->At(i);
      if (NULL == proj) continue;
      double x = proj->GetX();
      double y = proj->GetY();
      if (y > 0) {
         fh_proj_xy_u->Fill(x, y);
      } else {
         fh_proj_xy_d->Fill(x, y);
      }
      if (proj->GetX() != 0 && proj->GetY() != 0) nofGoodProj++;
   }

   fh_nof_proj_per_event->Fill(nofGoodProj);

}

void CbmRichUrqmdTest::Vertex()
{
   int nMcTracks = fMcTracks->GetEntries();
   for (int i = 0; i < nMcTracks; i++){
      //At least one hit in RICH
      if (fNofHitsInRingMap[i] < 1) continue;
      CbmMCTrack* mctrack = (CbmMCTrack*) fMcTracks->At(i);
      TVector3 v;
      mctrack->GetStartVertex(v);
      fh_vertex_z->Fill(v.Z());
   } // nMcTracks
}

void CbmRichUrqmdTest::DrawHist()
{
   cout.precision(4);

   SetDefaultDrawStyle();
   fh_vertex_z->Scale(1./fEventNum);
   TCanvas* cVertex = CreateCanvas("rich_urqmd_vertex_z", "rich_urqmd_vertex_z", 800, 800);
   DrawH1(fh_vertex_z);
   gPad->SetLogy(true);

   TCanvas* c2 = CreateCanvas("rich_urqmd_nof_rings", "rich_urqmd_nof_rings", 800, 800);
   fh_nof_rings_1hit->Scale(1./fh_nof_rings_1hit->Integral());
   fh_nof_rings_7hits->Scale(1./fh_nof_rings_7hits->Integral());
   DrawH1(list_of(fh_nof_rings_1hit)(fh_nof_rings_7hits), list_of(string("At least 1 hit detected"))(string("At least 7 hits detected")),
         kLinear, kLinear, true, 0.4, 0.78, 0.99, 0.99);
   cout << "Mean nof rings per event (1 hit) = " << fh_nof_rings_1hit->GetMean() << endl;
   cout << "Mean nof rings per event (7 hits) = " << fh_nof_rings_7hits->GetMean() << endl;

   TCanvas* c3 = CreateCanvas("rich_urqmd_nof_rings_prim", "rich_urqmd_nof_rings_prim", 800, 800);
   fh_nof_rings_prim_1hit->Scale(1./fh_nof_rings_prim_1hit->Integral());
   fh_nof_rings_prim_7hits->Scale(1./fh_nof_rings_prim_7hits->Integral());
   DrawH1(list_of(fh_nof_rings_prim_1hit)(fh_nof_rings_prim_7hits), list_of("At least 1 hit detected")("At least 7 hits detected"),
         kLinear, kLinear, true, 0.4, 0.78, 0.99, 0.99);
   cout << "Mean nof primary rings per event (1 hit) = " << fh_nof_rings_prim_1hit->GetMean() << endl;
   cout << "Mean nof primary rings per event (7 hits) = " << fh_nof_rings_prim_7hits->GetMean() << endl;

   TCanvas* cTarget = CreateCanvas("rich_urqmd_nof_rings_target", "rich_urqmd_nof_rings_target", 800, 800);
   fh_nof_rings_target_1hit->Scale(1./fh_nof_rings_target_1hit->Integral());
   fh_nof_rings_target_7hits->Scale(1./fh_nof_rings_target_7hits->Integral());
   DrawH1(list_of(fh_nof_rings_target_1hit)(fh_nof_rings_target_7hits), list_of("At least 1 hit detected")("At least 7 hits detected"),
         kLinear, kLinear, true, 0.4, 0.78, 0.99, 0.99);
   cout << "Mean nof target rings per event (1 hit) = " << fh_nof_rings_target_1hit->GetMean() << endl;
   cout << "Mean nof target rings per event (7 hits) = " << fh_nof_rings_target_7hits->GetMean() << endl;


   TCanvas* c4 = CreateCanvas("rich_urqmd_sources_mom", "rich_urqmd_sources_mom", 800, 800);
   fh_gamma_target_mom->Scale(1./fEventNum);
   fh_gamma_nontarget_mom->Scale(1./fEventNum);
   fh_secel_mom->Scale(1./fEventNum);
   fh_pi_mom->Scale(1./fEventNum);
   fh_kaon_mom->Scale(1./fEventNum);
   fh_mu_mom->Scale(1./fEventNum);
   DrawH1(list_of(fh_gamma_target_mom)(fh_gamma_nontarget_mom)(fh_secel_mom)(fh_pi_mom)(fh_kaon_mom)(fh_mu_mom),
         list_of("e^{#pm}_{target} from #gamma")("e^{#pm}_{not target} from #gamma")("e^{#pm}_{sec} other")("#pi^{#pm}")("K^{#pm}")("#mu^{#pm}"),
         kLinear, kLog, true, 0.6, 0.7, 0.99, 0.99);

   cout << "Nof sec electrons from gamma target per event = " << fh_gamma_target_mom->GetEntries() / fEventNum << endl;
   cout << "Nof sec electrons from gamma NOT target per event = " << fh_gamma_nontarget_mom->GetEntries() / fEventNum << endl;
   cout << "Nof sec electrons other per event = " << fh_secel_mom->GetEntries() / fEventNum << endl;
   cout << "Nof pions per event = " << fh_pi_mom->GetEntries() / fEventNum << endl;
   cout << "Nof kaons per event = " << fh_kaon_mom->GetEntries() / fEventNum << endl;
   cout << "Nof muons per event = " << fh_mu_mom->GetEntries() / fEventNum << endl;


   TCanvas *c5 = CreateCanvas("rich_urqmd_hits_xy", "rich_urqmd_hits_xy", 800, 800);
   double binArea = fh_hits_xy_u->GetXaxis()->GetBinWidth(1) * fh_hits_xy_u->GetYaxis()->GetBinWidth(1);
   cout << "binArea = " << binArea << endl;
   fh_hits_xy_u->Scale(1./(fEventNum * binArea));
   fh_hits_xy_d->Scale(1./(fEventNum * binArea));

   c5->Divide(1, 2);
   c5->cd(1);
   DrawH2(fh_hits_xy_u);
   fh_hits_xy_u->GetYaxis()->SetTitleOffset(0.75);
   fh_hits_xy_u->GetZaxis()->SetTitleOffset(0.75);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.15);
   c5->cd(2);
   DrawH2(fh_hits_xy_d);
   fh_hits_xy_d->GetYaxis()->SetTitleOffset(0.75);
   fh_hits_xy_d->GetZaxis()->SetTitleOffset(0.75);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.15);

   TCanvas* c6 = CreateCanvas("rich_urqmd_nof_hits_per_event", "rich_urqmd_nof_hits_per_event", 800, 800);
   fh_nof_hits_per_event->Scale(1./fh_nof_hits_per_event->Integral());
   DrawH1(fh_nof_hits_per_event);
   cout << "Mean number of hits per event = " << fh_nof_hits_per_event->GetMean() << endl;

   TCanvas *cHitRate = CreateCanvas("rich_urqmd_hitrate_xy", "rich_urqmd_hitrate_xy", 800, 800);
   fh_hitrate_xy_u->Scale(1e7/(fEventNum * 4.));
   fh_hitrate_xy_d->Scale(1e7/(fEventNum * 4.));
   cHitRate->Divide(1, 2);
   cHitRate->cd(1);
   DrawH2(fh_hitrate_xy_u);
   fh_hitrate_xy_u->GetYaxis()->SetTitleOffset(0.75);
   fh_hitrate_xy_u->GetZaxis()->SetTitleOffset(0.87);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.15);
   cHitRate->cd(2);
   DrawH2(fh_hitrate_xy_d);
   fh_hitrate_xy_d->GetYaxis()->SetTitleOffset(0.75);
   fh_hitrate_xy_d->GetZaxis()->SetTitleOffset(0.87);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.15);

   TCanvas *c7 = CreateCanvas("rich_urqmd_proj_xy", "rich_urqmd_proj_xy", 800, 800);
   fh_proj_xy_u->Scale(1./fEventNum);
   fh_proj_xy_d->Scale(1./fEventNum);
   c7->Divide(1, 2);
   c7->cd(1);
   DrawH2(fh_proj_xy_u);
   fh_proj_xy_u->GetYaxis()->SetTitleOffset(0.75);
   fh_proj_xy_u->GetZaxis()->SetTitleOffset(0.75);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.15);
   c7->cd(2);
   DrawH2(fh_proj_xy_d);
   fh_proj_xy_d->GetYaxis()->SetTitleOffset(0.75);
   fh_proj_xy_d->GetZaxis()->SetTitleOffset(0.75);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.15);

   TCanvas* c8 = CreateCanvas("rich_urqmd_nof_proj_per_event", "rich_urqmd_nof_proj_per_event", 800, 800);
   fh_nof_proj_per_event->Scale(1./fEventNum);
   DrawH1(fh_nof_proj_per_event);
   cout << "Number of track projections per event = " << fh_nof_proj_per_event->GetMean() << endl;
}

void CbmRichUrqmdTest::Finish()
{
   DrawHist();
   for (Int_t i = 0; i < fHists.size(); i++){
      fHists[i]->Write();
   }
   SaveCanvasToImage();
}


TCanvas* CbmRichUrqmdTest::CreateCanvas(
      const string& name,
      const string& title,
      int width,
      int height)
{
   TCanvas* c = new TCanvas(name.c_str(), title.c_str(), width, height);
   fCanvas.push_back(c);
   return c;
}

void CbmRichUrqmdTest::SaveCanvasToImage()
{
   for (int i = 0; i < fCanvas.size(); i++)
   {
      lit::SaveCanvasAsImage(fCanvas[i], fOutputDir);
   }
}

ClassImp(CbmRichUrqmdTest)

