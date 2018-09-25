/**
* \file CbmRichRingTrackAssignIdeal.cxx
*
* \author Claudia Hoehne and Semen Lebedev
* \date 2007
**/

#include "CbmRichRingTrackAssignIdeal.h"

#include "TClonesArray.h"

#include "CbmRichRing.h"

#include "CbmMCTrack.h"
#include "FairRootManager.h"
#include "FairTrackParam.h"
#include "CbmGlobalTrack.h"
#include "CbmTrackMatch.h"
#include "CbmTrackMatch.h"

#include <iostream>

using std::cout;
using std::endl;

CbmRichRingTrackAssignIdeal::CbmRichRingTrackAssignIdeal():
   fMcTracks(NULL),
   fGlobalTracks(NULL),
   fRingMatches(NULL),
   fStsTrackMatches(NULL)
{

}

CbmRichRingTrackAssignIdeal::~CbmRichRingTrackAssignIdeal()
{
}

void CbmRichRingTrackAssignIdeal::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmRichRingTrackAssignIdeal::Init", "RootManager not instantised!");}

   fMcTracks  = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) {Fatal("CbmRichRingTrackAssignIdeal::Init", "No MCTrack array!");}

   fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
   if ( NULL == fGlobalTracks) {Fatal("CbmRichRingTrackAssignIdeal::Init", "No GlobalTrack array!");}

   fRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
   if ( NULL == fRingMatches) {Fatal("CbmRichRingTrackAssignIdeal::Init", "No RichRingMatch array!");}

   fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
   if ( NULL == fStsTrackMatches) {Fatal("CbmRichRingTrackAssignIdeal::Init", "No StsTrackMatch array!");}
}

void CbmRichRingTrackAssignIdeal::DoAssign(
      TClonesArray *rings,
      TClonesArray* richProj)
{
   Int_t nofTracks = richProj->GetEntriesFast();
   Int_t nofRings = rings->GetEntriesFast();

   for (Int_t iRing=0; iRing < nofRings; iRing++){
      CbmRichRing* pRing = (CbmRichRing*) rings->At(iRing);
      if (NULL == pRing) continue;
      if (pRing->GetNofHits() < fMinNofHitsInRing) continue;

      CbmTrackMatch* pRingMatch = (CbmTrackMatch*) fRingMatches->At(iRing);
      if (NULL == pRingMatch) continue;
      Int_t ringID = pRingMatch->GetMCTrackId();
      Double_t xRing = pRing->GetCenterX();
      Double_t yRing = pRing->GetCenterY();

      for (Int_t iTrack=0; iTrack < nofTracks; iTrack++){
         FairTrackParam* pTrack = (FairTrackParam*) richProj->At(iTrack);
         if (NULL == pTrack) continue;
         Double_t xTrack = pTrack->GetX();
         Double_t yTrack = pTrack->GetY();

         // no projection to photodetector plane
         if (xTrack == 0 && yTrack == 0) continue;

         CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iTrack);
         if (NULL == gTrack) continue;
         if (gTrack->GetStsTrackIndex() == -1) continue;
         CbmTrackMatch* pTrackMatch = (CbmTrackMatch*) fStsTrackMatches->At(gTrack->GetStsTrackIndex());
         if (NULL == pTrackMatch) continue;

         if (pTrackMatch->GetMCTrackId() == ringID){
            gTrack -> SetRichRingIndex(iRing);
            pRing -> SetTrackID(iTrack);
            Double_t dist = TMath::Sqrt( (xRing-xTrack)*(xRing-xTrack)+(yRing-yTrack)*(yRing-yTrack) );
            pRing->SetDistance(dist);
         } // ideal assignement
      } // loop tracks
   } // loop rings
}
