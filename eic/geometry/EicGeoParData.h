//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC geometry mapping classes;
//

#include <stdlib.h>
#include <assert.h>

#include <set>

#include <TString.h>
#include <TObject.h>
#include <TFile.h>
#include <TGeoManager.h>
#include <TTimeStamp.h>
#include <TVector3.h>

class FairGeoMedia;
class FairGeoBuilder;
class FairGeoLoader;
class FairGeoInterface;

class EicDetName;
#include <EicGeoMap.h>
#include <EicNamePatternHub.h>

#ifndef _EIC_GEO_PAR_DATA_
#define _EIC_GEO_PAR_DATA_

// Prefer to decouple from EicGeoMapLevel (even that in principle could 
// allow inheritance);
class LogicalVolumeGroupProjection: public TObject
{
  // No sense to complicate access for the master class;
  friend class EicGeoParData;

 public:
 LogicalVolumeGroupProjection(): mMaxEntryNum(0), mBitMask(0), mCircular(false) {};
 LogicalVolumeGroupProjection(unsigned maxEntryNum): 
  mMaxEntryNum(maxEntryNum), mCircular(false) {
    // NB: in fact maxEntryNum is guaranteed not to be 0 in the calling routine;
    mBitMask = maxEntryNum ? new EicBitMask<ULogicalIndex_t>(maxEntryNum) : 0;
  };
  ~LogicalVolumeGroupProjection() { if (mBitMask) delete mBitMask; };

 private:
  ULogicalIndex_t mMaxEntryNum;          // max number of volume copies on this level

  EicBitMask<ULogicalIndex_t> *mBitMask; //-> bit mask parameters associated with this level

  Bool_t mCircular;                      // "true" if respective dimension is of "barrel" type

  ClassDef(LogicalVolumeGroupProjection,1);
};

// Well, do not need more that XYZ-projections in EicRoot logical volume groups;
// in fact may live with just XY, but who cares;
#define _LOGICAL_VOLUME_GROUP_COORD_NUM_ 3

// No reason to make them bound to whatever class;
TVector3 LocalToMaster    (const TGeoMatrix *mtx, const TVector3& local);
TVector3 LocalToMasterVect(const TGeoMatrix *mtx, const TVector3& local);
TVector3 MasterToLocal    (const TGeoMatrix *mtx, const TVector3& master);
TVector3 MasterToLocalVect(const TGeoMatrix *mtx, const TVector3& master);

class LogicalVolumeLookupTableEntry: public TObject 
{
 public:
 LogicalVolumeLookupTableEntry(): mGeoNode(0), mGeoMtx(0) {};
  ~LogicalVolumeLookupTableEntry() {};
  
  // NB: this whole class is transient stuff -> does not bother ROOT streamer;
  TGeoNode *mGeoNode;     //! pointer to respective TGeoNode
  TGeoHMatrix *mGeoMtx;   //! GEANT transformation between local and master coord. systems

  TString mVolumePath;    //! do not mind to store a complete path as well

  ClassDef(LogicalVolumeLookupTableEntry,1);
};

class LogicalVolumeGroup: public TObject 
{
  // No sense to complicate access for the master class;
  friend class EicGeoParData;

 public:
  LogicalVolumeGroup() {
    mLookup = 0; 
    
    mDim3D = 0;
    for(unsigned iq=0; iq<_LOGICAL_VOLUME_GROUP_COORD_NUM_; iq++)
      mRealDim[iq] = 0;
  };
  ~LogicalVolumeGroup() {};

 private:
  std::vector<LogicalVolumeGroupProjection*> mProjections;

  // These variables are initialized and used upon EicGeoParData import from 
  // a geometry file (so they are not present in the mapping file);
  ULogicalIndex_t mRealDim[_LOGICAL_VOLUME_GROUP_COORD_NUM_]; //! max index size in XYZ-directions
  ULogicalIndex_t mDim3D;                                     //! their product
  LogicalVolumeLookupTableEntry *mLookup;                     //! lookup table

  ClassDef(LogicalVolumeGroup,4);
};

class SourceFile {
 public:
  SourceFile(const char *fileName = 0);
  ~SourceFile() {};

  const TString &GetFileName() const { return mFileName; };
  bool IsOk()                  const { return mOkFlag; };

  void Print();

