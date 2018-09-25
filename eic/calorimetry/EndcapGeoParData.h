//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Endcap extension of a generic calorimeter mapping class;
//

#include <CalorimeterGeoParData.h>

#ifndef _ENDCAP_GEO_PAR_DATA_
#define _ENDCAP_GEO_PAR_DATA_

class EndcapGeoParData: public CalorimeterGeoParData
{
 private:
  void ResetVars() {
    mEndcapMinR = mEndcapMaxTheta = mSafetyVolume = mInterQuadrantGap = 0.0;
  };

 public:
 EndcapGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  CalorimeterGeoParData(detName, version, subVersion) { ResetVars();};
  ~EndcapGeoParData() {};
  
  Double_t mInterQuadrantGap;   // allow few hundred um gap between quadrants;

  // Yes, it seems it is natural to define the global endcap parameters like this;
  Double_t mEndcapMinR;         // min distance of endcap volume to beam line
  Double_t mEndcapMaxTheta;     // max theta to be covered from the IP
  Double_t mSafetyVolume;       // min estimated cell distance to quadrant volume edge

  ClassDef(EndcapGeoParData,7);
};

#endif
