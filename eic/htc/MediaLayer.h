//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Single media layer definition;
//

#include <TGeoMaterial.h>

#ifndef _MEDIA_LAYER_
#define _MEDIA_LAYER_

class MediaLayer {
 public:
 MediaLayer(TGeoMaterial *material, double z0);
  ~MediaLayer() {};

  void SetThickness(double thickness) { mThickness = thickness; };
  double GetThickness()         const { return mThickness; };

  TGeoMaterial *GetMaterial()   const { return mMaterial; };
  double GetZ0()                const { return mZ0; };
  double GetMoliereChc()        const { return mMoliereChc; };

 private:
  TGeoMaterial *mMaterial;

  // 'chc' parameter of Moliere theory;
  double mMoliereChc;

  // Well, layers should in fact be consequitive in Z, so up to numerical 
  // rounding mZ0+mThickness of a given layer should be equal to the mZ0
  // of the next layer; FIXME one day perhaps?;
  double mZ0, mThickness;
};

#endif
