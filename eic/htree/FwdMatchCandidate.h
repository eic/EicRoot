//
// AYK (ayk@bnl.gov)
//
//  STAR forward track candidate class;
//
//

#include <TVector3.h>

#include <Math/DistFunc.h>

#include <MatchCandidate.h>

#ifndef _FWD_MATCH_CANDIDATE_
#define _FWD_MATCH_CANDIDATE_

class HoughTree;

class FwdMatchCandidate: public MatchCandidate {
 public:
 FwdMatchCandidate(const HoughTree *tree = 0): MatchCandidate(tree),
    mNdf(0), mFilterChiSquare(0.0), mFilterChiSquareCCDF(0.0), mMcTrackId(-1), 
    mWrongHitCount(0), mMcTrackAssociationAmbiguous(false), mCharge(0), 
    mPassedKalmanFilterOnce(false)
    {
#if _LATER_
  //mAlreadyAccountedAsGoodTrack(false),
  mPassedKalmanFilter(false),
#endif
#if _LATER_
  mUsedInFit   = new int[gdim];
#endif

  //memset(mCoord,      0x00, sizeof(mCoord));
  //memset(mSlope,      0x00, sizeof(mSlope));
  memset(mCoordSigma, 0x00, sizeof(mCoordSigma));
  memset(mSlopeSigma, 0x00, sizeof(mSlopeSigma));
  };
  ~FwdMatchCandidate() {};

  void ShapeItUpForInspection(const HoughTree *tree, const unsigned id[]) {
    mPassedKalmanFilterOnce = false;

    MatchCandidate::ShapeItUpForInspection(tree, id);
  };
  void AssertKalmanFilterPassedFlag() { mPassedKalmanFilterOnce = true; };
  bool IsReadyForFinalFit() const {
    return (mPassedKalmanFilterOnce && !HasAmbiguousHits());
  };

  void SetFilterChiSquare(double chiSquare, double chiSquareCCDF, unsigned ndf) { 
    mNdf                 = ndf;
    mFilterChiSquare     = chiSquare;

    mFilterChiSquareCCDF = chiSquareCCDF;
  };

  void SetMcTrackId(int id)                  { mMcTrackId = id; };
  int  GetMcTrackId()                  const { return mMcTrackId; };
  void SetWrongHitCount(unsigned count)      { mWrongHitCount = count; };
  unsigned GetWrongHitCount()          const { return mWrongHitCount; };
  void SetAmbiguityFlag(bool flag)           { mMcTrackAssociationAmbiguous = flag; };
  bool IsAmbiguous()                   const { return mMcTrackAssociationAmbiguous; };

  void SetVtxCoord(double x, double y, double z) { mVertex = TVector3(x, y, z); };
  void SetVtxMomentum(double sx, double sy, double invp);
  void SetVtxCoordSigma(double sigmaX, double sigmaY) {
    mCoordSigma[0] = sigmaX;
    mCoordSigma[1] = sigmaY;
  };
  void SetVtxSlopeSigma(double sigmaX, double sigmaY) {
    mSlopeSigma[0] = sigmaX;
    mSlopeSigma[1] = sigmaY;
  };

  double GetX()                        const { return mVertex[0]; };
  double GetY()                        const { return mVertex[1]; };
  double GetZ()                        const { return mVertex[2]; };
  double GetVtxCoordSigma(unsigned xy) const { return xy <= 1 ? mCoordSigma[xy] : 0.0; };
  double GetVtxSlopeSigma(unsigned xy) const { return xy <= 1 ? mSlopeSigma[xy] : 0.0; };

  int GetFilterNdf()                   const { return mNdf; };
  double GetFilterChiSquareCCDF()      const { return mFilterChiSquareCCDF; };
  // NB: this is defined as a pure virtual method in the base MatchCandidate class;
  double GetTrackQualityParameter()    const { return GetFilterChiSquareCCDF(); };

  const TVector3 &GetMomentum()        const { return mMomentum; };
  double GetSlopeX()                   const { return mMomentum[2] ? mMomentum[0]/mMomentum[2] : 0.0; };
  double GetSlopeY()                   const { return mMomentum[2] ? mMomentum[1]/mMomentum[2] : 0.0; };
  double GetP()                        const { return mMomentum.Mag(); }; 
  int GetCharge()                      const { return mCharge; };

 private:
  // Fit results; chi^2, ndf (assume can not go negative for a good track), 
  // respective CCDF value;
  unsigned mNdf;
  double mFilterChiSquare, mFilterChiSquareCCDF;

  // MC track ID (based on the highest hit count);  
  int mMcTrackId;
  // Number of hits from other track(s);
  unsigned mWrongHitCount;
  // Well, basically 2 MC tracks have equal hit count in this track;
  // FIXME: eventually may want to distribute RC tracks over MC ones in such 
  // a way, that to maximize found MC tracks (in cases of heavy ambiguities);
  // say if two RC tracks have 5+5 hits from wto MC tracks, makes sense to 
  // force 1->1 relationship rather than one of the MC tracks will occasionally
  // remain "idle"; yet I guess track parameters in this case will be totally 
  // screwed up, so why bother?;
  bool mMcTrackAssociationAmbiguous;

  // Estimated vertex location (well, Z-vertex was fixed) and 3D momentum;
  TVector3 mVertex, mMomentum;
  // Estimated slope and coordinate errors at the nominal vertex location;
  double mCoordSigma[2], mSlopeSigma[2];

  int mCharge;

  // Indicates that this track candidate passed through KF chain;
  bool mPassedKalmanFilterOnce;

#if _LATER_  

  // Once a given track candidate was selected to be BEST during one of the 
  // iterations in launchHtreePatternFinder(), it is marked by this flag and
  // will not be considered for further BEST comparisons any longer;
  //bool mAlreadyAccountedAsGoodTrack;

  // Either member ID or -1 (if had to discard this plane); for now assume, that 
  // only one hit per plane group can survive; FIXME: may want to account >1 later;
  int *mUsedInFit;
#endif

  ClassDef(FwdMatchCandidate,5)
};

#endif
