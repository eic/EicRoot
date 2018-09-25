//
// AYK (ayk@bnl.gov), 2014/02/03
//
//  An attempt to port HERMES & OLYMPUS forward tracking KF code to EicRoot;
//

//#include <3d.h>
//#include <htclib.h>

#ifndef _EIC_HTC_TASK_
#define _EIC_HTC_TASK_

class TGeoNode;

// FIXME: save on typing?; really?;
#define _EIC_HTC_TREE_         "htc"
#define _EIC_HTC_BRANCH_       "EicHtcTrack"
#define _EIC_HTC_TRACK_        "track"

#include <TGeoMatrix.h>

#include <FairTask.h>

#include <TrKalmanFilter.h>
#include <FairTrackParP.h>

class EicTrackingDigiHit;

#include <EicIdealTrackingCode.h>
#include <EicTrackingDigiHitProducer.h>

class EicHtcHitComponent: public TObject {
  // No reason to overcomplicate access here;
  friend class EicHtcHit;
  friend class EicHtcTask;

 public:
  EicHtcHitComponent() { 
    mLocalCoord1D = mXsResidual = mXmResidual = mSigmaRS = mSigmaRM = mResolution = 0.0;
  };
  ~EicHtcHitComponent() {}; 

 private:
  Double_t mLocalCoord1D;    // local coordinate

  Double_t mXsResidual;      // smoothed residual       (~inclusive in Aiwu's notation)
  Double_t mXmResidual;      // smoothed-subtracted one (~exclusive in Aiwu's notation)

  Double_t mResolution;      // 1D sigma used for track fitting (and smearing in case of MC data)

  // This is in node-projected space (H*M*H^T); assume only diagonal terms of respective 
  // cov.matrix are of interest; otherwise will have to put full matrix right into 
  // the EicHtcHit class;
  Double_t mSigmaRS;         // sqrt(diagonal term) of H-projected smoother cov.matrix
  Double_t mSigmaRM;         // same as above, without information from this KF node

  ClassDef(EicHtcHitComponent,4);
};

class EicHtcHit: public TObject {
  friend class EicHtcTask;

 public:
 EicHtcHit(unsigned dim = 0): mDim(dim) { 
    mSmootherChiSquare = mSmootherProbability = 0.0;
 
    //memset(mSigmaCS, 0x00, sizeof(mSigmaCS));
    mGlobalCoordXY[0] = mGlobalCoordXY[1] = 0.0;

    mComponents = dim ? new EicHtcHitComponent[dim] : 0;
  };
  ~EicHtcHit() {};

  double ChiSq()  const { return mSmootherChiSquare; };
  double Prob()   const { return mSmootherProbability; };

  // Hit resolution (sqrt of diagonal term);
  double GetResolution(unsigned what = 0) const { 
    return what < mDim ? mComponents[what].mResolution : 0.0;
  };

  // Local coordinate(s);
  double GetLc(unsigned what = 0) const { 
    return what < mDim ? mComponents[what].mLocalCoord1D : 0.0;
  };
  // Global XY-coordinates;
  double GetGc(unsigned what = 0) const { 
    return what < 2 ? mGlobalCoordXY[what] : 0.0;
  };

  // Smoother residuals and estimated error (component onto detector 
  // plane coordinate system); aka "inclusive residual";
  double GetRs(unsigned what = 0) const { 
    return what < mDim ? mComponents[what].mXsResidual : 0.0; 
  };
  double GetEs(unsigned what = 0) const { 
    return what < mDim ? mComponents[what].mSigmaRS : 0.0;
  };

  // Same as above, but with the present KF node measurement excluded;
  // aka "exclusive residual";
  double GetRm(unsigned what = 0) const { 
    return what < mDim ? mComponents[what].mXmResidual : 0.0; 
  };
  double GetEm(unsigned what = 0) const { 
    return what < mDim ? mComponents[what].mSigmaRM : 0.0; 
  };

  private:
  UInt_t mDim;
  EicHtcHitComponent *mComponents; //[mDim] up to 2 components for now

  Double_t mSmootherChiSquare;     // chi^2 after Kalman smoother pass for this node
  Double_t mSmootherProbability;   // "probability" of this chi^2 value

  Double_t mGlobalCoordXY[2];      // global XY-coordinates at this node

