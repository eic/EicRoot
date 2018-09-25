/**
* \file CbmRichTrackExtrapolationIdeal.cxx
*
* \author Claudia Hoehne
* \date 2006
**/

#include <assert.h>

#include "CbmRichTrackExtrapolationIdeal.h"

#include "CbmRichPoint.h"
#include "FairTrackParam.h"
#include "CbmMCTrack.h"
//#include "CbmStsTrack.h"
#include "CbmTrackMatch.h"
#include "CbmGlobalTrack.h"
#include "FairRootManager.h"

#include "TClonesArray.h"
#include "TMatrixFSym.h"

#include <iostream>

using std::cout;
using std::endl;

CbmRichTrackExtrapolationIdeal::CbmRichTrackExtrapolationIdeal():
   fRefPlanePoints(NULL),
   fMcTracks(NULL)/*,
   fStsTracks(NULL),
   fStsTrackMatches(NULL)*/
{

}

CbmRichTrackExtrapolationIdeal::~CbmRichTrackExtrapolationIdeal()
{
}

void CbmRichTrackExtrapolationIdeal::Init()
{
  FairRootManager* ioman = FairRootManager::Instance();
  if (NULL == ioman) { Fatal("CbmRichTrackExtrapolationIdeal::Init", "RootManager not instantised!");}

  fMcTracks  = (TClonesArray*) ioman->GetObject("MCTrack");
  if ( NULL == fMcTracks) { Fatal("CbmRichTrackExtrapolationIdeal::Init", "No MCTrack array!");}

  fRefPlanePoints  = (TClonesArray*) ioman->GetObject("RefPlanePoint");
  if ( NULL == fRefPlanePoints) { Fatal("CbmRichTrackExtrapolationIdeal::Init", "No RefPlanePoint array!");}

  assert(0);
#if _OFF_
  fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
  if ( NULL == fStsTracks) { Fatal("CbmRichTrackExtrapolationIdeal::Init", "No StsTrack array!");}

  fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
  if ( NULL == fStsTrackMatches) { Fatal("CbmRichTrackExtrapolationIdeal::Init", "No StsTrackMatch array!");}
#endif
}

void CbmRichTrackExtrapolationIdeal::DoExtrapolation(
      TClonesArray* globalTracks,
      TClonesArray* extrapolatedTrackParams,
      double z,
      int minNofStsHits)
{
   if ( NULL == extrapolatedTrackParams ) {
      cout << "-E- CbmRichTrackExtrapolationIdeal::DoExtrapolate: TrackParam Array missing!" << endl;
      return;
   }

   if ( NULL == globalTracks ) {
      cout << "-E- CbmRichTrackExtrapolationIdeal::DoExtrapolate: Global Track Array missing! " << endl;
      return;
   }

   // some default variables
   Double_t tx,ty,qp;
   Double_t charge = 1.;
   TMatrixFSym covMat(5);
   for(Int_t i=0;i<5;i++) for(Int_t j=0; j<=i; j++) covMat(i,j) = 0;
   covMat(0,0) = covMat(1,1) = covMat(2,2) = covMat(3,3) = covMat(4,4) = 1.e-4;

   TVector3 pos, mom;
   Int_t nTracks = globalTracks->GetEntriesFast();
   for (Int_t iTrack=0; iTrack < nTracks; iTrack++){
      CbmGlobalTrack* gTrack = (CbmGlobalTrack*) globalTracks->At(iTrack);
      new((*extrapolatedTrackParams)[iTrack]) FairTrackParam(0.,0.,0.,0.,0.,0.,covMat);
      Int_t idSTS = gTrack->GetStsTrackIndex();
      if (idSTS < 0 ) continue;

      assert(0);
#if _OFF_
      CbmStsTrack* pSTStr = (CbmStsTrack*) fStsTracks->At(idSTS);
      if ( NULL == pSTStr ) continue;

      Int_t Nsts = pSTStr->GetNStsHits() + pSTStr->GetNMvdHits();
      if ( Nsts >= minNofStsHits) {
         CbmTrackMatch* pTrackMatch = (CbmTrackMatch*)fStsTrackMatches->At(idSTS);
         if ( NULL == pTrackMatch) continue;
         Int_t iMCmatch = pTrackMatch->GetMCTrackId();
         for (Int_t ii=0; ii < fRefPlanePoints->GetEntriesFast(); ii++){
            CbmRichPoint* pRefPlane = (CbmRichPoint*)fRefPlanePoints->At(ii);
            if (pRefPlane->GetTrackID() == iMCmatch){
               pRefPlane->Momentum(mom);
               pRefPlane->Position(pos);
               tx = mom.Px()/mom.Pz();
               ty = mom.Py()/mom.Pz();
               qp = charge/mom.Mag();
               FairTrackParam richtrack(pos.X(),pos.Y(),pos.Z(),tx,ty,qp,covMat);
               *(FairTrackParam*)(extrapolatedTrackParams->At(iTrack)) = richtrack;
            }  // select MCPoint to that track
         }  // loop MCPoints
     }  // require fMinNsts
#endif
   } // loop tracks
}
