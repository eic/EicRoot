//////////////////////////////////////////////////////////////////////////
//                                                                      //
// PndPidCandidate	                                                //
//                                                                      //
// Definition of the Panda pid candidate.	                        //
//                                                                      //
// Author: Klaus Goetzen, GSI, 12.06.08		                        //
// Copyright (C) 2008, GSI Darmstadt.		         		//
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "PndPidCandidate.h"

//class VAbsPidInfo;

//  ========================================================================
//  ===== PndPidCandidate - Class definig the AOD interface           ====
//  ========================================================================

PndPidCandidate::PndPidCandidate(): 
  fLocked(kFALSE),
  fCharge(0),
  fXposition(0.),		
  fYposition(0.),		
  fZposition(0.),		
  fXmomentum(0.),		
  fYmomentum(0.),		
  fZmomentum(0.),		
  fEnergy(0.),		
  fFirstHitX(0.),		
  fFirstHitY(0.),		
  fFirstHitZ(0.),		
  fLastHitX(0.),		
  fLastHitY(0.),		
  fLastHitZ(0.),		
  fMcIndex(-1),
  fTrackIndex(-1), 
  fTrackBranch(-1),
#if _OLD_
  fMvdDEDX(0.),
  fMvdHits(0),
  fSttMeanDEDX(0.),
  fSttHits(0),
  fGemHits(0), 
  fTofStopTime(0.),
  fTofM2(0.),
  fTofTrackLength(0.),
  fTofQuality(-1.),
  fTofIndex(-1),
  fDrcThetaC(0.),
  fDrcThetaCErr(0.),
  fDrcQuality(-1.),
  fDrcNumberOfPhotons(0),
  fDrcIndex(-1),
  fDiscThetaC(0.),
  fDiscThetaCErr(0.),
  fDiscQuality(-1.),
  fDiscNumberOfPhotons(0),
  fDiscIndex(-1),
  fRichThetaC(0.),
  fRichThetaCErr(0.),
  fRichQuality(-1.),
  fRichNumberOfPhotons(0),
  fRichIndex(-1),
  fEmcRawEnergy(0.),
  fEmcCalEnergy(0.),
  fEmcQuality(-1.),
  fEmcNumberOfCrystals(0),
  fEmcNumberOfBumps(0),
  fEmcModule(-1), 
  fEmcIndex(-1),
  fEmcZ20(0.),
  fEmcZ53(0.),
  fEmcLat(0.),
  fEmcE1 (0.),
  fEmcE9 (0.),
  fEmcE25(0.),
  fMuoNumberOfLayers(0),
  fMuoProbability(0.),
  fMuoQuality(-1.), 
  fMuoIron(0.),
  fMuoMomentumIn(0.),
  fMuoModule(0), 
  fMuoHits(0),
  fMuoIndex(-1),
#endif
  fDegreesOfFreedom(0),
  fFitStatus(-1),
  fChiSquared(0.)		
{	
  SetDefault();
}

PndPidCandidate::PndPidCandidate(Int_t charge, TVector3 &pos, TLorentzVector &p4, TMatrixD &cov7 ) :
  fLocked(kFALSE),
  fCharge(0),
  fXposition(0.),		
  fYposition(0.),		
  fZposition(0.),		
  fXmomentum(0.),		
  fYmomentum(0.),		
  fZmomentum(0.),		
  fEnergy(0.),		
  fFirstHitX(0.),		
  fFirstHitY(0.),		
  fFirstHitZ(0.),		
  fLastHitX(0.),		
  fLastHitY(0.),		
  fLastHitZ(0.),		
  fMcIndex(-1),
  fTrackIndex(-1),
  fTrackBranch(-1),