 private:
  Bool_t mOkFlag;        // flag indicating whether file was imported or not
  TString mFileName;     // file name
  UInt_t mFileSize;      // file size (I guess 32 bits suffices? :-)
  UChar_t *mFileContent; //[mFileSize] file content

  ClassDef(SourceFile,3);
};

//
//  In principle one may want to define certain type of ID for each level 
//  of every map (say declare who is sensitive and who is absorber in 
//  femcFiberCore->femcFiber->femcTower sequence); or perhaps declare whole 
//  maps as either sensitive or absorber ones; in practice this yields more 
//  confusion that help, because 1) one still can not always pack levels
//  in a single map (say fiber core and cladding may be on the same level 
//  in geometry tree), 2) structure becomes much less intuitively clear;
//  so the logic of presently implemented code looks like this:
//
//    - detector may have as many logical maps as needed; actually if a given volume 
//      type (name) is expected to act as GEANT sensitive one, this *requires*
//      creation of a separate map in respective *.C script; even if two maps 
//      overlap in all levels but the top one(s);
//
//    - maps are distinguished by the 0-th - top - level which 1) can be 
//      sensitive volume in GEANT terms, 2) can be either active or dead 
//      (absorber) volume in terms of digitization & reconstruction; 
//
//    - maps may have the same top-level volumes (say 2x2 and 1x2 towers 
//      may have the same crystals as building blocks); 
//
//    - one can de-activate part of the top-level volume types at a time
//      when simulation starts (so that eg. hits are not produced in absorber
//      material; saves greatly CPU time); all top-level volumes are declared 
//      GEANT-sensitive ones per default (check on that!);
//
//    - during digitization one can *select* out of the whole set of GEANT-sensitive 
//      volumes which produced hits a subset of "digitization-sensitive" volume types; 
//      say it should be possible to digitize and find clusters assuming only fiber 
//      core volumes to be sensitive or core+cladding or core+cladding+absorber and just 
//      change light yield, threshold, etc accordingly; energy deposit for *all* types
//      of volumes associated with a given cell will be recorded separately if needed, 
//      and besides this separately for each primary mother particle; only energy deposit
//      in volumes declared as "digitization-sensitive" will be used for photon 
//      generation, clustering, etc; indeed a situation with transparent and scintillating 
//      fibers in one tower is not covered within this logic, since only a single 
//      reconstructed energy per cell is calculated; but at present we have no such detectors 
//      in EIC; logic can be expanded later at a price of yet another complication, so why 
//      bother now?; 
//

class EicGeoParData: public TObject
{
 public:
  EicGeoParData(const char *detName = 0, int version = -1, int subVersion = 0);
  ~EicGeoParData() {};

  void ResetVars();

  enum GeometryType {Undefined, NoStructure, SimpleStructure, FullStructure};

  // 
  // Mapping file creation part and matching access methods
  // 

  void SetGeometryType(GeometryType gType)   { mGeometryType = gType; };
  GeometryType GetGeometryType() const       { return mGeometryType; };

  // Default file name will look like 'vst-test.root' in this case;
  void SetTestGeometryFlag(bool flag = true) { mTestGeometryFlag = flag; };
  bool IsTestGeometry()          const       { return mTestGeometryFlag; };

  int GetVersion()               const       { return mVersion; };
  int GetSubVersion()            const       { return mSubVersion; };

  // Default file name like 'vst-v00.0.root' will be composed using detector name, 
  // version and subversion; one may want to excplicitely overrid this behaviour
  // either proving a fixed name via SetFileName() or a format string like 
  // 'bemc-v02%d-%d-pr.root' where version and subversion will be used; in the latter
  // case user is responsible for sanity control;
  void SetFileName(const char *fileName) { if (fileName) mFileName = fileName; };
  void SetFileNameFormat(const char *fileNameFormat) { 
    if (fileNameFormat) mFileNameFormat = fileNameFormat; 
  };

  void SetComment(const char *comment) { if (comment) mComment = comment; };

  int AttachSourceFile(const char *fileName);
  void PrintAttachedSourceFile(const char *fileName);

  EicGeoMap *CreateNewMap();

 private:
  int SetCircularCore(unsigned group, unsigned what);
 public:
  int SetCircularX(   unsigned group = 0) { return SetCircularCore(group, IDX); };
  int SetCircularY(   unsigned group = 0) { return SetCircularCore(group, IDY); };
  int SetCircularZ(   unsigned group = 0) { return SetCircularCore(group, IDZ); };

