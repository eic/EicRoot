//
// AYK (ayk@bnl.gov), 2015/08/06
//
//  EIC field-map-related fake detector class;
//

#include <EicFieldGradDetector.h>


#include <EicMagneticFieldGrad.h>

// ---------------------------------------------------------------------------------------

EicFieldGradDetector::EicFieldGradDetector(EicMagneticFieldGrad *fmap, Bool_t Active):
  EicDetector(fmap ? fmap->GetDetectorName().Data() : 0, (char*)"no-geometry.cfg", 
	      qDUMMY, qOneStepOneHit, Active), 
  mMap(fmap) 
{

} // EicFieldGradDetector::EicFieldGradDetector()

// ---------------------------------------------------------------------------------------

void EicFieldGradDetector::ConstructGeometry()
{
  if (mMap && mMap->ConstructGeometry())
    fLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to construct geometry for '%s' field map!  \033[0m", 
		   mMap->GetFileName().Data());

  // Yes, just add sensitivity flag by hand if needed;
  if (mMap && IsActive()) {
    TGeoVolume *yoke = mMap->GetYokeVolume();

    // Yes, for now (acceptance studies) just want to kill the particle 
    // right away when it enters this volume;
    if (yoke) AddKillerVolume(yoke);
  } //if
} // EicFieldGradDetector::ConstructGeometry()

// ---------------------------------------------------------------------------------------

ClassImp(EicFieldGradDetector)
