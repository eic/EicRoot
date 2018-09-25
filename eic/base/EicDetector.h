//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Basic EIC detector class;
//

#include <ostream> 

#ifndef _EIC_DETECTOR_
#define _EIC_DETECTOR_

#include <TLorentzVector.h>
#include "TVector3.h"
class TClonesArray;

class FairVolume;
#include "FairDetector.h"

#include <PndDetectorList.h>

#include <EicBlackHole.h>
#include <EicEnergyMonitor.h>
#include <EicDetName.h>
#include <EicNamePatternHub.h>
#include <EicGeoPar.h>
#include <EicContFact.h>
#include <EicGeoParData.h>
#include <EicMoCaPoint.h>
//#include <EicFakeMoCaPoint.h>
//#include <EicFakeMoCaPointDbEntry.h>

/// \brief Detector IDs, following logic behind PandaRoot DetectorId enum usage
///
/// \note As far I see functionality of GetNPoints()/SetNPoints() calls will not 
/// suffer if EIC-specific IDs starting from exactly the same 0 are used (?);
/// just assume PandaRoot-specific 2 bits per detector suffice; may want 
/// to introduce a better counting scheme later; NB: there is a check in 
/// EicDetector() constructor, that requested ID does not exceed PandaRoot DetectorID::kHYP.
enum EicDetectorId {qDUMMY, qTPC, qFST, qFGT, qBST, qCEMC, qBEMC, qFEMC, qVST, qBGT, qFHAC, qBHAC};

/// \brief Stepping type for this detector active volume (either merge GEANT steps in one hit or not)
///
/// \note Requires certain care; max.step in media.geo file should limit step value, 
/// but also (if this limit is too small), steps should be merged into one hit 
/// in default digitization procedure.
enum SteppingType {qSteppingTypeUndefined = 0, qOneStepOneHit, qMergeStepsInOneHit, qSteppingTypeDefault};


/// EIC detector 
class EicDetector : public FairDetector {
  friend class EicMCApplication;

public:
  /// Main constructor
  ///
  /// @param Name         detector name like \a Tpc
  /// @param geometryName geometry file name like \a tpc.geo, relative to ${VMCWORKDIR}/geometry (unless
  ///                     starts with either "./" or "/", then it is an absolute path)
  /// @param dType        \ref EicDetectorId detector type
  /// @param stType       \ref SteppingType stepping type for this detector active volume
  /// @param Active       if kFALSE, detector is "de-activated" on FairDetector level (preserve this 
  ///                     PandaRoot functionality, whatever it is good for)
  ///
  /// \note As of June'2013 do not allow default value for OneStepOneHit (it is just dangerous, 
  /// since eg. can lead to production of several hits in a silicon layer instead of one); 
  /// so user should better specify this parameter explicitely;
  EicDetector(const char *Name, char *geometryName, EicDetectorId dType = qDUMMY, 
	      SteppingType stType = qSteppingTypeUndefined, Bool_t Active  = kTRUE);

  /*! Default constructor; */
  EicDetector() { 
    //printf("Entering default constructor ...\n"); //exit(0);
    ResetVars(); 
  };
  
  void ResetVars() {
    dname = 0; eicContFact = 0; //fEicFakeMoCaPointCollection = 0; 
    fEicMoCaPointCollection = 0;

    fTime = fELoss = fStep = fLength = 0.0;

    mPrintGeometryInfoFlag = false;

    mAllVolumesSensitiveFlag = false;
    fListOfGeantSensitives = 0;
    gptr = 0; vptr = 0; 

    //dbEntry = 0; dbFile = 0; dbTree = 0; _fCutOffMap = 0;

    mKillerVolumes = 0;
  };

  /*! Destructor, you guessed it; */
  ~EicDetector();
  
  /*! Initialization call, see source file for details; */
  virtual void Initialize();

  /// Final operations; called at the end of run to merge geometry and other service info 
  /// into the output ROOT file
  virtual void FinishRun();

  /// Main stepping function
  ///
  /// @param v current GEANT volume
  virtual Bool_t ProcessHits( FairVolume *v=0);

  /*! Register produced true and fake MC hit collections in FairRootManager; */
  virtual void Register();

  /*! Print out service info about current event; */
  virtual void Print() const;
  