 private:
  bool GetCircularCore(unsigned group, unsigned what) const;
 public:
  bool GetCircularX(   unsigned group = 0)            const { return GetCircularCore(group, IDX); };
  bool GetCircularY(   unsigned group = 0)            const { return GetCircularCore(group, IDY); };
  bool GetCircularZ(   unsigned group = 0)            const { return GetCircularCore(group, IDZ); };
  bool GetCircular (   unsigned group, unsigned what) const { return GetCircularCore(group, what);};

  int SetMappingTableEntry(EicGeoMap *map, const unsigned geant[], unsigned group, unsigned logical[]);

  // GEANT hierarchy can be any and may have several levels; logical tables are less 
  // demanding; allow at most 256 groups of XYZ indices and avoid further complications;
  int AddLogicalVolumeGroup(unsigned dimX = 0, unsigned dimY = 0, unsigned dimZ = 0);

  // FIXME: no double-counting check, whetsoever?;
  void AddBlackHoleVolume(const char *vName) { 
    if (vName) mBlackHoleVolumes.insert(TString(vName)); 
  };
  void AddStepEnforcedVolume(const char *vName) { 
    if (vName) mStepEnforcedVolumes.insert(TString(vName));
  };
  void AddStepEnforcedVolumeLookupEntry(int volumeID, double step) { 
    mStepEnforcedVolumesLookup.insert(std::pair<int, double>(volumeID, step));
  };

  // A wrapper to gGeoMan->GetMedium();
  const TGeoMedium *GetMedium(const char *medium);

  void SetTopVolumeTransformation(TGeoMatrix *transformation) {
    mTopVolumeTransformation = transformation;
  };
  const TGeoMatrix* GetTopVolumeTransformation() const { return mTopVolumeTransformation; };

 private:
  TString GetGeometryFileName() const;

 public:
  // NB: yes, these methods can not be protected, otherwise CINT complains like 
  // "Error: ConstructGeometry() declared but no dictionary for the base class";
  // public here and public in derived classes works; what a *hit!;
  virtual void Print(const char *option = 0) const;

  // In fact every derived class is supposed to have its own ConstructGeometry() call
  // unless everything happens in .C script up to the final FinalizeOutput() call;
  virtual int ConstructGeometry() { return 0; };

  // NB: this is not really the top volume in ROOT TGeo sense (see mRootGeoManager->SetTopVolume()
  // call in EicGeoParData::EicGeoParData() -> there is another wrapper volume on top of it); 
  // but it is indeed a top meaning volume of the detector hierarchy;
  TGeoVolume *GetTopVolume()     const { return mTopVolume; };
  TGeoManager *GetRootGeoManager()     { return mRootGeoManager; };

  // Yes, prefer to put all output operations in one user call;
  void FinalizeOutput();

  //
  // simulation/digitization/reconstruction code calls
  //

  enum IDXYZ {IDX=0, IDY, IDZ};

  UInt_t GetMapNum() const { return mGeantVolumeMaps.size(); };
  // (Perhaps write) access to map pointers;
  EicGeoMap *GetMapPtrViaMapID(unsigned mapId) const { 
    return mapId < mGeantVolumeMaps.size() ? mGeantVolumeMaps[mapId] : 0; 
  };
  const EicGeoMap *GetMapPtrViaHitMultiIndex(ULong64_t multi) const { 
    return GetMapPtrViaMapID(unsigned((multi >> _GEANT_INDEX_BIT_NUM_) & _SERVICE_BIT_MASK_)); 
  };

  int CalculateMappingTableSignatures();

  unsigned GetMaxVolumeLevelNum() const { return mMaxVolumeLevelNum;};

  ULogicalIndex_t GeantMultiToLogicalIndex(ULong64_t multi) const;

 private:
  unsigned GetDimCore(unsigned group, unsigned what) const;
 public:
  unsigned GetDimX   (unsigned group = 0)            const { return GetDimCore(group, IDX); };
  unsigned GetDimY   (unsigned group = 0)            const { return GetDimCore(group, IDY); };
  unsigned GetDimZ   (unsigned group = 0)            const { return GetDimCore(group, IDZ); };
  unsigned GetDim    (unsigned group, unsigned what) const { return GetDimCore(group, what);};

