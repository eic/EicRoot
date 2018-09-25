//
// AYK (ayk@bnl.gov), 2015/01/28
//
//  EIC MuMegas detector definition wrapper 
//

#ifndef _EIC_MUMEGAS_
#define _EIC_MUMEGAS_

#include <EicDetector.h>

class EicMuMegas : public EicDetector {
public:	
  EicMuMegas() {};

 EicMuMegas(const char *Name, char *geometryName, EicDetectorId dType, Bool_t Active  = kTRUE):
  // Yes, want to merge all steps in one hit, otherwise 
  // tracking code will get much more hits and screw up all resolutions;
  EicDetector(Name, geometryName, dType, qMergeStepsInOneHit, Active) {};
  
  virtual ~EicMuMegas() {};
  
  ClassDef(EicMuMegas,1)  
};

#endif
