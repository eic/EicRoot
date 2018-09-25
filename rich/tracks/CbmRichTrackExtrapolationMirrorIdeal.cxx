/**
* \file CbmRichTrackExtrapolationMirrorIdeal.cxx
*
* \author Claudia Hoehne
* \date 2006
**/

#include <assert.h>

#include "CbmRichTrackExtrapolationMirrorIdeal.h"

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

CbmRichTrackExtrapolationMirrorIdeal::CbmRichTrackExtrapolationMirrorIdeal():
   fRichMirrorPoints(NULL),
   fMcTracks(NULL)/*,
   fSTSArray(NULL),
   fTrackMatchArray(NULL)*/
{

}

CbmRichTrackExtrapolationMirrorIdeal::~CbmRichTrackExtrapolationMirrorIdeal()
{

}

void CbmRichTrackExtrapolationMirrorIdeal::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) {Fatal("CbmRichTrackExtrapolationMirrorIdeal::Init", "RootManager not instantised!");}

   fMcTracks  = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) {Fatal("CbmRichTrackExtrapolationMirrorIdeal::Init", "No MCTrack array!");}

   fRichMirrorPoints  = (TClonesArray*) ioman->GetObject("RichMirrorPoint");
   if ( NULL == fRichMirrorPoints) {Fatal("CbmRichTrackExtrapolationMirrorIdeal::Init", "No RichMirrorPoint array!");}

   assert(0);
#if _OFF_
   fSTSArray = (TClonesArray*) ioman->GetObject("StsTrack");
   if ( NULL == fSTSArray) {Fatal("CbmRichTrackExtrapolationMirrorIdeal::Init", "No StsTrack array!");}

   fTrackMatchArray = (TClonesArray*) ioman->GetObject("StsTrackMatch");
   if ( NULL == fTrackMatchArray) {Fatal("CbmRichTrackExtrapolationMirrorIdeal::Init", "No StsTrackMatch array!");}
#endif
}

void CbmRichTrackExtrapolationMirrorIdeal::DoExtrapolation(
      TClonesArray* globalTracks,
      TClonesArray* extrapolatedTrackParams,
      double z,
      int minNofStsHits)
{
   if ( NULL == extrapolatedTrackParams ) {
      cout << "-E- CbmRichTrackExtrapolationMirrorIdeal::DoExtrapolate: TrackParam Array missing!" << endl;
      return;
   }

   if ( NULL == globalTracks ) {
      cout << "-E- CbmRichTrackExtrapolationMirrorIdeal::DoExtrapolate: Global Track Array missing!" << endl;
      return;
   }

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
      CbmStsTrack* pSTStr = (CbmStsTrack*) fSTSArray->At(idSTS);
      if ( NULL == pSTStr ) continue;
      Int_t Nsts = pSTStr->GetNStsHits() + pSTStr->GetNMvdHits();
      if ( Nsts >= minNofStsHits) {
         CbmTrackMatch* pTrackMatch = (CbmTrackMatch*)fTrackMatchArray->At(idSTS);
         if (NULL == pTrackMatch) continue;
         Int_t iMCmatch = pTrackMatch->GetMCTrackId();
         for (Int_t ii=0; ii < fRichMirrorPoints->GetEntriesFast(); ii++){
            CbmRichPoint* pMirror = (CbmRichPoint*) fRichMirrorPoints->At(ii);
            if (pMirror->GetTrackID() == iMCmatch){
               pMirror->Momentum(mom);
               pMirror->Position(pos);
               tx = mom.Px()/mom.Pz();
               ty = mom.Py()/mom.Pz();
               qp = charge/mom.Mag();
               FairTrackParam richtrack(pos.X(),pos.Y(),pos.Z(),tx,ty,qp,covMat);
               *(FairTrackParam*)(extrapolatedTrackParams->At(iTrack)) = richtrack;
            }
         }
      }
#endif
   }
}
