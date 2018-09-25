/*
 * PndTrack.cpp
 *
 *  Created on: 05.03.2009
 *      Author: stockman
 */

#include <Math/DistFunc.h>

#include "PndTrack.h"

ClassImp(PndTrack);

PndTrack::PndTrack()
{
  fPidHypo = 0;
  fFlag    = 0;
  fChi2    = -1.;
  fChiSquareCCDF    = -1.;
  fNDF     = 0;
  fRefIndex = -1;
}


PndTrack::PndTrack(const FairTrackParP& first, const FairTrackParP& last, const PndTrackCand& cand, Int_t flag, Double_t chi2, Int_t ndf, Int_t pid, Int_t id, Int_t type):
  fTrackParamFirst(first), fTrackParamLast(last), fTrackCand(cand), fPidHypo(pid), fFlag(flag), fChi2(chi2), fNDF(ndf), fRefIndex(id)
{
  fChiSquareCCDF = fNDF > 0 ? ROOT::Math::chisquared_cdf_c(fChi2, fNDF) : 0.;

	SetLink(FairLink(type, id));
	SetTimeStamp(cand.GetTimeStamp());
	SetTimeStampError(cand.GetTimeStampError());
}



void PndTrack::Print(){
  std::cout << "FirstTrackPar" << std::endl;
  fTrackParamFirst.Print();
  std::cout << "LastTrackPar" << std::endl;
  fTrackParamLast.Print();
  std::cout << "chi2 " << fChi2 << std::endl;
  std::cout << "ndf " << fNDF << std::endl;
  std::cout << "fRefIndex " << fRefIndex << std::endl;
}