#if _OLD_
  fMvdDEDX(0.),
  fMvdHits(0),
  fSttMeanDEDX(0.),
  fSttHits(0),
  fGemHits(0), 
  fTofStopTime(0.),
  fTofM2(0.),
  fTofTrackLength(0.),
  fTofQuality(-1.),
  fTofIndex(-1),
  fDrcThetaC(0.),
  fDrcThetaCErr(0.),
  fDrcQuality(-1.),
  fDrcNumberOfPhotons(0),
  fDrcIndex(-1),
  fDiscThetaC(0.),
  fDiscThetaCErr(0.),
  fDiscQuality(-1.),
  fDiscNumberOfPhotons(0),
  fDiscIndex(-1),
  fRichThetaC(0.),
  fRichThetaCErr(0.),
  fRichQuality(-1.),
  fRichNumberOfPhotons(0),
  fRichIndex(-1),
  fEmcRawEnergy(0.),
  fEmcCalEnergy(0.),
  fEmcQuality(-1.),
  fEmcNumberOfCrystals(0),
  fEmcNumberOfBumps(0),
  fEmcModule(-1), 
  fEmcIndex(-1),
  fEmcZ20(0.),
  fEmcZ53(0.),
  fEmcLat(0.),
  fEmcE1 (0.),
  fEmcE9 (0.),
  fEmcE25(0.),
  fMuoNumberOfLayers(0),
  fMuoProbability(0.),
  fMuoQuality(-1.), 
  fMuoIron(0.),
  fMuoMomentumIn(0.),
  fMuoModule(0), 
  fMuoHits(0),
  fMuoIndex(-1),
#endif
  fDegreesOfFreedom(0),
  fFitStatus(-1),
  fChiSquared(0.)
{
  SetDefault();
  SetPosition(pos);
  SetLorentzVector(p4);
  SetCov7(cov7);
  SetCharge(charge);
}

PndPidCandidate::PndPidCandidate(Int_t charge, TVector3 &pos, TLorentzVector &p4) :
  fLocked(kFALSE),
  fCharge(0),
  fXposition(0.),		
  fYposition(0.),		
  fZposition(0.),		
  fXmomentum(0.),		
  fYmomentum(0.),		
  fZmomentum(0.),		
  fEnergy(0.),		
  fFirstHitX(0.),		
  fFirstHitY(0.),		
  fFirstHitZ(0.),		
  fLastHitX(0.),		
  fLastHitY(0.),		
  fLastHitZ(0.),		
  fMcIndex(-1),
  fTrackIndex(-1),
  fTrackBranch(-1),
#if _OLD_
  fMvdDEDX(0.),
  fMvdHits(0),
  fSttMeanDEDX(0.),
  fSttHits(0),
  fGemHits(0), 
  fTofStopTime(0.),
  fTofM2(0.),
  fTofTrackLength(0.),
  fTofQuality(-1.),
  fTofIndex(-1),
  fDrcThetaC(0.),
  fDrcThetaCErr(0.),
  fDrcQuality(-1.),
  fDrcNumberOfPhotons(0),
  fDrcIndex(-1),
  fDiscThetaC(0.),
  fDiscThetaCErr(0.),
  fDiscQuality(-1.),
  fDiscNumberOfPhotons(0),
  fDiscIndex(-1),
  fRichThetaC(0.),
  fRichThetaCErr(0.),
  fRichQuality(-1.),
  fRichNumberOfPhotons(0),
  fRichIndex(-1),
  fEmcRawEnergy(0.),
  fEmcCalEnergy(0.),
  fEmcQuality(-1.),
  fEmcNumberOfCrystals(0),
  fEmcNumberOfBumps(0),
  fEmcModule(-1), 
  fEmcIndex(-1),
  fEmcZ20(0.),
  fEmcZ53(0.),
  fEmcLat(0.),
  fEmcE1 (0.),
  fEmcE9 (0.),
  fEmcE25(0.),
  fMuoNumberOfLayers(0),
  fMuoProbability(0.),
  fMuoQuality(-1.), 
  fMuoIron(0.),
  fMuoMomentumIn(0.),
  fMuoModule(0),
  fMuoHits(0),
  fMuoIndex(-1),
#endif
  fDegreesOfFreedom(0),
  fFitStatus(-1),
  fChiSquared(0.)
{
  SetDefault();
  SetPosition(pos);
  SetLorentzVector(p4);
  SetCharge(charge);
}

	
PndPidCandidate::~PndPidCandidate()
{
}

