//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  A trivial calorimeter wrapper class
//

#ifndef _EIC_CALORIMETER_
#define _EIC_CALORIMETER_

#include <EicDetector.h>

class EicCalorimeter : public EicDetector {
public:	
  EicCalorimeter() {};

 EicCalorimeter(const char *Name, char *geometryName, EicDetectorId dType, Bool_t Active  = kTRUE):
  // Well, use qOneStepOneHit here; this should cost some CPU time, but is perhaps 
  // more "clean"; actually there should not be much of a problem to merge 
  // steps for a calorimeter device;
  EicDetector(Name, geometryName, dType, qOneStepOneHit, Active) {};
  
  virtual ~EicCalorimeter() {};
  
  ClassDef(EicCalorimeter,1)  
};

#endif
