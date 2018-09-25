//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC digitized hit producer class; assume some basic functionality 
//  can be established, although tracking and calorimeter detectors
//  will have their own (inherited) classes for this purpose;
//

#include <TVector3.h>
class TClonesArray;
class TObjectArray;

#include <FairTask.h>
#include <PndGeoHandling.h>

#include <EicDetName.h>
#include <EicNamePatternHub.h>
#include <EicMoCaPoint.h>
#include <EicGeoParData.h>

#ifndef _EIC_DIGI_HIT_PRODUCER_
#define _EIC_DIGI_HIT_PRODUCER_

/// Base digitization class 
class EicDigiHitProducer : public FairTask
{
 public:
  /// Default constructor
  ///
  EicDigiHitProducer() { ResetVars(); }; 
  /// Main constructor
  ///
  /// @param name detector name 
  EicDigiHitProducer(const char *name);

  ~EicDigiHitProducer() {};

  void ResetVars() {
    mDigiHitArray = 0; 
    mGptr = 0; 
    mMoCaPointArray = 0; mDetName = 0;
  };

  /// \brief digitization model identifiers
  ///
  /// May either want to smear MC coordinates according to gaussian 
  /// (say GEM-type detector) or to quantize assuming silicon-type
  /// detector with digital readout; 3-d option is to call external
  /// digitizer (should be provided by user);  
  enum SmearingModel {NoAction, Smear, Quantize, Calculate};

  /// Initialization part, common for all detectors
  ///
  /// Opens read access to MoCa point TClonesArray and write access to 
  /// digi hit TClonesArray;
  InitStatus Init();
  /// Optional (?) detector-specific initialization part
  ///
  virtual InitStatus ExtraInit() {return kSUCCESS; };

  /// Detector-specific event-level initialization
  ///
  virtual int PreExec() { return 0; };
  /// Wrapper routine converting MoCa hits into digis
  ///
  /// \note Certain versatility is 
  /// provided by event-level PreExec() and PostExec() calls which 
  /// embed a loop over MC points with a detector-specific HandleHit() call;
  /// if that is not enough, go ahead and rewrite this (virtual) call from scratch;
  virtual void Exec(Option_t* opt);
  /// Detector-specific event-level post-execution call
  ///
  virtual int PostExec() { return 0; };

  /// Core routine converting a single MoCa hit into digi
  ///
  /// Should be provided for all detectors separately (pure virtual method); in fact 
  /// there is no one-to-one relationship between MC points and digis for some 
  /// detectors (say TPC);
  virtual int HandleHit(const EicMoCaPoint *point) = 0;
 
  /// Set output digi TClonesArray persistency
  ///
  /// @param persistence kTRUE (write digi array out) or kFALSE (do not)
  void SetPersistence(Bool_t persistence) { mPersistence = persistence; }

  // Birk's constant should be given in [cm]/[GeV] units; 
  void DeclareDigiSensitiveVolume(const char *name, Double_t Kb = 0.0) /*const*/ { 
    /* mDigi->*/mSensitiveVolumes.AddExactMatch(name, Kb);
  };
  void DeclareDigiSensitiveVolumePrefix(const char *name, Double_t Kb = 0.0) /*const*/ { 
    /*mDigi->*/mSensitiveVolumes.AddPrefixMatch(name, Kb);
  };

 protected:
  /*! Detector name */
  EicDetName *mDetName;          //!

  /*! Output array of detector-specific digi hits */
  TClonesArray* mDigiHitArray;   //! 

  // Put EicGeoMap->TGeoNode mapping table creator in a separate class;
  EicGeoParData *mGptr;          //!

  EicNamePatternHub<double> mSensitiveVolumes;// sensitive volumes with their respective Birks' constants

  /*! Object persistency flag */
  Bool_t  mPersistence;          //!

 private: 
  void SetParContainers();

  /*! Input array of detector-specific EicMoCaPoint's */
  TClonesArray* mMoCaPointArray; //!

  /*! Event counter */
  Int_t mEventCounter;           //!
  
  EicDigiHitProducer(const  EicDigiHitProducer& L);
  EicDigiHitProducer& operator= (const  EicDigiHitProducer&) {return *this;}

  ClassDef(EicDigiHitProducer,8);
};

#endif
