//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Single media layer definition;
//

#include <htclib.h>

#include <MediaLayer.h>

// ---------------------------------------------------------------------------------------

MediaLayer::MediaLayer(TGeoMaterial *material, double z0): mMaterial(material), mZ0(z0) 
{
  mThickness = 0.0;

  unsigned nmat = material->GetNelements();
  float omc, chc, af[nmat], zf[nmat], wf[nmat];

  for(unsigned el=0; el<nmat; el++) {
    double ad, zd, wd;
    
    material->GetElementProp(ad, zd, wd, el);
    af[el] = ad; zf[el] = zd; wf[el] = wd;
  } //for el

  G3MOLI(af, zf, wf, nmat, material->GetDensity(), omc, chc);

  // Prefer to store double value;
  mMoliereChc = chc;
} // MediaLayer::MediaLayer()

// ---------------------------------------------------------------------------------------
