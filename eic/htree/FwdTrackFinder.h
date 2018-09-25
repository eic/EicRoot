
#include <map>

#include <TVector3.h>
#include <TH1D.h>

#include <FairTask.h>

#include <EicBoxGenerator.h>
#include <EicHtcTask.h>
//#include <KalmanFilter.h>
#include <EicGeoParData.h>
#include <FwdHoughTree.h>
//#include <FwdKalmanFilter.h>

#ifndef _FWD_TRACK_FINDER_
#define _FWD_TRACK_FINDER_

class ResolutionLevelPattern: public TObject
{
 public:
 ResolutionLevelPattern(): mDiv(0), mCount(0), mDim(0) {};
 ResolutionLevelPattern(const unsigned div[], unsigned dim, unsigned count): 
  mDim(dim), mCount(count) {
    mDiv = new unsigned[dim];

    for(unsigned iq=0; iq<dim; iq++)
      mDiv[iq] = div[iq];   
  };
  ~ResolutionLevelPattern() { if (mDiv) delete [] mDiv; }

  unsigned GetRepetitionCount() const { return mCount; };
  const unsigned *GetDivPtr()   const { return mDiv; }; 

 private:
  unsigned mDim;
  unsigned *mDiv;   // [mDim]
  unsigned mCount;

  ClassDef(ResolutionLevelPattern,2);
};

class PhaseSpaceVariable: public TObject
{
 public:
 PhaseSpaceVariable(): mGra(0.0), mMin(0.0), mMax(0.0) {};  
 PhaseSpaceVariable(double min, double max, double gra): 
  mGra(gra), mMin(min), mMax(max) {};  
  ~PhaseSpaceVariable() {};  

  double GetRange() const { return mMax - mMin; };
  double GetMin()   const { return mMin; };
  double GetMax()   const { return mMax; };
  double GetGra()   const { return mGra; };

 private:
  double mMin, mMax, mGra;

  ClassDef(PhaseSpaceVariable,1)
};

//#define _DEFAULT_EXTRA_GRANULARITY_FACTOR_ (20./6.)
#define _DEFAULT_EXTRA_GRANULARITY_FACTOR_ (2.0)

// Assume 3 sigma is fine in both gaussian and digital case;
#define _DEFAULT_RELATIVE_HIT_SMEARING_ (3.0)

class FwdTrackFinder: public EicHtcTask
{
  //FwdTrackFinderEicHtcTask(): FairTask("EIC HTC Task") { ResetVars(); };
  //EicHtcTask(EicIdealTrackingCode *ideal, MfieldMode fieldMode = WithField);
 public:
 FwdTrackFinder(): EicHtcTask() {};
 FwdTrackFinder(EicIdealTrackingCode *ideal, MfieldMode fieldMode = WithField): 
  EicHtcTask(ideal, fieldMode),
    //mGptr(0), mHits(0), mSmearing(0), 
    mRelativeHitSmearing(_DEFAULT_RELATIVE_HIT_SMEARING_), 
    mAbsoluteSpatialSmearing(0.0),
    mBlindCellDecisionLevel(0),
    //mMinOkHitCounter(0), mMaxOkHitCounter(0), 
    mBorrowedHitCounterMax(0),
    mMeasurementNoiseInflationFactor(1.0),
    mVtxKfNode(0), mEicBoxGenerator(0), 
    mExtraGranularityFactor(_DEFAULT_EXTRA_GRANULARITY_FACTOR_),
    mBorrowedPlusMissingHitCounterMax(0), mCylindricalCoordPreference(false),
    mMissingHitCounterMax(0) {
    // Allocate Hough transform class instance;
    mFwdHoughTree = new FwdHoughTree(this);

    mTracks = new TClonesArray("FwdMatchCandidate"); assert(mTracks);

    mPhiId = mThetaId = mInvMomentumId = mInvPtId = -1;
    mAngularCovMtxEstimate = mInversedMomentumRelatedCovMtxEstimate = 0.0;

    mStoredMinFilterChiSquareCCDF = 0.0;

    ccdf = new TH1D("ccdf", "ccdf", 100, 0.0, 1.0);
  };
  ~FwdTrackFinder() {};

  void ResetVtxNode(MatchCandidate *match);
  void UpdateVtxNode();

  InitStatus Init();
  void Exec(Option_t* opt);
  void FinishTask();
  // FIXME: a temporary hack;
  TH1D *ccdf;

  int DefinePhiRange(double min, double max, double gra);
  // It turns out, that defining p-range in momentum itself is more confusing
  // (because +/- signs need to be accounted and also min.granularity should 
  // be given in inversed momentum anyway);
  int DefineInversedMomentumRange(double min, double max, double gra);
  int DefineInversedPtRange(double min, double max, double gra);
  int DefineThetaRange(double min, double max, double gra);

  // These methods can be propagated to HoughTree right from the reconstruction.C;
  void SetVerbosityLevel(unsigned level) { mFwdHoughTree->SetVerbosityLevel(level); };
  int AddDimension(const char *name, double min, double max) {
    return mFwdHoughTree->AddDimension(name, min, max);
  };
  void SetFastTreeSearchMode(unsigned qualityItrNum) { 
    mFwdHoughTree->SetFastTreeSearchMode(qualityItrNum); 
  };
  //void ResolveAmbiguityViaWorstHit() { mFwdHoughTree->ResolveAmbiguityViaWorstHit(); };