  /*! Has to be called after each event to reset the containers; */
  virtual void   Reset();
  
  // May be called from more than one detector; does not hurt;
  virtual void   BeginEvent() { 
    if (gptr && gptr->GetBlackHoleVolumes().size()) EicBlackHole::ResetTrackList(); 
  };

  /*! Create the detector geometry; */
  virtual void ConstructGeometry();
    
  /// Declare sensitive volumes
  ///
  /// @param name volume name as appears in respective .geo or .root geometry file
  ///
  /// \note Well, ROOT geometry does not allow to account for "volume sensitivity" in 
  /// the media file; or FairRoot does not allow; or I'm lazy/stupid to find this out;
  /// NB: EicRoot ROOT geometry provides an option to encode sensitive volume name 
  /// patterns into the GEANT geometry file; also want to have an option to define 
  /// all volumes sensitive without resorting back to using "" pattern or such;
  void DeclareAllVolumesSensitive() { mAllVolumesSensitiveFlag = true; };
  int DeclareGeantSensitiveVolume(const char *name, SteppingType stType = qSteppingTypeDefault);
  int DeclareGeantSensitiveVolumePrefix(const char *name, SteppingType stType = qSteppingTypeDefault);
  /// Returns \a true if this volume is declared as a sensitive one
  ///
  /// @param name volume name as appears in respective .geo or .root geometry file
  bool CheckIfSensitive(std::string name);

  /// Calculate unique node identifier in GEANT geometry hierarchy
  ///
  /// Basically figures out parent node daughter IDs up to the depth sufficient 
  /// to establish this node's unique geometric mapping ID; 
  ///
  /// \note Well, at some point I should probably figure out how FairRoot sensor numbering 
  /// scheme works (and is it any better than this routine or not); also, may want to 
  /// create a look-up table if this routine ever becomes too expensive;
  ULong64_t GetNodeMultiIndex();

  /*! Get pointer to the MoCa point TClonesArray, whetever it is good for; */
  virtual TClonesArray* GetCollection(Int_t iColl) const ;

  void ResetSteppingVariables();

  /// Add one more **true** MC point (hit) to the output TClonesArray
  ///
  /// @param trackID         current track identifier in MC stack
  /// @param primaryMotherId primary GEANT mother track ID; see also \ref GetPrimaryMotherId()
  /// @param detID           current GEANT volume ID
  /// @param volumePath      full ROOT path to the current volume
  /// @param multiIndex      current node unique geometric multi-index; see also \ref GetNodeMultiIndex()
  /// @param PosIn           track (global) position at volume entry
  /// @param PosOut          track (global) position at volume exit
  /// @param MomIn           track (global) momentum at volume entry
  /// @param MomOut          track (global) momentum at volume exit
  /// @param time            time elapsed between event start and hit occurence
  /// @param length          track length from origin to hit occurence
  /// @param eLoss           hit energy deposit
  ///
  /// \note Leave this method virtual, assuming that more advanced detectors should
  /// be able to have their specific MC point types inherited from EicMoCaPoint and 
  /// therefore \ref fEicMoCaPointCollection should be filled out by that objects;
  virtual void AddMoCaPoint(Int_t trackID, Int_t primaryMotherID, Int_t secondaryMotherID, Int_t detID, 
			    ULong64_t multiIndex,
			    TVector3 PosIn, TVector3 PosOut,
			    TVector3 MomIn, TVector3 MomOut, 
			    Double_t time, Double_t length,
			    Double_t eLoss, Double_t step) {
    new((*fEicMoCaPointCollection)[fEicMoCaPointCollection->GetEntriesFast()])
      EicMoCaPoint(trackID, primaryMotherID, secondaryMotherID, detID, 
		   multiIndex, 
		   PosIn, PosOut, MomIn, MomOut, time, length, eLoss, step);
  }