  // FIXME: may later want to add both state vector, as well as the full smoother 
  // covariance matrix; not really needed for FLYSUB business right now;
  //
  // This is the "global" 3D space, {x,y,sx,sy} or {x,y,sx,sy,1/p} track parameterization;
  //Double_t mSigmaCS[5];            // sqrt(diagonal terms) of smoother cov.matrix in 3D

  ClassDef(EicHtcHit,12);
};

// NB: "tuple" is available in C++ 11 only, sorry;
typedef std::pair<const char*, std::pair<unsigned, unsigned> > kEntry;
typedef std::map<kEntry, EicHtcHit*, bool(*)(kEntry, kEntry)> hEntry;
bool HitKeyCompare(kEntry, kEntry);

class EicHtcTrack: public TObject {
  friend class EicHtcTask;

 public:
 EicHtcTrack(): mMomentum(0.0), mNdf(0), mFilterChiSquare(0.0), mFilterChiSquareCCDF(0.0) {
    mHits = new hEntry(HitKeyCompare);

    memset(mBeamCoordXY,      0x00, sizeof(mBeamCoordXY));
    memset(mBeamCoordSigmaXY, 0x00, sizeof(mBeamCoordSigmaXY));

    memset(mBeamSlopeXY,      0x00, sizeof(mBeamSlopeXY));
    memset(mBeamSlopeSigmaXY, 0x00, sizeof(mBeamSlopeSigmaXY));
  };
  ~EicHtcTrack() { if (mHits) delete mHits; };

  // Simplify access method names (typing issues);
  int Ndf()              const { return mNdf; };
  double ChiSquare()     const { return mFilterChiSquare; };
  double ChiSquareCCDF() const { return mFilterChiSquareCCDF; };

  double BeamX()         const { return mBeamCoordXY     [0]; };
  double BeamEX()        const { return mBeamCoordSigmaXY[0]; };
  double BeamY()         const { return mBeamCoordXY     [1]; };
  double BeamEY()        const { return mBeamCoordSigmaXY[1]; };

  double BeamSX()        const { return mBeamSlopeXY     [0]; };
  double BeamESX()       const { return mBeamSlopeSigmaXY[0]; };
  double BeamSY()        const { return mBeamSlopeXY     [1]; };
  double BeamESY()       const { return mBeamSlopeSigmaXY[1]; };

  const EicHtcHit* GetHit(const char *detName, unsigned group, unsigned node = 0) const;

  Double_t mMomentum;

  private:
  // General parameters;
  Int_t mNdf;                   // number of degrees of freedom
  Double_t mFilterChiSquare;    // chi^2 after Kalman filter pass for the whole track
  Double_t mFilterChiSquareCCDF;// respective complement of the cumulative distribution function

  Double_t mBeamCoordXY[2];     // beam coordinate estimate at the KF head node
  Double_t mBeamCoordSigmaXY[2];// respective diagonal errors
  Double_t mBeamSlopeXY[2];     // beam slope estimate at the KF head node
  Double_t mBeamSlopeSigmaXY[2];// respective diagonal errors
  
  // Track/hit info at registering plane locations;
  hEntry *mHits;

  ClassDef(EicHtcTrack,9);
};

class HtcKalmanFilter: public TrKalmanFilter {
 public:
 HtcKalmanFilter(): TrKalmanFilter() {};
 HtcKalmanFilter(MfieldMode mode): TrKalmanFilter(mode), mMgslices(0) {};
  ~HtcKalmanFilter() {};

  int KalmanFilterMagneticField(TVector3 &xx, TVector3 &B);

 private:
  MgridSlice *mMgslices;

  MgridSlice *InitializeMgridSlice(double z0);
};

class EicHtcTask: public FairTask {
 public:

 EicHtcTask(): FairTask("EIC HTC Task") { ResetVars(); };
  EicHtcTask(EicIdealTrackingCode *ideal, MfieldMode fieldMode = WithField);

  void ResetVars() { 
    mMediaBank = 0; mFitTrackArray = 0;

    mHtcBranch = 0; mHtcTrack = 0; mPersistency = true;
    mParticleMomentumSeed = 30.0;

    mCoordinateScaleXY = mSlopeScale = mResidualScaleXY = 1.0;

    mKalmanFilter = 0;

    mMediaScanDirection = TVector3(0.0, 0.0, 1.0);
  };