TMatrixD& PndPidCandidate::Cov7() const
{
  static TMatrixD cov(7,7);
    
  // position error
    
  cov(0,0) = fErrP7[0];  cov(1,0) = fErrP7[1]; cov(1,1) = fErrP7[2];  
  cov(2,0) = fErrP7[3];  cov(2,1) = fErrP7[4]; cov(2,2) = fErrP7[5];
    
  // position-momentum covariance
    
  cov(3,0) = fErrP7[6];   cov(3,1) = fErrP7[7];  cov(3,2) = fErrP7[8];
  cov(4,0) = fErrP7[9];   cov(4,1) = fErrP7[10]; cov(4,2) = fErrP7[11];
  cov(5,0) = fErrP7[12];  cov(5,1) = fErrP7[13]; cov(5,2) = fErrP7[14];
  cov(6,0) = fErrP7[15];  cov(6,1) = fErrP7[16]; cov(6,2) = fErrP7[17];
    
  // momentum error
  cov(3,3) = fErrP7[18];  cov(4,3) = fErrP7[19]; cov(4,4) = fErrP7[20];
  cov(5,3) = fErrP7[21];  cov(5,4) = fErrP7[22]; cov(5,5) = fErrP7[23];
  cov(6,3) = fErrP7[24];  cov(6,4) = fErrP7[25]; cov(6,5) = fErrP7[26]; 
  cov(6,6) = fErrP7[27];
    
  for (int i=0; i<6; i++)
    for (int j=i+1; j<7; j++)
      cov(i,j)=cov(j,i);
   
  return cov;
}

TMatrixD& PndPidCandidate::P4Cov() const
{
  static TMatrixD covP4(4,4);
	
  covP4(0,0) = fErrP7[18];  covP4(1,0) = fErrP7[19]; covP4(1,1) = fErrP7[20];
  covP4(2,0) = fErrP7[21];  covP4(2,1) = fErrP7[22]; covP4(2,2) = fErrP7[23];
  covP4(3,0) = fErrP7[24];  covP4(3,1) = fErrP7[25]; covP4(3,2) = fErrP7[26]; 
  covP4(3,3) = fErrP7[27];
	
  for (int i=0; i<3; i++)
    for (int j=i+1; j<4; j++)
      covP4(i,j)=covP4(j,i);
			
  return covP4;
}

void PndPidCandidate::SetLorentzVector(TLorentzVector &p4)
{
  fXmomentum = p4.X();
  fYmomentum = p4.Y();
  fZmomentum = p4.Z();
  fEnergy    = p4.T();	
}

void PndPidCandidate::SetCov7(const TMatrixD &cov7 )
{
  // position error
    
  fErrP7[0] = cov7(0,0);  fErrP7[1] = cov7(1,0); fErrP7[2] = cov7(1,1);  
  fErrP7[3] = cov7(2,0);  fErrP7[4] = cov7(2,1); fErrP7[5] = cov7(2,2);
    
  // position-momentum covariance
    
  fErrP7[6] = cov7(3,0);   fErrP7[7] = cov7(3,1);  fErrP7[8] = cov7(3,2);
  fErrP7[9] = cov7(4,0);   fErrP7[10] = cov7(4,1); fErrP7[11] = cov7(4,2);
  fErrP7[12] = cov7(5,0);  fErrP7[13] = cov7(5,1); fErrP7[14] = cov7(5,2);
  fErrP7[15] = cov7(6,0);  fErrP7[16] = cov7(6,1); fErrP7[17] = cov7(6,2);
    
  // momentum error
  fErrP7[18] = cov7(3,3);  fErrP7[19] = cov7(4,3); fErrP7[20] = cov7(4,4);
  fErrP7[21] = cov7(5,3);  fErrP7[22] = cov7(5,4); fErrP7[23] = cov7(5,5);
  fErrP7[24] = cov7(6,3);  fErrP7[25] = cov7(6,4); fErrP7[26] = cov7(6,5); 
  fErrP7[27] = cov7(6,6);
}

void PndPidCandidate::SetP4Cov(const TMatrixD &covP4 )
{
  // position error
    
  fErrP7[0] = 0;  fErrP7[1] = 0; fErrP7[2] = 0;  
  fErrP7[3] = 0;  fErrP7[4] = 0; fErrP7[5] = 0;
    
  // position-momentum covariance
    
  fErrP7[6] = 0;   fErrP7[7] = 0;  fErrP7[8] = 0;
  fErrP7[9] = 0;   fErrP7[10] = 0; fErrP7[11] = 0;
  fErrP7[12] = 0;  fErrP7[13] = 0; fErrP7[14] = 0;
  fErrP7[15] = 0;  fErrP7[16] = 0; fErrP7[17] = 0;
    
  // momentum error
  fErrP7[18] = covP4(0,0);  fErrP7[19] = covP4(1,0); fErrP7[20] = covP4(1,1);
  fErrP7[21] = covP4(2,0);  fErrP7[22] = covP4(2,1); fErrP7[23] = covP4(2,2);
  fErrP7[24] = covP4(3,0);  fErrP7[25] = covP4(3,1); fErrP7[26] = covP4(3,2); 
  fErrP7[27] = covP4(3,3);
}

