//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  CEMC (Central rapidity EMC) specific data definitions
//

#include <BarrelGeoParData.h>
#include <FiberParData.h>

#ifndef _CEMC_GEO_PAR_DATA_
#define _CEMC_GEO_PAR_DATA_

class CemcGeoParData: public BarrelGeoParData
{
 public:
 CemcGeoParData(int version = -1, int subVersion = 0): 
  BarrelGeoParData("CEMC", version, subVersion), mFiber(0) {};
  ~CemcGeoParData() { if (mFiber) delete mFiber; };

  FiberParData *mFiber; // fiber geometry; if NULL, active material is smeared

  ClassDef(CemcGeoParData,9);
};

#endif
