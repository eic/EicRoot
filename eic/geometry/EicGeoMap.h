//
// AYK (ayk@bnl.gov), 2014/08/20
//
//  EIC basic geometry map class;
//

#include <TString.h>
#include <TObject.h>

#ifndef _EIC_GEO_MAP_
#define _EIC_GEO_MAP_

//
// It looks like ROOT streaming does not allow to save arrays 'arr[DIM]' where
// 'DIM' is a 64-bit variable (in other words '<typedef> *arr; //[DIM]' syntax
// assumes that 'DIM' is at most of UInt_t type); Ok, fine; not really needed
// for EicRoot implementation; so consider a mixed scheme:
//
//   - packed GEANT hierarchy tree node indices are encoded as 32-bit;
//   - internally routines pass 64-bit index where higher 32 bits are 
//     reserved for a map ID (which is an overkill, clear);
//   - all the code however is written in such a way, that changing 
//     UGeantIndex_t to ULong64_t should in principle work (provided the above 
//     mentioned ROOT feature is fixed); NB: of course 32-bit mapping tables 
//     (and MC files produced using them) will not be backwards compatible;
//   - if  UGeantIndex_t ever becomes ULong64_t, one has a freedom to set 
//     _GEANT_INDEX_BIT_NUM_ to any number up to 63 (at least one bit is needed
//     to encode MAP ID); for now _GEANT_INDEX_BIT_NUM_ is set to its max 
//     possible value: 32;
//
//   - logical indices (group of 3D volumes, each of which can have XYZ mapping)
//     have no this limitation and as of 2014/08/20 'mMappingTable' is converted
//     to ULogicalIndex_t* (so 64 bit);
//
//   - it is implicitely assumed, that single dimension indices (say X) can not
//     exceed a 32-bit unsigned value; FIXME: may want to verify this;
//

// So: 32-bit GEANT indices ...
typedef UInt_t    UGeantIndex_t;

// ... and 64-bit logical ones (packed together: group ID and XYZ);
typedef ULong64_t ULogicalIndex_t;

// NB: once again, as long as mentioned above ROOT restriction is valid, 
// _GEANT_INDEX_BIT_NUM_ '32';
#define _GEANT_INDEX_BIT_NUM_    32
#define _GEANT_INDEX_BIT_MASK_   ((ULong64_t(0x1) << _GEANT_INDEX_BIT_NUM_) - 1)
// Bits remaining to encode map ID (so 32+32 split in internal routine arguments);
#define _SERVICE_BIT_NUM_        ((sizeof(ULong64_t) << 3) - _GEANT_INDEX_BIT_NUM_)
// Well, want this mask to be applied to values shifted to bit #0;
#define _SERVICE_BIT_MASK_       ((ULong64_t(0x1) << _SERVICE_BIT_NUM_) - 1)

// Well, voluntary consider to allocate 16 highest bits for logical group encoding
// and remaining 48 bits for XYZ encoding; NB: in a few places in the code assume 
// implicitely, that 1) _LOGICAL_GROUP_BIT_NUM_ and _LOGICAL_XYZ_BIT_NUM_ sum up to 
// the bit count of UGeo_t, 2) group 8 bits are the highest ones (24..31);
#define _LOGICAL_GROUP_BIT_NUM_  16
#define _LOGICAL_XYZ_BIT_NUM_    ((sizeof(ULogicalIndex_t) << 3) - _LOGICAL_GROUP_BIT_NUM_)
#define _LOGICAL_GROUP_NUM_MAX_  (ULogicalIndex_t(0x1) << (_LOGICAL_GROUP_BIT_NUM_))
// Again, want it to be applied to values shifted to bit #0;
#define _LOGICAL_GROUP_MASK_     (_LOGICAL_GROUP_NUM_MAX_-1)

// Encountered in a few places -> create a separate #define;
#define _LOGICAL_INDEX_INVALID_  (~ULogicalIndex_t(0))

template <typename T>
class EicBitMask: public TObject
{
 public:
  EicBitMask() { ResetVars(); };
  EicBitMask(unsigned maxEntryNum) {
    ResetVars();

    // Well, the idea is that level entry may be skipped alltogether and will not 
    // contribute to the mapping table size; so 'mBitNum' and 'mBitMask' will stay 0;
    if (!maxEntryNum) return;
    
    // Looks strange, but works; in principle could save a bit for maxEntryNum=1; 
    // do not mind to overcomplicate things though;
    mBitNum = 1;
    for(T value = maxEntryNum-1; value>>1; value >>= 1)
      mBitNum++;
    
    SetMask();
  };
  ~EicBitMask() {};

  void ResetVars() { mBitNum = mShift = 0; mMask = 0; };

  void SetShift(unsigned shift)   { mShift = shift;};
  void SetBitNum(unsigned bitNum) { mBitNum = bitNum; SetMask(); };

  unsigned GetBitNum()      const { return mBitNum;};
  unsigned GetShift()       const { return mShift; };
  T GetBitMask()            const { return mMask; };

  T GetMaskedBits(T value)  const { return ((value >> mShift) & mMask); };

 private:
  UInt_t mBitNum;   // number of bits in this mask
  UInt_t mShift;    // shift of this level bit mask in the UGeantIndex_t-wide bit index 
  T mMask;          // bit mask itself (bits [0..mBitNum-1], so offset to #0 bit

  void SetMask() { mMask = (T(0x1) << mBitNum) - 1;};

  ClassDef(EicBitMask<T>,1);
};

class GeantVolumeLevel: public TObject
{
  // Yes, no sense to complicate access for the master classes;
  friend class EicGeoMap;
  friend class EicGeoParData;

