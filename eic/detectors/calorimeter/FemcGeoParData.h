//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  FEMC (Forward EMC) specific data definitions
//

#include <EndcapGeoParData.h>
#include <FiberParData.h>

#ifndef _FEMC_GEO_PAR_DATA_
#define _FEMC_GEO_PAR_DATA_

class FemcGeoParData: public EndcapGeoParData
{
 public:
 FemcGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  EndcapGeoParData(detName, version, subVersion) { ResetVars(); };
  ~FemcGeoParData() { if (mFiber) delete mFiber; if (mTower) delete mTower; };

  void ResetVars() {
    mFiber = 0;
    mTower = 0; 
    mRotationY = mAluBoxThickness = mEntranceWndThickness;
  };

  FiberParData *mFiber;           // fiber geometry; if NULL, active material is smeared

  TowerParData *mTower;           // generic tower geometry

  Double_t mAluBoxThickness;      // T1018: FEMC aluminum box thickness
  Double_t mEntranceWndThickness; // T1018: FEMC aluminum box entrance window thickness

  Double_t mRotationY;            // overall Y-rotation (2 degree, relevant for T1018 test run only)
    
  ClassDef(FemcGeoParData,10);
};

#endif
