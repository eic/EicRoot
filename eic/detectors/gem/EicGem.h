//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  EIC GEM detector definition wrapper 
//

#ifndef _EIC_GEM_
#define _EIC_GEM_

#include <EicDetector.h>

class EicGem : public EicDetector {
public:	
  EicGem() {};

 EicGem(const char *Name, const char *geometryName, EicDetectorId dType, Bool_t Active  = kTRUE):
  // Yes, want to merge all steps in one hit, otherwise 
  // tracking code will get much more hits and screw up all resolutions;
  //EicDetector(Name, geometryName, kFALSE, dType, Active) {};
  EicDetector(Name, geometryName, dType, qMergeStepsInOneHit, Active) {};
  
  virtual ~EicGem() {};
  
  ClassDef(EicGem,1)  
};

#endif
