//
// AYK (ayk@bnl.gov), 2015/08/06
//
//  EIC field-map-related fake detector class;
//

#ifndef _EIC_FIELD_MAP_DETECTOR_
#define _EIC_FIELD_MAP_DETECTOR_

#include <EicDetector.h>

class EicMagneticFieldMap;

class EicFieldMapDetector: public EicDetector {
 public:	
  EicFieldMapDetector(EicMagneticFieldMap *fmap = 0, Bool_t Active = kFALSE);
  ~EicFieldMapDetector() {};
  
  void ConstructGeometry();
  
 private:
  EicMagneticFieldMap *mMap; //! magnetic field map
  
  ClassDef(EicFieldMapDetector,2);
};

#endif