void PndPidCandidate::SetDefault()
{
  fLocked = kFALSE;
  fCharge = 0;
  fXposition = 0.;		
  fYposition = 0.;		
  fZposition = 0.;		
  fXmomentum = 0.;		
  fYmomentum = 0.;		
  fZmomentum = 0.;		
  fEnergy = 0.;		
  fFirstHitX = 0.;		
  fFirstHitY = 0.;		
  fFirstHitZ = 0.;		
  fLastHitX = 0.;		
  fLastHitY = 0.;		
  fLastHitZ = 0.;		
  fMcIndex = -1;
  fTrackIndex = -1;
  fTrackBranch = -1;
#if _OLD_
  fMvdDEDX = 0.;
  fMvdHits = 0;
  fSttMeanDEDX = 0.;
  fSttHits = 0;
  fGemHits = 0; 
  fTofStopTime = 0.;
  fTofM2 = 0.;
  fTofTrackLength = 0.;
  fTofQuality = -1.;
  fTofIndex = -1;
  fDrcThetaC = 0.;
  fDrcThetaCErr = 0.;
  fDrcQuality = -1.;
  fDrcNumberOfPhotons = 0;
  fDrcIndex = -1;
  fDiscThetaC = 0.;
  fDiscThetaCErr = 0.;
  fDiscQuality = -1.;
  fDiscNumberOfPhotons = 0;
  fDiscIndex = -1;
  fRichThetaC = 0.;
  fRichThetaCErr = 0.;
  fRichQuality = -1.;
  fRichNumberOfPhotons = 0;
  fRichIndex = -1;
  fEmcRawEnergy = 0.;
  fEmcCalEnergy = 0.;
  fEmcQuality = -1.;
  fEmcNumberOfCrystals = 0;
  fEmcNumberOfBumps = 0;
  fEmcModule = -1; 
  fEmcIndex = -1;
  fEmcZ20 = 0.;
  fEmcZ53 = 0.;
  fEmcLat = 0.;
  fEmcE1  = 0.;
  fEmcE9  = 0.;
  fEmcE25 = 0.;
  fMuoNumberOfLayers = 0;
  fMuoProbability = 0.;
  fMuoQuality = -1.; 
  fMuoIron = 0.;
  fMuoMomentumIn = 0.;
  fMuoModule = 0; 
  fMuoHits = 0;
  fMuoIndex = -1;
#endif
  fDegreesOfFreedom = 0;
  fFitStatus = 0;
  fChiSquared = 0.;  
  
  Int_t i;
  for (i=0; i<28;i++) fErrP7[i] = 0;
  for (i=0; i<5;i++) fParams[i] = 0;
  for (i=0; i<15;i++) fCov[i] = 0;
}


const NaiveTrackParameterization *PndPidCandidate::GetNearestParameterization(const TVector3 &x0, const TVector3 &n0)
{
  int best_hit = -1;
  double best_dist = 0.0;

  for(unsigned iq=0; iq<mParameterizations.size(); iq++) {
    const NaiveTrackParameterization &param = mParameterizations[iq];
    double dist = param.DistanceToPlane(x0, n0);
    if (best_hit == -1 || dist < best_dist) {
      best_hit  = iq;
      best_dist = dist;
    } //if
  } //for iq

  return (best_hit == -1 ? 0 : &mParameterizations[best_hit]);
} // PndPidCandidate::GetNearestParameterization()

// FIXME: unify these two calls;
const NaiveTrackParameterization *PndPidCandidate::GetNearestParameterization(double r)
{
  int best_hit = -1;
  double best_dist = 0.0;

  for(unsigned iq=0; iq<mParameterizations.size(); iq++) {
    const NaiveTrackParameterization &param = mParameterizations[iq];
    const TVector3 &pos = param.GetMoCaPosition();
    double x = pos.X(), y = pos.Y(), dist = fabs(sqrt(x*x+y*y) - r);
    if (best_hit == -1 || dist < best_dist) {
      best_hit  = iq;
      best_dist = dist;
    } //if
  } //for iq

  return (best_hit == -1 ? 0 : &mParameterizations[best_hit]);
} // PndPidCandidate::GetNearestParameterization()