 public:
 GeantVolumeLevel(): mMaxEntryNum(0), mBitMask(0), mVolumeID(0) {};
  ~GeantVolumeLevel() { if (mBitMask) delete mBitMask; };

  const TString& GetVolumeName()                   const { return mVolumeName; };
  const EicBitMask<UGeantIndex_t> *GetBitMaskPtr() const { return mBitMask; };
  UGeantIndex_t GetMaxEntryNum()                   const { return mMaxEntryNum; };

  UGeantIndex_t GetMaskedBits(UGeantIndex_t value) const { 
    return mBitMask ? mBitMask->GetMaskedBits(value) : 0;
  };

 private:
  UGeantIndex_t mMaxEntryNum;          // max number of identical volume copies on this level
  TString mVolumeName;                 // GEANT volume name

  UInt_t mVolumeID;                    //! GEANT volume ID in the complete simulation geometry tree;

  EicBitMask<UGeantIndex_t> *mBitMask; //-> bit mask parameters associated with this level

  ClassDef(GeantVolumeLevel,1);
};

class EicGeoMap: public TObject
{
  // Yes, simplify access for the master class and also hide few methods 
  // from the outside world;
  friend class EicGeoParData;

 public:
 EicGeoMap(): mMappingTableDim(0), mMappingTable(0), mSensitivityFlag(false),
    mBirkConstant(0.0) {};
  ~EicGeoMap() {};
  
  unsigned GetGeantVolumeLevelNum() const { return mGeantVolumeLevels.size();};
  const GeantVolumeLevel *GetGeantVolumeLevelPtr(unsigned volumeID) const { 
    return volumeID < mGeantVolumeLevels.size() ? mGeantVolumeLevels[volumeID] : 0;
  };

  // Initialize next volume level;
  int AddGeantVolumeLevel(const TString &volumeName, UGeantIndex_t maxEntryNum); 

  bool IsMySignature(const unsigned lvIDs[]) const;

  const TString* GetBaseVolumePath() const { return &mBaseVolumePath; }
  void AssignBaseVolumePath(const char *baseVolumePath) { mBaseVolumePath = baseVolumePath;};

  void SetSensitivityFlag(double birkConstant = 0.0) { 
    mBirkConstant    = birkConstant;
    mSensitivityFlag = true; 
  };
  bool IsSensitive() const { return mSensitivityFlag; };

  // FIXME: consistency check is done in EicDigiHitProducer::ExtraInit() only;
  // may want to implement it earlier (check volume existence with such name); 
  void SetSingleSensorContainerVolume(const char *singleSensorContainerVolumeName) { 
    mSingleSensorContainerVolumeName = TString(singleSensorContainerVolumeName); 
  }; 
  const TString& GetSingleSensorContainerVolumeName() const { 
    return mSingleSensorContainerVolumeName; 
  };  

  const TString* GetInnermostVolumeName()  const { 
    return mGeantVolumeLevels.size() ? &GetGeantVolumeLevelPtr(0)->GetVolumeName() : 0;
  };

  UGeantIndex_t GetMappingTableDim()       const { return mMappingTableDim; };
  const ULogicalIndex_t *GetMappingTable() const { return mMappingTable; };

  double GetBirkConstant()                 const { return mBirkConstant; };

 private:
  // Digitization procedure declares maps as sensitive based on their top-level
  // volume name; may also want to assign Kb constant by hand;
  bool mSensitivityFlag;         //! 
  // Well, it is convenient to have a transient variable right in the mapping table;
  double mBirkConstant;          //!
  
  // TGeoVolume levels (including current - sensitive volume - level!) 
  // which uniquely characterize elementary sensitive node location in the geometry 
  // tree; for instance for endcap backward EMC (crystals) the sequence will be 
  // BemcQuadrant -> BemcTower -> BemcCrystal; 
  std::vector <GeantVolumeLevel*> mGeantVolumeLevels; // volume names and other service info

  // Calculates 'mBitNum' and 'mBitShift' values on all levels; also 'mMappingTableDim';
  int CalculateBitPattern();

  // Overall dimension of the mapping table (2**N, where N is the minimum
  // number of bits needed to pack all level indices);
  UGeantIndex_t mMappingTableDim;     // number of entries in the mapping table
  ULogicalIndex_t *mMappingTable;     //[mMappingTableDim] mapping table array

  // Well, it is assumed of course that this path is unique for all elements
  // of this map; does it make sense?;
  TString mBaseVolumePath;       // exact mother volume path (like "/cave_1/CEMC_0");

  // Well, for the FakeMoCa logic one needs to decide whether 3D point of a fake hit 
  // is inside some cell or not; check is done on all TGeoNode nodes with full path
  // names ending with this volume name (like /cave_1/CEMC_0/cemcSector_11/cemcTower_3);
  // 
  // As of July'2014 this information and respective look-up table are also used
  // for standard digitization (got rid of fGeoH pointer and point->volumePath);
  TString mSingleSensorContainerVolumeName;  // volume which is an "elementary cell" for this map

  // Write access to mapping table for initialization; 
  int SetMappingTableEntry(const unsigned geo[], ULogicalIndex_t logical);

  // Calculate actual multi-dim volume index for the "global" geometry tree; 
  // depends on the sequence in which detectors were put into geometry, etc;
  int CalculateMappingTableSignature();
  
  // Remaps 32-bit GEANT hierarchy tree geographic multi-index into 64-bit 
  // logical map index (say encoded XY-indices for the endcap calorimeter matrix);
  ULogicalIndex_t GeantToLogicalIndex(UGeantIndex_t geant) const {
    return (mMappingTable && geant < mMappingTableDim) ? 
      mMappingTable[geant] : _LOGICAL_INDEX_INVALID_;
  };

  ClassDef(EicGeoMap,16);
};

#endif