  /// Add one more **fake** MC point (hit) to the output TClonesArray
  ///
  /// @param trackID         current track identifier in MC stack
  /// @param primaryMotherId primary GEANT mother track ID; see also \ref GetPrimaryMotherId()
  /// @param PDG             track PDG code
  /// @param detID           current GEANT volume ID
  /// @param volumePath      full ROOT path to the current volume
  /// @param multiIndex      current node unique geometric multi-index; see also \ref GetNodeMultiIndex()
  /// @param PosIn           track (global) position at volume entry
  /// @param MomIn           track (global) momentum at volume entry
  /// @param time            time elapsed between event start and hit occurence
  /// @param length          track length from origin to hit occurence
  ///
  /// In order to speed-up simulation in calorimeter-type detectors stop propagation once
  /// a shower particle reaches certain energy threshold; just record particle parameters
  /// at this point, terminate track and let digitization procedure do the rest using 
  /// \a frozen \a shower database;
  //virtual void AddFakeMoCaPoint(Int_t trackID, Int_t primaryMotherId, Int_t PDG, Int_t detID, 
  //				TString volumePath, ULong64_t multiIndex, 
  //				TVector3 PosIn, TVector3 MomIn,
  //				Double_t time, Double_t length) {
  //new((*fEicFakeMoCaPointCollection)[fEicFakeMoCaPointCollection->GetEntriesFast()]) 
  //  EicFakeMoCaPoint(trackID, primaryMotherId, PDG, detID, volumePath, multiIndex, PosIn, 
  //		       MomIn, time, length);
  //}

  /// Populate fCutOffMap map
  ///
  /// @param PDG             track PDG code
  /// @param cutMin          min kinetic energy
  /// @param cutMax          max kinetic energy
  ///
  /// Table entry will be used this way: once particle-with-abs(PDG)-code kinetic energy falls 
  /// into the [cutMin .. cutMax] range during shower propagation, it will be stopped and fake 
  /// MC point created; see also AddFakeMoCaPoint(); 
  //int SetEnergyCutOff(Int_t PDG, Double_t cutMin, Double_t cutMax); 

  /*! Dump fake MC entries into the output ROOT file; */
  void EndOfEvent();

  /*! Get \ref EicDetectorId detector type; */
  EicDetectorId GetType() { return fDetType;} ;
  
  /*! Derived classes may want to allocate their own frames; */
  virtual FairParSet* EicGeoParAllocator(FairContainer *c);

  /// Allow easy access to various flavors of detector names for non-friend classes
  ///
  EicDetName *GetDname() { return dname; };

  /** 
      Initialize (create) fake MC point database for \a frozen \a shower application
  
      @param outFileName output file name
  */
  int createFakeMoCaDatabase(const char *outFileName);

  // NB: can not print anything at initialization stage in simulation.C since respective 
  // EicGeoParData block is not imported yet -> just book printout;
  void RequestGeometryInfoPrintout(const char *option = 0) {
    mPrintGeometryInfoFlag   = true;
    mPrintGeometryInfoOption = option;
  };

  void RequestAttachedFilePrintout(const char *fileName, const char *option = 0) {
    mAttachedFilePrintoutRequestName = TString(fileName);
    mAttachedFilePrintoutOption      = option;
  };

  EicEnergyMonitor *AddEnergyMonitorVolume(const char *volumeName, Int_t PDG, 
					   char *histogramName, double histogramMin, 
					   double histogramMax, unsigned histogramBinNum = 1000) {
    EicEnergyMonitor *monitor = new EicEnergyMonitor(volumeName, PDG, 
						     histogramName, histogramMin, 
						     histogramMax, histogramBinNum);

    mEnergyMonitorVolumes.push_back(monitor);

    return monitor;
  };

 protected:
  /*! Detector name frame; */
  EicDetName *dname; //!

  /*! Container factory; */ 
  EicContFact *eicContFact; 

  /*! **true** MC point collection (basically points of energy deposit); */
  TClonesArray  *fEicMoCaPointCollection; //!

  /// Array of **fake** MC points, which would require further processing 
  /// at digitization phase (basically points at where ~low energy particle
  /// tracks were forced to stop via gMC->StopTrack() calls);
  //TClonesArray  *fEicFakeMoCaPointCollection; //!

  // Geometry printout request flag and respective option; 
  bool mPrintGeometryInfoFlag; //!
  TString mPrintGeometryInfoOption; //!

  TString mAttachedFilePrintoutRequestName; //!
  TString mAttachedFilePrintoutOption;      //!