  ~EicHtcTask() {};

  // Want to propagate detector group names to the Kalman filter initialization;
  InitStatus Init();

  void Exec(Option_t* opt);

  void Print(Option_t* option = "") const;

  void FinishTask();

  void SetTrackOutBranchName(const TString& name)  { mTrackOutBranchName = name; } 

  // Basically "proton" or "pion"; THINK: momentum = 0.0 means: take 
  // simulated one as a seed; or perhaps the one from EicIdealTrack?;
  int SetParticleHypothesis(const char *name, double momentumSeed = 0.0) {
    if (name) mParticleHypothesis = TString(name);
    mParticleMomentumSeed = momentumSeed;

    return 0;
  };

  // 'virtual': well, let user code an opportunity to explicitely specify both axis 
  // and scan 3D lines in space explicitely;
  virtual MediaBank *ConfigureMediaBank();

  void SetMediaScanThetaPhi(double theta, double phi);
  TVector3 GetMediaScanDirection() const { return mMediaScanDirection; };

  // May want to change coordinate and residual scale in the encoded hits to 
  // something like [mm] and [um] (default is 1:1, so [cm]);
  void SetHitOutputCoordinateScaleXY(double scale) { mCoordinateScaleXY = scale; };
  void SetHitOutputSlopeScale(double scale)        { mSlopeScale        = scale; };
  void SetHitOutputResidualScaleXY(double scale)   { mResidualScaleXY   = scale; };

  void SetResolutionByHand(const char *plName, double value) {
    if (plName) mResolutionsByHand[TString(plName)] = value;
  };
  double GetResolutionByHand(const char *plName) {
    //printf("%d\n", mResolutionsByHand.size());
    if (plName && mResolutionsByHand.find(TString(plName)) != mResolutionsByHand.end()) 
      return mResolutionsByHand[TString(plName)];
    else
      return 0.0;
  };

  HtcKalmanFilter *GetKalmanFilter() const { return mKalmanFilter; };

  virtual unsigned GetMissingHitCounterMax() const { return 0; };

 protected:
  EicIdealTrackingCode *mIdealTrCode;//!

  HtcKalmanFilter *mKalmanFilter;    //! Kalman filter pointer

  //void SetMaxPossibleHitCount(unsigned max)  { mMaxPossibleHitCount = max; };
  unsigned GetMaxPossibleHitCount()  const;// { 
    //return mMaxPossibleHitCount; 
  //};

private:
  MediaBank *mMediaBank;             //!

  TClonesArray* mFitTrackArray;      //! Output TCA for track

  TString mTrackOutBranchName;       //! Name of the output TCA

  TBranch *mHtcBranch;               //! HTC track branch
  EicHtcTrack *mHtcTrack;            //! track buffer to be fed to tree->Fill()
  Bool_t mPersistency;               //!  

  TVector3 mMediaScanDirection;

 public:
  TString mParticleHypothesis;       // particle hypothesis used for fitting
  // NB: this will remain constant for magnet-off mode;
  Double_t mParticleMomentumSeed;    // particle momentum seed used for fitting

  //
  // FIXME: not radians in fact; need atan() somewhere -> check!;
  //
 private:
  // Local 1D spatial detector coordinates (say XY) and beam profile XY-coordinates; 
  Double_t mCoordinateScaleXY;       // [cm]  may be rescaled to say [mm]
  // Beam profile XY-slopes;
  Double_t mSlopeScale;              // [rad] may be rescaled to say [urad]
  // Local 1D spatial detector residuals (say XY);
  Double_t mResidualScaleXY;         // [cm]  may be rescaled to say [um]

  std::map<TString, double> mResolutionsByHand; // resolutions set by hand in reconstruction.C

  //unsigned mMaxPossibleHitCount;

  int PerformMediaScan();
  int DeclareSensitiveVolumes();
  int ConfigureKalmanFilter();

  int ConstructLinearTrackApproximation(KfMatrix *A, KfMatrix *b);

  FairTrackParP GetFairTrackParP(TrKalmanNode *node);

  ClassDef(EicHtcTask,22);
};

#endif
