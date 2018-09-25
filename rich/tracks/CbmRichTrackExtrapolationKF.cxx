/**
* \file CbmRichTrackExtrapolationKF.cxx
*
* \author Claudia Hoehne
* \date 206
**/
#include "CbmRichTrackExtrapolationKF.h"

//@@#include "CbmStsKFTrackFitter.h"

#include "FairTrackParam.h"
//#include "CbmStsTrack.h"
#include "CbmGlobalTrack.h"
#include "FairTrackParam.h"
#include "FairRootManager.h"

#include "TClonesArray.h"
#include "TMatrixFSym.h"

#include <iostream>

#include <PndTrack.h>
#include <FairGeanePro.h>

using std::cout;
using std::endl;

CbmRichTrackExtrapolationKF::CbmRichTrackExtrapolationKF():
  //fStsTracks(0)
   fEicTracks(0)
{
}

CbmRichTrackExtrapolationKF::~CbmRichTrackExtrapolationKF()
{
}

void CbmRichTrackExtrapolationKF::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
   if ( NULL == ioman) { Fatal("CbmRichTrackExtrapolationKF::Init", "RootManager not instantised!");}

   //fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   //if ( NULL == fStsTracks) {Fatal("CbmRichTrackExtrapolationKF::Init", "No StsTrack array!");}
   fEicTracks = (TClonesArray*) ioman->GetObject("EicIdealGenTrack");
   if ( NULL == fEicTracks) {Fatal("CbmRichTrackExtrapolationKF::Init", "No reconstructed Track array!");}
}

void CbmRichTrackExtrapolationKF::DoExtrapolation(
      TClonesArray* globalTracks,
      TClonesArray* extrapolatedTrackParams,
      double z,
      int minNofStsHits)
{
   if ( NULL == extrapolatedTrackParams ) {
      cout << "-E- CbmRichTrackExtrapolationKF::DoExtrapolate: TrackParamArray missing!" << endl;
      return;
   }

#if _OLD_
   if ( NULL == globalTracks ) {
      cout << "-E- CbmRichTrackExtrapolationKF::DoExtrapolate: Track Array missing!" << endl;
      return;
   }
#endif

   TMatrixFSym covMat(5);
   for(Int_t i=0;i<5;i++) for(Int_t j=0; j<=i; j++) covMat(i,j) = 0;
   covMat(0,0) = covMat(1,1) = covMat(2,2) = covMat(3,3) = covMat(4,4) = 1.e-4; 

   TVector3 pos, mom;

#if _ORIG_
   Int_t nTracks = globalTracks->GetEntriesFast();
   for (Int_t iTrack=0; iTrack < nTracks; iTrack++){
      CbmGlobalTrack* gTrack = (CbmGlobalTrack*)globalTracks->At(iTrack);
      new((*extrapolatedTrackParams)[iTrack]) FairTrackParam(0.,0.,0.,0.,0.,0.,covMat);
      Int_t idSTS = gTrack->GetStsTrackIndex();
      if (idSTS < 0 ) continue;
      CbmStsTrack* pSTStr = (CbmStsTrack*) fStsTracks->At(idSTS);
      if ( NULL == pSTStr ) continue;

      CbmStsKFTrackFitter KF;
      FairTrackParam ExTrack;

      KF.Extrapolate(pSTStr, z, &ExTrack);

      Int_t Nsts = pSTStr->GetNStsHits();

      if ( Nsts >= minNofStsHits) {
         *(FairTrackParam*)(extrapolatedTrackParams->At(iTrack)) = ExTrack;
      }
   }
#else
   Int_t nTracks = fEicTracks->GetEntriesFast();
   for (Int_t iTrack=0; iTrack < nTracks; iTrack++) {
     PndTrack *track = (PndTrack*) fEicTracks->At(iTrack);
     //CbmGlobalTrack* gTrack = (CbmGlobalTrack*)globalTracks->At(iTrack);


     //Int_t idSTS = gTrack->GetStsTrackIndex();
     //if (idSTS < 0 ) continue;
     //CbmStsTrack* pSTStr = (CbmStsTrack*) fStsTracks->At(idSTS);
     //if ( NULL == pSTStr ) continue;

     //CbmStsKFTrackFitter KF;
     FairGeanePro *fPro0 = new FairGeanePro();
     FairTrackParP *fRes= new FairTrackParP();
     FairTrackParam ExTrack;
     FairTrackParP par = track->GetParamLast();
     TVector3 x0(0,0,z), u0(1,0,0), v0(0,1,0);
     fPro0->PropagateToPlane(x0, u0, v0);
     //KF.Extrapolate(track, z, &ExTrack);
     Bool_t rc =  fPro0->Propagate(&par, fRes, -211); // FIXME: PID correctly, please!;	

     new((*extrapolatedTrackParams)[iTrack]) 
       FairTrackParam(fRes->GetX(), fRes->GetY(), fRes->GetZ(),
		      fRes->GetPx()/fRes->GetPz(), fRes->GetPy()/fRes->GetPz(),
		      fRes->GetQp(), covMat);
     //printf("%f %f %f, %f %f %f\n", fRes->GetX(), fRes->GetY(), fRes->GetZ(),
     //		      fRes->GetPx()/fRes->GetPz(), fRes->GetPy()/fRes->GetPz(),
     //	    fRes->GetQp()); 

#if 0
     //Int_t Nsts = pSTStr->GetNStsHits();
     //if ( Nsts >= minNofStsHits) {
     //*(FairTrackParam*)(extrapolatedTrackParams->At(iTrack)) = ExTrack;
     *(FairTrackParP*)(extrapolatedTrackParams->At(iTrack)) = *fRes;
     // }
#endif
   } //for iTrack
#endif
}