  unsigned GetGroup(ULogicalIndex_t logicalID) const { 
    return ((logicalID >> _LOGICAL_XYZ_BIT_NUM_) & _LOGICAL_GROUP_MASK_);
  };

 private:
  unsigned GetLogicalCoordCore(unsigned what, ULogicalIndex_t logicalID) const;
 public:
  unsigned GetX    (                          ULogicalIndex_t logicalID) const { 
    return GetLogicalCoordCore( IDX, logicalID); 
  };
  unsigned GetY    (                          ULogicalIndex_t logicalID) const { 
    return GetLogicalCoordCore( IDY, logicalID); 
  };
  unsigned GetZ    (                          ULogicalIndex_t logicalID) const { 
    return GetLogicalCoordCore( IDZ, logicalID); 
  };
  unsigned GetCoord(unsigned what,            ULogicalIndex_t logicalID) const { 
    return GetLogicalCoordCore(what, logicalID); 
  };

  bool IsBlackHoleVolume(const char *vName) const { 
    // In this case no need to do further steps like char* -> TString conversion;
    if (!mBlackHoleVolumes.size()) return false;

    return (mBlackHoleVolumes.find(TString(vName)) != mBlackHoleVolumes.end());
  };
  const std::set<TString> &GetBlackHoleVolumes() const { return mBlackHoleVolumes; };

  // Not exactly the most efficient call I guess;
  double GetEnforcedStep(int volumeID) { 
    if (mStepEnforcedVolumesLookup.find(volumeID) == mStepEnforcedVolumesLookup.end())
      return 0.0;

    return mStepEnforcedVolumesLookup[volumeID];
  };
  const std::set<TString> &GetStepEnforcedVolumes() const { return mStepEnforcedVolumes; };

  // Need an extra routine to initialize lookup tables since in the constructor call
  // during ROOT streamer import eg mLogicalVolumeGroups.size() is = 0 (in other words
  // nothing is imported yet);
  void InitializeLookupTables();

  LogicalVolumeLookupTableEntry *GetLookupTableNode(ULogicalIndex_t xy)   const;
  LogicalVolumeLookupTableEntry *GetLookupTableNode(const TGeoNode *node) const;

  // Yes, prefer to allow direct access to this (and below)  pointer rather than 
  // creating 4 different access routines (prefix, ..., pattern);
  EicNamePatternHub<Color_t> *GetColorTable() {
    if (!mColorRequests) mColorRequests = new EicNamePatternHub<Color_t>();

    return mColorRequests;
  }; 
  EicNamePatternHub<Char_t> *GetTransparencyTable() {
    if (!mTransparencyRequests) mTransparencyRequests = new EicNamePatternHub<Char_t>();

    return mTransparencyRequests;
  }; 

  // Provide a reasonable default routine; also may want to apply different distance limits 
  // for different dimensions; default distance limits are "natural" ones (3x3 square in 2D 
  // case and 3x3x3 cube in 3D case); maxChebyshevDist=0 explicitely indicates this limit is 
  // of no use per default;
  virtual bool AreNeighbours(ULogicalIndex_t l1, ULogicalIndex_t l2, unsigned maxLinearDist = 1, 
			     unsigned maxChebyshevDist = 0) const;

  const EicDetName *GetDetName() const { return mDetName; };

  void AddWantedParticle(const char *vName, int pdg) {
    mWantedParticles.insert(std::pair<TString, Int_t>(TString(vName), pdg));
  };
  bool IsWantedParticle(const char *vName, int pdg) const {
    return (mWantedParticles.find(std::pair<TString, Int_t>(TString(vName), pdg)) != 
	    mWantedParticles.end());
  };

 protected:
  GeometryType mGeometryType;                // geometry type (no structure / simple / full)
  Bool_t mTestGeometryFlag;                  // "test type" geometry (no "fs/ss/ns" suffix will be used)

 private:
  Int_t mVersion;                            // optional version ID
  Int_t mSubVersion;                         // optional subversion ID

  TString mFileName;                         //! excplicit file name
  TString mFileNameFormat;                   //! explicit file name formatting string

  TString mAuthor;                           // author in a way user@hostname
  TString mComment;                          // optional comment

  std::vector<SourceFile*> mSourceFiles;     // attached source (or whatever other) files

  TTimeStamp mTimeStamp;                     // creation time stamp

