//
// AYK (ayk@bnl.gov), 2014/08/20
//
//  EIC basic geometry map class;
//

#include <TGeoManager.h>

#include <EicGeoMap.h>

// =======================================================================================

int EicGeoMap::CalculateBitPattern()
{
  unsigned accu = 0;

  for(int lv=0; lv<mGeantVolumeLevels.size(); lv++)
  {
    GeantVolumeLevel *lptr = mGeantVolumeLevels[lv];

    if (!lptr->mMaxEntryNum) continue;

    EicBitMask<UGeantIndex_t> *mask = lptr->mBitMask = new EicBitMask<UGeantIndex_t>(lptr->mMaxEntryNum);

    // Setting shift on "empty" (ignored) levels will not hurt;
    mask->SetShift(accu);
    accu += mask->GetBitNum();
  } //for lv

  if (accu > _GEANT_INDEX_BIT_NUM_) {
    printf("-E- EicGeoMap::CalculateBitPattern(): bit pattern can not handle more "
	   "than %d bits, sorry!\n", _GEANT_INDEX_BIT_NUM_);
    return -1;
  } //if

  if (accu) mMappingTableDim = UGeantIndex_t(0x1) << accu;

  return 0;
} // EicGeoMap::CalculateBitPattern()

// ---------------------------------------------------------------------------------------

int EicGeoMap::AddGeantVolumeLevel(const TString &volumeName, UGeantIndex_t maxEntryNum)
{
  // Check for volume name double counting;
  for(int lv=0; lv<mGeantVolumeLevels.size(); lv++)
  {
    GeantVolumeLevel *lptr = mGeantVolumeLevels[lv];

    if (volumeName == lptr->mVolumeName)
    {
      printf("-E- EicGeoMap::addVolumeLevel(): '%s' accounted twice!\n", volumeName.Data());
      return -1;
    } //if
  } //for lv

  GeantVolumeLevel *lptr = new GeantVolumeLevel();
  mGeantVolumeLevels.push_back(lptr);
  lptr->mVolumeName  = volumeName;
  lptr->mMaxEntryNum = maxEntryNum;

  return 0;
} // EicGeoMap::AddGeantVolumeLevel()

// ---------------------------------------------------------------------------------------

int EicGeoMap::SetMappingTableEntry(const unsigned id[], ULogicalIndex_t value)
{
  UGeantIndex_t idL = 0x0;

  // A lousy protection; yet want to see this happen before taking more clean measures;
  if (value == _LOGICAL_INDEX_INVALID_) {
    printf("-E- EicGeoMap::SetMappingTableEntry(): attempt to set invalid mapping entry!\n");
    return -1;
  } //if

  if (!mMappingTable)
  {
    // See error message in CalculateBitPattern();
    if (CalculateBitPattern()) return -1;

    // Allocate mapping table and initialize to "illegal" values;
    mMappingTable = new ULogicalIndex_t[mMappingTableDim];

    for(UGeantIndex_t iq=0; iq<mMappingTableDim; iq++)
      mMappingTable[iq] = _LOGICAL_INDEX_INVALID_;//~ULogicalIndex_t(0);
  } // if

  // Calculate multi-index; 
  for(unsigned lv=0; lv<mGeantVolumeLevels.size(); lv++)
  {
    GeantVolumeLevel *lptr = mGeantVolumeLevels[lv];

    // Skip dummy levels entirely;
    if (!lptr->mMaxEntryNum) continue;

    if (id[lv] >= lptr->mMaxEntryNum) {
      printf("-E- EicGeoMap::SetMappingTableEntry(): index '%d' exceeds limit (%d) for '%s'\n", 
	     id[lv], lptr->mMaxEntryNum-1, lptr->GetVolumeName().Data());
      return -1;
    } //if
    
    idL |= id[lv] << lptr->mBitMask->GetShift();
  } //for lv

  // Since individual values were checked, can not be out of range here;
  // yet check for double-assignment (no harm, but better should not happen);
  if (mMappingTable[idL] != _LOGICAL_INDEX_INVALID_) { //~ULogicalIndex_t(0)) {
    printf("-E- EicGeoMap::SetMappingTableEntry(): attempt to set index '%u' twice!\n", idL);
    return -1;
  } //if

  mMappingTable[idL] = value;

  return 0;
} // EicGeoMap::SetMappingTableEntry()

// ---------------------------------------------------------------------------------------

int EicGeoMap::CalculateMappingTableSignature()
{
  for(unsigned lv=0; lv<mGeantVolumeLevels.size(); lv++)
  {
    GeantVolumeLevel *lptr = mGeantVolumeLevels[lv];
    TGeoVolume *volume   =  gGeoManager->GetVolume(lptr->mVolumeName);
      
    if (!volume)
    {
      printf("-E- EicGeoMap::CalculateMappingTableSignature(): unknown volume name (%s0!\n", 
	     lptr->mVolumeName.Data());
      return -1;
    } //if

    lptr->mVolumeID = volume->GetNumber();
  } //for lv

  return 0;
} // EicGeoMap::CalculateMappingTableSignature()

// ---------------------------------------------------------------------------------------

bool EicGeoMap::IsMySignature(const unsigned lvIds[]) const 
{
  for(unsigned lv=0; lv<mGeantVolumeLevels.size(); lv++)
  {
    GeantVolumeLevel *lptr = mGeantVolumeLevels[lv];

    if (lvIds[lv] != lptr->mVolumeID) return false;
  } //for lv

  return true;
} // EicGeoMap::IsMySignature()

// =======================================================================================

ClassImp(EicBitMask<ULogicalIndex_t>)
ClassImp(EicBitMask<UGeantIndex_t>)
ClassImp(GeantVolumeLevel)
ClassImp(EicGeoMap)