  //
  // Working variables; 
  //
  /*! Entry position in global frame; */
  TLorentzVector fPosIn;             //!  
  /*! Entry momentum in global frame; */
  TLorentzVector fMomIn;             //!
  /*! Time elapsed from event start; */
  Double32_t     fTime;              //! 
  /*! Current trajectory length; */
  Double32_t     fLength;            //!  
  /*! (Accumulated) energy deposit; */
  Double32_t     fELoss;             //!   
  /*! (Accumulated) step; */
  Double32_t     fStep;              //! 

  /**
     Set geometry file name
   
     @param fname file name
     @param geoVer whatever FairModule::SetGeometryFileName() requires
  
     Prefer to allow usage of geometry files in current directory; 
     so overload original FairModule method; see also 
     EicDetector::EicDetector(const char *, char *, EicDetectorId, SteppingType, Bool_t)
  */
  virtual void SetGeometryFileName(TString fname, TString geoVer="0");

  void AddKillerVolume(TGeoVolume *volume) {
    if (!mKillerVolumes) mKillerVolumes = new EicNamePatternHub<unsigned>();

    // I guess no double-counting check is really needed here?;
    mKillerVolumes->AddExactMatch(volume->GetName());
  };
 public:
  void AddKillerVolume(const char *vname) {
    if (!mKillerVolumes) mKillerVolumes = new EicNamePatternHub<unsigned>();

    // I guess no double-counting check is really needed here?;
    mKillerVolumes->AddExactMatch(vname);
  };
 protected:
  bool IsKillerVolume(const char *name);

  void CheckEnergyMonitors(const char *name, Int_t trackID, 
			   Int_t PDG, bool isPrimary, bool isEntering, bool isExiting, 
			   double energy);
  
 private: 
  /*! Geometry mapping table; */
  EicGeoParData *gptr;    //!

  /*! Current GEANT volume (?); */
  TGeoVolume *vptr;       //!

  /*! Detector type; */
  EicDetectorId fDetType; //!
  /*! Stepping model; */
  SteppingType fStType;   //!

  /*! ROOT volume path upon entry; */
  TString fPathUponEntry; //!

  /// Sensitive volume names; \note std::vector is perhaps not the best option here; 
  /// leave like this, for historical reasons;
  //std::vector<std::string> fListOfGeantSensitives; //!
  Bool_t mAllVolumesSensitiveFlag;
  EicNamePatternHub<SteppingType> *fListOfGeantSensitives;

  /// Fake MC point database energy range [E_min .. E_max] table for particle "fake" tracing 
  /// in this detector; PDG-code dependent; may be do it volume/material-specific 
  /// later as well?;
  //std::map<Int_t, std::pair<Double_t,Double_t> > *_fCutOffMap; //!
  //
  // May want to clean up here (move to local variables in respective routines)?;
  //
  /*! Fake MC point database file name; */
  //TString fFakeMoCaDatabaseFile;    //!
  /*! Fake MC point database entry; */
  //EicFakeMoCaPointDbEntry *dbEntry; //!
  /*! Fake MC point database ROOT file; */
  //TFile *dbFile;                    //!
  /*! Fake MC point database ROOT tree; */
  //TTree *dbTree;                    //! 

  // Let it be unsigned here; not used anyway;
  EicNamePatternHub<unsigned> *mKillerVolumes; // list of volumes which kill particles upon entry

  // May want to declare some of the volumes as energy measuring ones for particles upon 
  // entrance(exit); may want to extend this formalism to other quantities later; 
  std::vector<EicEnergyMonitor*> mEnergyMonitorVolumes; //!

  ClassDef(EicDetector,49)   
};

/// Convenient default inactive detector of \a qDUMMY type
class EicDummyDetector : public EicDetector {
public:	
  /// Main constructor
  ///
  /// @param Name         detector name
  /// @param geometryName geometry file name
  ///
  /// Other 3 parameters given to EicDetector::EicDetector(const char *, char *, EicDetectorId, 
  /// SteppingType, Bool_t) are {qDUMMY, qOneStepOneHit, kFALSE};
 EicDummyDetector(const char *Name, char *geometryName): 
  EicDetector(Name, geometryName, qDUMMY, qOneStepOneHit, kFALSE) {};

  /// Dummy constructor
  ///
 EicDummyDetector() {};
  
  /// Destructor
  ///
 ~EicDummyDetector() {};

 private:

  ClassDef(EicDummyDetector,7)   
};

#endif
