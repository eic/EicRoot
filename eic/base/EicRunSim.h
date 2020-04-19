//
// AYK (ayk@bnl.gov), 2014/08/22
//
//  A trivial (for now) extension of FairRunSim class;
//

#include <TVector3.h>

#include <FairRunSim.h>
#include <FairModule.h>
#include <FairGenerator.h>
#include <FairField.h>

#include <PndMultiField.h>

#include <EicDetector.h>

#ifndef _EIC_RUN_SIM_
#define _EIC_RUN_SIM_

#define _SEED_DEFAULT_ 0x12345678

#define _GPHYSI_DAT_   "gphysi.dat"

class FluxMonitorParticleType: public TObject 
{
  friend class FluxMonitorGrid;

 public:
  FluxMonitorParticleType() { ResetVars(); };
 FluxMonitorParticleType(int pdg, double eMin = 0.0, double eMax = 0.0):
  mPDG(pdg), mEmin(eMin), mEmax(eMax) {};
  ~FluxMonitorParticleType() {};

 private:
  Int_t mPDG;      // particle type
  Double_t mEmin;  // min kin.energy
  Double_t mEmax;  // max kin.energy

  // Transient track length, density*length and dE/dx arrays;
  Double_t *mLength; //! 
  Double_t *mDensityLength; //! 
  Double_t *mEdep; //! 

  void ResetVars() {
    mPDG = 0;
    mEmin = mEmax = 0.0;

    mLength = mDensityLength = mEdep = 0;
  };

  ClassDef(FluxMonitorParticleType, 1)
}; 

class FluxMonitorGrid: public TObject 
{
 public:
  FluxMonitorGrid() { ResetVars(); };
  FluxMonitorGrid(double rMax, unsigned rDim, double zMin, double zMax, unsigned zDim);
  ~FluxMonitorGrid() {};

  int AddFluxMonitorParticleType(int pdg, double eMin = 0.0, double eMax = 0.0);
  void AddEntry(int pdg, double eKin, double dE, TVector3 in, TVector3 out);

  void FillOutHistograms(unsigned evStat);

  void SetRadiationMapType(bool what = true) { mRadiationMapType = what; };
  // NB: use this call only if you really understand what you are doing; otherwise
  // consider to use input .root file with MC events, which was properly put together
  // via eic-smear BuildTree() call (so ASCII event generator file and a matching log
  // file were used); in the latter case normalization is properly extracted from 
  // this .root file; limitations apply, see comments in FluxMonitorGrid::FillOutHistograms()
  // source code;
  void SetNormalization(double xsec, unsigned trials/*, unsigned all_events*/) { 
    mNormalizedCrossSection = xsec;
    mTrials = mTotalOriginalStat = trials;
    //mTotalOriginalStat = all_events;
  }

 private:
  void ResetVars() {
    mRdim = mZdim = mTrials = mTotalOriginalStat = 0;
    mRmax = mZmin = mZmax = mZbwidth = mRbwidth = mNormalizedCrossSection = 0.0;

    mRadiationMapType = false;
    //mDensity = 0;
  };

  // Assume {RZ}-grid symmetric wrt the beam axis is fine;
  Double_t mRmax;                                  // grid max radius
  Double_t mZmin;                                  // grid min Z along the beam line
  Double_t mZmax;                                  // grid max Z along the beam line
  UInt_t mRdim;                                    // grid granularity in radius
  UInt_t mZdim;                                    // grid granularity in Z

  Double_t mRbwidth;                               // bin width in R direction
  Double_t mZbwidth;                               // bin width in Z direction

  // For neutrons want Fisyak-like variable; for radiation map just dE/dx losses map; 
  Bool_t mRadiationMapType;                        // crossing flux (if false) or dE/dx rad.map

  // Normalized cross-section in [microbarn], total number of trials in the MC file 
  // and total number of events in the original generator file;
  Double_t mNormalizedCrossSection;
  UInt_t mTrials;
  UInt_t mTotalOriginalStat;
  
  // Transient RxZ grid cell material density array; FIXME: implementation is 
  // clearly suboptimal
  //Double_t *mDensity; //! 

  // NB: energy ranges can overlap; this why in particular do not see a good
  // reason to use more tricky type than std::vector here;
  std::vector<FluxMonitorParticleType> mParticles; // particle types and energy range

  ClassDef(FluxMonitorGrid, 3)
}; 

class EicRunSim : public FairRunSim 
{
 public:
  EicRunSim(const char *engine = 0);
  ~EicRunSim() {};

  enum InitializationState {stUndefined, stFailed, stSucceeded};

  // Singelton instance get method;
  static EicRunSim* Instance() { return mInstance; };
 
  // Prefer to match AddModule() name (even that this works for EicDetector only);
  EicDetector *GetModule(const char *detName) const {
    return dynamic_cast<EicDetector*>(ListOfModules->FindObject(detName));
  };
  
