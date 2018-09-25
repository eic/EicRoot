//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  BEMC (Backward EMC) specific data definitions
//

#include <EndcapGeoParData.h>

#ifndef _BEMC_GEO_PAR_DATA_
#define _BEMC_GEO_PAR_DATA_

class BemcGeoParData: public EndcapGeoParData
{
 private:
  void ResetVars() {
    mCellRearSize = mSubunitFocalDist = mInterSubunitGap = 0.0;
  };

 public:
 BemcGeoParData(int version = -1, int subVersion = 0): 
  EndcapGeoParData("BEMC", version, subVersion) { ResetVars(); };
  ~BemcGeoParData() {};

  //
  // -> Do not mind to have these variables public?; yes, they are pure 
  //    data containers, so set()/get() methods really make no sense;
  //

  // Crystals will follow a mixture of CMS &  PANDA designs (right angle on two 
  // sides, tapered on two other sides, grouped in "subunits" in 2x2 configuration);
  Double_t mCellRearSize;       // crystal rear side (square) dimension

  // Well, may want to couple this to the crystal tapering?;  
  Double_t mSubunitFocalDist;   // focal parameter used to arrange crystal subunits
  Double_t mInterSubunitGap;    // min distance between 2x2 subunit

  ClassDef(BemcGeoParData,13);
};

#endif
