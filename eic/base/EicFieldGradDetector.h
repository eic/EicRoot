//
// 
//
//  EIC field-map-related fake detector class;
//

#ifndef _EIC_FIELD_GRAD_DETECTOR_
#define _EIC_FIELD_GRAD_DETECTOR_

#include <EicDetector.h>

class EicMagneticFieldGrad;

class EicFieldGradDetector: public EicDetector {
 public:	
  EicFieldGradDetector(EicMagneticFieldGrad *fmap = 0, Bool_t Active = kFALSE);
  ~EicFieldGradDetector() {};
  
  void ConstructGeometry();
  
 private:
  EicMagneticFieldGrad *mMap; //! magnetic field map
  
  ClassDef(EicFieldGradDetector,1);
};

#endif
