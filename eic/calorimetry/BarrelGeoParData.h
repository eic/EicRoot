//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Barrel extension of a generic calorimeter mapping class;
//

#include <CalorimeterGeoParData.h>

#ifndef _BARREL_GEO_PAR_DATA_
#define _BARREL_GEO_PAR_DATA_

class BarrelGeoParData: public CalorimeterGeoParData
{
 private:
  void ResetVars() { SetCircularX(); };

 public:
 BarrelGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  CalorimeterGeoParData(detName, version, subVersion) { ResetVars(); };
  ~BarrelGeoParData() {};
  
  ClassDef(BarrelGeoParData,2);
};

#endif
