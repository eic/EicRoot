//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  A trivial MAPS (silicon tracker) wrapper class
//

#ifndef _EIC_MAPS_
#define _EIC_MAPS_

#include <EicDetector.h>

class EicMaps : public EicDetector {
public:	
  EicMaps() {};

 EicMaps(const char *Name, const char *geometryName, EicDetectorId dType, Bool_t Active  = kTRUE):
  // Yes, want to merge all steps in one hit, otherwise 
  // tracking code will get much more hits and screw up all resolutions;
  //EicDetector(Name, geometryName, kFALSE, dType, Active) {};
  EicDetector(Name, geometryName, dType, qMergeStepsInOneHit, Active) {};
  
  virtual ~EicMaps() {};
  
  ClassDef(EicMaps,1)  
};

#endif
