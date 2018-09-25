/**
* \file CbmRichMatchRings.cxx
*
* \author Supriya Das
* \date 2006
**/

#include <assert.h>

#include "CbmRichMatchRings.h"

#include "CbmRichHit.h"
#include "CbmRichRing.h"
#include "CbmTrackMatch.h"

#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

#include <iostream>
#include <map>

using std::cout;
using std::endl;
using std::map;

CbmRichMatchRings::CbmRichMatchRings():
   FairTask("CbmRichMatchRings"),
   fRings(NULL),
   fPoints(NULL),
   fTracks(NULL),
   fHits(NULL),
   fMatches(NULL),

   fMatchMap()//,
//   fMatchMCMap()
{

}

CbmRichMatchRings::~CbmRichMatchRings()
{
}


InitStatus CbmRichMatchRings::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) {Fatal("CbmRichMatchRings::Init", "RootManager not instantiated!");}

   fHits = (TClonesArray*) ioman->GetObject("RichHit");
   if (NULL == fHits) {Fatal("CbmRichMatchRings::Init", "No RichHit array!");}

   fRings = (TClonesArray*) ioman->GetObject("RichRing");
   if (NULL == fRings) { Fatal("CbmRichMatchRings::Init", "No RichRing array!");}

   fPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if (NULL == fPoints) {Fatal("CbmRichMatchRings::Init", "No RichPoint array!");}

   fTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if (NULL == fTracks ) {Fatal("CbmRichMatchRings::Init", "No MCTrack array!");}

   // Create and register RichRingMatch array
   fMatches = new TClonesArray("CbmTrackMatch",100);
   ioman->Register("RichRingMatch", "RICH", fMatches, kTRUE);

   return kSUCCESS;
}

void CbmRichMatchRings::Exec(
      Option_t* opt)
{
   // Clear output array
   if (fMatches != NULL) fMatches->Clear();
   map<Int_t, Int_t>::iterator it ;
//   fMatchMCMap.clear();

//   // Loop over Rich hits
//   Int_t nRichHits = fHits->GetEntriesFast();
//   for (Int_t iHit=0; iHit < nRichHits; iHit++) {
//      CbmRichHit* hit = (CbmRichHit*) fHits->At(iHit);
//      if ( NULL == hit ) continue;
//
//      Int_t iPoint = hit->GetRefId();
//      if ( iPoint < 0 ) { // Fake or background hit
//         continue;
//      }
//      //Get the MC Point corresponding to the hit
//      FairMCPoint* point = (FairMCPoint*) fPoints->At(iPoint);
//      if ( NULL == point ) continue;
//      //Get the MC Track ID corresponding to the MC Point
//      Int_t iMCTrack = point->GetTrackID();
//      // Get the MC Track corresponding to the ID
//      CbmMCTrack* track = (CbmMCTrack*)fTracks->At(iMCTrack);
//      if (NULL == track) continue;
//      Int_t iMother = track->GetMotherId();
//      fMatchMCMap[iMother]++;
//   }

   // Loop over RichRings
   Int_t nRings = fRings->GetEntriesFast();
   for (Int_t iRing=0; iRing<nRings; iRing++) {
      CbmRichRing* ring = (CbmRichRing*) fRings->At(iRing);
      if (NULL == ring) continue;

      Int_t nHits = ring->GetNofHits();
      Int_t nAll = 0;
      Int_t nTrue = 0; //number of true hits in ring
      Int_t nFake = 0; // number of fake hits in ring
      Int_t nWrong = 0; // number of wrong hits in ring
      Int_t nMCTracks = 0; //number of MC tracks from which hits ring was formed.

      fMatchMap.clear();

      // Loop over Hits of ring
      for (Int_t iHit=0; iHit<nHits; iHit++) {
	assert(0);
#if _NOW_
         CbmRichHit* hit = (CbmRichHit*) fHits->At(ring->GetHit(iHit));
         if ( NULL == hit ) continue;
         Int_t iPoint = hit->GetRefId();
         if ( iPoint < 0 ) { // Fake or background hit
            nFake++;
            continue;
         }

         //Get the MC Point corresponding to the hit
         FairMCPoint* point = (FairMCPoint*) fPoints->At(iPoint);
         if (NULL == point ) continue;
         //Get the MC Track ID corresponding to the MC Point
         Int_t iMCTrack = point->GetTrackID();

         // Get the MC Track corresponding to the ID
         CbmMCTrack* track   = (CbmMCTrack*)fTracks->At(iMCTrack);
         Int_t iMother = track->GetMotherId();
         fMatchMap[iMother]++;
#endif
      }// Hit loop

      assert(0);
#if _NOW_
      // Search for best matching MCTrack
      Int_t iMCTrack = -1;
      for (it=fMatchMap.begin(); it!=fMatchMap.end(); it++) {
         nMCTracks++;
         nAll += it->second;
         if ( it->second > nTrue ) {
            iMCTrack = it->first;
            nTrue = it->second;
         }
      }

//      Int_t nMCHits = fMatchMCMap[iMCTrack]; //number of hits in MC ring
      nWrong = nAll - nTrue;

      // Create RichRingMatch
      new ((*fMatches)[iRing]) CbmTrackMatch(iMCTrack, nTrue, nWrong, nFake, nMCTracks);
#endif
   }// Ring loop
}

void CbmRichMatchRings::Finish()
{

}

ClassImp(CbmRichMatchRings)