  // FIXME: there should be a way to modify this from simulation.C if needed;
  TGeoMatrix *mTopVolumeTransformation;      //

  // Collection of maps for this detector;
  std::vector<EicGeoMap*> mGeantVolumeMaps;  // vector with detector maps

  // Well, can not make these variables persistent, because 'mMaxVolumeLevelNum'
  // is not really defined in C-scripts without an extra call at the end; 
  UInt_t mMaxVolumeLevelNum;                 //! max number of volume levels on any of the maps

  // Well, nothing is wrong to add some extra functionality right in this class, 
  // without creating an extra layer; if multi-index has a meaning of encrypted set of 
  // NXYZ-indices (or perhaps just 1- or 2-dimensional ones), one can consider to implement 
  // neighbour check routines; they should not necessarily be the most efficient 
  // ones, but rather generic enough to be useable by various EicRoot detectors;
  std::vector <LogicalVolumeGroup*> mLogicalVolumeGroups;// table describing GEANT->Logical conversion

  //   Well, eg for ideal calorimeter clustering algorithm I want to collect separately 
  // hits from all "mother" particles; just in order to simplify things before the actual 
  // cluster finding algorithm is operational; the question is how to define who is "mother particle";
  // the convention is: as soon as a particle enters one of such "black hole volumes" its
  // daughters can NOT be mother particles for shower energy deposit hits; a typical scenario is:
  // 
  //  - "FemcTowerShell" is defined as one of such *experiment-wide* (so global) volumes;
  //  - primary electron produces a bremsstrahlung photon in the TPC inner field cage;
  //  - this photon eventually reaches FEMC calorimeter and produces a shower; NB: it enters 
  //    "FemcTowerShell" volume first;
  //
  //     --> hits from all shower particles in FEMC will be assigned bremsstrahlung photon 
  //         as a mother particle even that it is neither a primary particle nor it differs
  //         essentially from all other photons in e/m shower inside the calorimeter;
  //
  //   This way one can get two separate e/m clusters in the calorimeter (one from electron 
  // and the other one from photon); besides this, the logic is arranged such, that list of 
  // "black hole" volumes is NOT detector-specific (so it is a *global* one); eg if one of the 
  // shower electrons escapes FEMC and produces a cluster in FHAC, mother particle for this 
  // FHAC cluster will NOT be escaped electron, but the original bremsstrahlung photon, even 
  // that FHAC may define say "FhacTower" as it's own "black hole volume"; 
  //
  //   The limitation: there should not be any overlap between black hole and sensitive volume names!;
  //
  std::set<TString> mBlackHoleVolumes;    // after entering such a volume particle becomes "secondary mother"

  std::set<std::pair<TString, Int_t> > mWantedParticles; 

  // Well, for whatever reason I can not make use of max step limit as given in media.geo
  // file; this is however essential for TPC tracking; just enforce this setting 
  // in EicDetector::ProcessHits() via explicit call to gMC->SetMaxStep(); yes, it is 
  // inefficient I guess;
  std::set<TString> mStepEnforcedVolumes; // max step will be set explicitely in G4 mode in these volumes
  std::map<int, double> mStepEnforcedVolumesLookup; //! respective lookup table (for efficiency)

 protected:
  // Detector name class;
  EicDetName *mDetName;                    //!

 private:
  // This stuff is specifically put here (see implementation as well) to hide most 
  // of the FairRoot geometry- and media-related calls in scripts like femc.C;
  TGeoManager* mRootGeoManager;            //!
  TGeoVolume *mWrapperVolume;              //!
  TGeoVolume *mTopVolume;                  //!

  FairGeoLoader *mGeoLoad;                 //!
  FairGeoInterface *mGeoFace;              //!

  FairGeoMedia *mFairMedia;                //!
  FairGeoBuilder *mGeobuild;               //!

  // Just need to store the names in order to make sure, that geobuild->createMedium()
  // was performed for all the media requested by GetMedium() calls;
  std::set<const char *> mMediaCache;      //!

  std::map<const TGeoNode*, LogicalVolumeLookupTableEntry*> mGeantToLogicalLookupTable; //!

  // I think there is no need to propagate this information further?;
  EicNamePatternHub<Color_t> *mColorRequests; //!
  EicNamePatternHub<Char_t>  *mTransparencyRequests; //!

  ClassDef(EicGeoParData,48);
};

#endif
