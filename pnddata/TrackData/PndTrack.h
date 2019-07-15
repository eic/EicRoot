/*
 * PndTrack.h
 *
 *  Created on: 05.03.2009
 *      Author: everybody
 */

#ifndef PNDTRACK_H_
#define PNDTRACK_H_

#include "TObject.h"
#include "PndTrackCand.h"
#include "FairTrackParP.h"
#include "PndDetectorList.h"
#include "FairTimeStamp.h"
#include "TRef.h"

class NaiveTrackParameterization: public TObject 
{
 public:
 NaiveTrackParameterization(bool valid = false): mValid(valid) {};
  ~NaiveTrackParameterization() {};

  bool IsValid            ( void ) const { return mValid; };
  TVector3 GetMoCaPosition( void ) const { return mMoCaPosition; };
  TVector3 GetMoCaMomentum( void ) const { return mMoCaMomentum; };
  TVector3 GetRecoPosition( void ) const { return mRecoPosition; };
  TVector3 GetRecoMomentum( void ) const { return mRecoMomentum; };

  double DistanceToPlane(const TVector3 &x0, const TVector3 &n0) const {
    // Use MC position here;
    return fabs((x0 - mMoCaPosition).Dot(n0.Unit()));
  };

  void SetValid( void )                          { mValid        = true; };
  void SetMoCaPosition(const TVector3 &position) { mMoCaPosition = position; };
  void SetMoCaMomentum(const TVector3 &momentum) { mMoCaMomentum = momentum; };
  void SetRecoPosition(const TVector3 &position) { mRecoPosition = position; };
  void SetRecoMomentum(const TVector3 &momentum) { mRecoMomentum = momentum; };

 private:
  bool mValid;

  // Simulated and reconstructed 3D positions and momenta at this hit location;
  TVector3 mMoCaPosition, mMoCaMomentum, mRecoPosition, mRecoMomentum;

  ClassDef(NaiveTrackParameterization, 1);
};

class PndTrack : public FairTimeStamp{
public:
	PndTrack();
	PndTrack(const FairTrackParP& first, const FairTrackParP& last, const PndTrackCand& cand,
		 Int_t flag = 0, Double_t chi2 = -1., Int_t ndf = 0, Int_t pid = 0, Int_t id = -1, Int_t type = -1);

	void Print();


	Int_t GetPidHypo()               const { return fPidHypo; }
	Int_t GetFlag()                  const { return fFlag; } //Quality flag
	Double_t GetChi2()               const { return fChi2; }
	Int_t GetNDF()                   const { return fNDF; }
        Int_t GetRefIndex()              const { return fRefIndex; }
	void SetPidHypo(Int_t i)         { fPidHypo=i; }
	void SetFlag(Int_t i)            { fFlag=i; }
	void SetChi2(Double_t d)         { fChi2=d; }
	void SetChiSquareCCDF(Double_t d){ fChiSquareCCDF=d; }
	void SetNDF(Int_t i)             { fNDF=i; }
    void SetRefIndex(TString branch, Int_t i)        { fRefIndex=i; SetLink(FairLink(branch, i)); }
    void SetRefIndex(Int_t i){fRefIndex = i;}
	void SetTrackCand(const PndTrackCand& cand) { fTrackCand = cand; };
	void SetTrackCandRef(PndTrackCand* candPointer){ fRefToPndTrackCand = candPointer; }
	PndTrackCand* GetPndTrackCandViaRef(){ return (PndTrackCand*)fRefToPndTrackCand.GetObject();}
	
	PndTrackCand GetTrackCand()      { return fTrackCand; }
	PndTrackCand* GetTrackCandPtr()  { return &fTrackCand; }
	FairTrackParP GetParamFirst() { return fTrackParamFirst; }
	FairTrackParP GetParamLast()  { return fTrackParamLast ; }

private:
	FairTrackParP fTrackParamFirst;
	FairTrackParP fTrackParamLast;

	PndTrackCand fTrackCand;
	TRef fRefToPndTrackCand;

	Int_t fPidHypo;
	Int_t fFlag;
	Double_t fChi2;
	// Well, it is convenient to have this value handy in ROOT macros 
	// (even that it is redundant, clear);
	Double_t fChiSquareCCDF;
	Int_t fNDF;
        Int_t fRefIndex;

public:
	// Parameterizations at the hit locations;
	std::vector<NaiveTrackParameterization> mParameterizations;

	ClassDef(PndTrack,6)

};

#endif /* PNDTRACK_H_ */