  //
  // There is a number of places in FairRoot where it deals with secondaries:
  //
  //   - PndStack::StoreSecondaries() in either g3Config.C or g4Config.C;
  //   - /mcTracking/saveSecondaries in g4config.in;
  //   - FairTrajFilter::SetStoreSecondaries() in simulation.C;
  //
  // Unless I'm blind, all this have to do with *output* only (namely what should 
  // be saved in simulation.root file); somehow the functionality to suppress
  // secondaries completely (a la GEANT3 GSKING() call) is missing; 
  //
  // NB: the functionality implemented in EicRunSim here is not clean; secondaries 
  // will not be killed at their production vertex, but only once they enter any 
  // of the detector sensitive volumes; which is fine, since they will not produce 
  // any hits then (which was the primary purpose of this excercise); will still affect 
  // the performance due to the interaction with dead material volumes;
  //
  void SuppressSecondaries()           { mSuppressSecondariesFlag = true; };
  bool SuppressSecondariesFlag() const { return mSuppressSecondariesFlag; };

  void IgnoreBlackHoleVolumes()           { mIgnoreBlackHoleVolumes = true; };
  bool IgnoreBlackHoleVolumesFlag() const { return mIgnoreBlackHoleVolumes; };

  int DefineFluxMonitorGrid(double rMax, unsigned rDim, double zMin, double zMax, unsigned zDim);
  int AddFluxMonitorParticleType(int pdg, double eMin = 0.0, double eMax = 0.0);
  FluxMonitorGrid* GetFluxMonitorGrid() { return mFluxMonitorGrid; };

  // Do not mind to use the same FairRunSim method names and make then virtual 
  // in the base class;
  void SetMaterials(const char *mediaFileName);
  void SetOutputFile(const char *outputFileName);
  void AddModule(FairModule *module);

  // For completeness; actually sometimes need to define a bigger cave;
  void SetCaveFileName(const char *caveFileName);

  void Init();
  void Run(Int_t NEvents = 0, Int_t NotUsed = 0);
  void RunCoreStart(Int_t NEvents = 0, Int_t NotUsed = 0);
  bool JanaLoopPossible( void ) const { return false; }

  void SetSeed(unsigned seed)  { mSeed = seed; };
  void SuppressTimerInfo()     { mTimerFlag = false; };

  void SuppressHitProduction()           { mSuppressHitProductionFlag = true; };
  bool SuppressHitProductionFlag() const { return mSuppressHitProductionFlag; };

  void SuppressFairRootSteppingCall()           { mSuppressFairRootSteppingCallFlag = true; };
  bool SuppressFairRootSteppingCallFlag() const { return mSuppressFairRootSteppingCallFlag; };

  // Just to avoid FairPrimaryGenerator-related black magic in simulation.C script;
  void AddGenerator(FairGenerator *generator);

  // Also avoid field-definition-related black magic in simulation.C script;
  // typically need just PndMultiField which can happily be a default option
  // hidden inside this AddField() call; of course old scheme with calling 
  // "new PndMultiField();" by hand is available as well; low Q^2 tagger and Co 
  // (which need more sofisticated field composition) can still invoke EicMagneticField;
  void AddField(FairField *field);

 private:
  // THINK: can probably live with dynamic cast to respective FairRunSim method?;
  static EicRunSim *mInstance;      //! singelton instance

  // FairRun::Init() conveniently does not return any value; arrange a work-around;
  InitializationState mInitState;    // EicRunSim::Init() call state
  
  // NB: having default here is not necessarily a good idea; so for now do not 
  // have this mechanism in EicRunAna class; THINK later;
  TString mOutputFileName;           // output file name; default: simulation.root

  TString mMediaFileName;            // media file name;  default: media.geo
  TString mCaveFileName;             // file name with cave description; default: cave.geo

  UInt_t mSeed;                      // random seed; default: 0x12345678

  // AddModule() checks for this flag and defines cave if not yet done
  Bool_t mCaveDefinedFlag;           //! 
  Bool_t mSuppressSecondariesFlag;   // try best to suppress secondaries; off by default
  Bool_t mTimerFlag;                 // either printout or not timer info; default: true
  
  // It looks like I do not need more than one grid?; otherwise change
  // later to a vector or something like this;
  FluxMonitorGrid *mFluxMonitorGrid; // flux monitor grid parameters

  Bool_t mIgnoreBlackHoleVolumes;    // may want to ignore those (say for neutron flux calculations)

  Bool_t mSuppressHitProductionFlag;        // may want to skip all the EicDetector::ProcessHits() calls ...
  Bool_t mSuppressFairRootSteppingCallFlag; // ... or FairMCApplication::Stepping() alltogether

  // Help simplify field definition calls -> need this transient variable;
  PndMultiField *mField;             //!

  ClassDef(EicRunSim, 5)
};

class EicFluxMonitorTask: public FairTask {
 public:
 EicFluxMonitorTask(): mStat(0) {};
  ~EicFluxMonitorTask() {};
  
  // The only two methods I need here;
  void FinishEvent() { mStat++; FairTask::FinishEvent(); };
  void FinishTask();
  
 private:
  // Well, do not mind to maintain local event counter;
  UInt_t mStat; //!

  ClassDef(EicFluxMonitorTask,2);
};

#endif