  // Well, not all of the operations with mFwdHoughTree pointer are allowed
  // from reconstruction.C script (say AddResolutionLevel() makes sense 
  // only after plane structure is defined); therefore prefer to formulate
  // "requests" in reconstruction.C, store data in intermediate variables
  // and defer actual calls to HoughTree configuration methods until Init()
  // call happens;
  void AddResolutionLevel(const unsigned div[], unsigned count) {
    mResolutionLevelPatterns.push_back(new ResolutionLevelPattern(div, mFwdHoughTree->GetDdim(), 
								  count));
    //return 0;
  };

  MediaBank *ConfigureMediaBank();

  void SetRelativeHitSmearing(double smearing) { mRelativeHitSmearing = smearing; };
  void SetAbsoluteSpatialSmearing(double smearing) { mAbsoluteSpatialSmearing = smearing; };
  double GetAbsoluteSpatialSmearing() const { return mAbsoluteSpatialSmearing; };

  void SetBorrowedHitCounterLimit(unsigned max) { mBorrowedHitCounterMax = max; };

  void SetMissingHitCounterLimit(unsigned max) {
    mMissingHitCounterMax = max;
  };
  void SetBorrowedPlusMissingHitCounterLimit(unsigned max) {
    mBorrowedPlusMissingHitCounterMax = max;
  };
  void SetBlindCellDecisionLevel(unsigned level) { 
    mBlindCellDecisionLevel = level; 
  };

  void PreferCylindricalCoordinates() { mCylindricalCoordPreference = true; };

  TrKalmanNode *GetVtxNode()            const { return mVtxKfNode; };
  EicBoxGenerator *GetEicBoxGenerator() const { return mEicBoxGenerator; };

  bool WithMagneticField()              const { return GetKalmanFilter()->GetFieldMode() == WithField; };

  void SetMeasurementNoiseInflationFactor(double scale) { mMeasurementNoiseInflationFactor = scale; };
  double GetMeasurementNoiseInflationFactor() const { return mMeasurementNoiseInflationFactor; };

  void SetMinFilterChiSquareCCDF(double value) { 
    mStoredMinFilterChiSquareCCDF = value;
    GetKalmanFilter()->SetMinFilterChiSquareCCDF(value); 
  };
  double GetStoredMinFilterChiSquareCCDF() const { return mStoredMinFilterChiSquareCCDF; };
  void SetLocationSeparationDistance(double value) { 
    GetKalmanFilter()->SetLocationSeparationDistance(value); 
  };
  void SetExtraNdfCount(int count) { GetKalmanFilter()->SetExtraNdfCount(count); };
  void AccountEnergyLosses(bool flag) { GetKalmanFilter()->AccountEnergyLosses(flag); };

  unsigned GetMissingHitCounterMax() const { return mMissingHitCounterMax; };

  void SetExtraGranularityFactor(double value)  { mExtraGranularityFactor = value; };
  double GetExtraGranularityFactor()      const { return mExtraGranularityFactor; };

  int GetThetaId() const { return mThetaId; };
  int GetPhiId()   const { return mPhiId; };
  int GetInvMomentumId() const { return mInvMomentumId; };
  int GetInvPtId() const { return mInvPtId; };
  // NB: can still return -1, fine;
  int GetMomentumRelatedId() const { return (mInvPtId != -1 ? mInvPtId : mInvMomentumId); };

  int ConfigureResolutionLevels(unsigned id/*const PhaseSpaceVariable *psvar*/);

 private:
  // If looks like there is no need to save anything?; FIXME: actually may want
  // to record FwdTrackFinder class;
  FwdHoughTree *mFwdHoughTree;                       //!

  TrKalmanNode *mVtxKfNode;                          //!

  EicBoxGenerator *mEicBoxGenerator;                 //!

  bool mCylindricalCoordPreference;

  double mExtraGranularityFactor;

  // Want to help MappingCall() not to be dependent on the order of 
  // Define*Range() calls;
  int mPhiId, mThetaId, mInvMomentumId, mInvPtId;
  std::vector<PhaseSpaceVariable> mPhaseSpaceVariables;
  double mAngularCovMtxEstimate, mInversedMomentumRelatedCovMtxEstimate;

  std::vector<ResolutionLevelPattern*> mResolutionLevelPatterns;

  // See HoughTree class description; same quantities, booked from 
  // reconstruction.C script for defered application in HoughTree::Init() calls;
  unsigned mBorrowedHitCounterMax, mBorrowedPlusMissingHitCounterMax;
  //unsigned mMinOkHitCounter, mMaxOkHitCounter;
  unsigned mMissingHitCounterMax;
  unsigned mBlindCellDecisionLevel;

  double mMeasurementNoiseInflationFactor;

  TClonesArray *mTracks;       //! Array of found tracks

  // Smear hit range by +/- that many *sigma* (defined by respective digi template);
  double mRelativeHitSmearing;  
  // Smear cell range by some number matching this value in [cm]; in case of 
  // XY- and R-measurements this will be just divided by spatial granularity when 
  // given to the ngroup->OffsetThisValue() call in HoughTree::CheckCell(); in case
  // of A-measurements respective smearing will be guessed based on min.radius;
  double mAbsoluteSpatialSmearing;

  double mStoredMinFilterChiSquareCCDF;

  ClassDef(FwdTrackFinder,17);
};

#endif
