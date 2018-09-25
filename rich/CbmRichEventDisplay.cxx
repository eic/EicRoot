/**
* \file CbmRichEventDisplay.cxx
*
* \author Supriya Das
* \date 2006
**/

#include "CbmRichEventDisplay.h"

#include "CbmRichHit.h"
#include "CbmRichRing.h"
#include "CbmTrackMatch.h"
#include "CbmRichPoint.h"
#include "CbmDrawHist.h"
#include "FairTrackParam.h"

#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "FairRootManager.h"

#include "CbmDrawHist.h"

#include "TClonesArray.h"
#include "TEllipse.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TMarker.h"

#include <iostream>
#include <map>
#include <sstream>

using namespace std;

CbmRichEventDisplay::CbmRichEventDisplay():
   FairTask("CbmRichEventDisplay"),
   fRichRings(NULL),
   fRichHits(NULL),
   fRichPoints(NULL),
   fRichMatches(NULL),
   fMcTracks(NULL),
   fRichProjections(NULL),
   fEventNum(0),
   fDrawRings(true),
   fDrawHits(true),
   fDrawPoints(true),
   fDrawProjections(true)
{
   SetDefaultDrawStyle();
}

CbmRichEventDisplay::~CbmRichEventDisplay()
{
}


InitStatus CbmRichEventDisplay::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) {Fatal("CbmRichEventDisplay::Init", "RootManager not instantiated!");}

   fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
   if (NULL == fRichHits) {Fatal("CbmRichEventDisplay::Init", "No RichHit array!");}

   fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
   if (NULL == fRichRings) { Fatal("CbmRichEventDisplay::Init", "No RichRing array!");}

   fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if (NULL == fRichPoints) {Fatal("CbmRichEventDisplay::Init", "No RichPoint array!");}

   fRichMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
   if (NULL == fRichMatches) {Fatal("CbmRichEventDisplay::Init", "No RichRingMatch array!");}

   fRichProjections = (TClonesArray*) ioman->GetObject("RichProjection");
   if (NULL == fRichProjections) {Fatal("CbmRichEventDisplay::Init", "No RichProjection array!");}

   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if (NULL == fMcTracks ) {Fatal("CbmRichEventDisplay::Init", "No MCTrack array!");}

   return kSUCCESS;
}

void CbmRichEventDisplay::Exec(
      Option_t* opt)
{
   fEventNum++;
   SetDefaultDrawStyle();
   DrawOneEvent();
}

void CbmRichEventDisplay::DrawOneEvent()
{
   stringstream ss;
   ss << "rich_event_display_event_"<< fEventNum;
   TCanvas *c = new TCanvas(ss.str().c_str(), ss.str().c_str(), 800, 800);
   c->Divide(1, 2);
   c->cd(1);
   TH2D* padU = new TH2D("padU", ";x [cm];y [cm]", 1, -110., 110., 1, 90., 180);
   DrawH2(padU);
   padU->GetYaxis()->SetTitleOffset(0.75);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.05);
   DrawOnePmtPlane("up");

   c->cd(2);
   TH2D* padD = new TH2D("padD", ";x [cm];y [cm]", 1, -110., 110., 1, -180., -90.);
   DrawH2(padD);
   padD->GetYaxis()->SetTitleOffset(0.75);
   gPad->SetLeftMargin(0.1);
   gPad->SetRightMargin(0.05);
   DrawOnePmtPlane("down");
}

void CbmRichEventDisplay::DrawOnePmtPlane(
      const string& plane)
{
   //Draw Track projections
   if (fDrawProjections) {
      int nofProjections = fRichProjections->GetEntriesFast();
      for (int iP = 0; iP < nofProjections; iP++){
         FairTrackParam* pr = (FairTrackParam*) fRichProjections->At(iP);
         if (NULL == pr) continue;
         if ( (plane == "up" && pr->GetY() >= 0.) ||
              (plane == "down" && pr->GetY() < 0.) ){
            TMarker* m = new TMarker(pr->GetX(), pr->GetY(), 3.);
            m->SetMarkerSize(0.7);
            m->SetMarkerColor(kGreen+3);
            m->Draw();
         }
      }
   }

   // Draw hits
   if (fDrawHits){
      int nofHits = fRichHits->GetEntriesFast();
      for (int iH = 0; iH < nofHits; iH++){
         CbmRichHit* hit = (CbmRichHit*) fRichHits->At(iH);
         if (NULL == hit) continue;
         if ( (plane == "up" && hit->GetY() >= 0.) ||
              (plane == "down" && hit->GetY() < 0.) ){

            TEllipse* hitDr = new TEllipse(hit->GetX(), hit->GetY(), 0.6);
            hitDr->SetFillColor(kRed);
            hitDr->SetLineColor(kRed);
            hitDr->Draw();
         }
      }
   }

   // Draw rings
   if (fDrawRings){
      int nofRings = fRichRings->GetEntriesFast();
      for (int iR = 0; iR < nofRings; iR++){
         CbmRichRing* ring = (CbmRichRing*) fRichRings->At(iR);
         if (NULL == ring) continue;
         if ( (plane == "up" && ring->GetCenterY() >= 0.) ||
              (plane == "down" && ring->GetCenterY() < 0.) ){
            DrawCircle(ring);
         }
      }
   }

   // Draw RICH MC Points
   if (fDrawPoints) {
      int nofPoints = fRichPoints->GetEntriesFast();
      for (int iP = 0; iP < nofPoints; iP++){
         CbmRichPoint* point = (CbmRichPoint*) fRichPoints->At(iP);
         if (NULL == point) continue;
         if ( (plane == "up" && point->GetY() >= 0.) ||
              (plane == "down" && point->GetY() < 0.) ){
            TEllipse* pointDr = new TEllipse(point->GetX(), point->GetY(), 0.4);
            pointDr->Draw();
         }
      }
   }


}

void CbmRichEventDisplay::DrawCircle(
      CbmRichRing* ring)
{
   TEllipse* circle = new TEllipse(ring->GetCenterX(), ring->GetCenterY(), ring->GetRadius());
   circle->SetFillStyle(0);
   circle->SetLineWidth(2);
   circle->SetLineColor(kBlue);
   circle->Draw();
   TMarker* center = new TMarker(ring->GetCenterX(), ring->GetCenterY(), 2);
   center->SetMarkerColor(kBlue);
   center->SetMarkerSize(0.4);
   center->Draw();
}

void CbmRichEventDisplay::Finish()
{

}

ClassImp(CbmRichEventDisplay)
