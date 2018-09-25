//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Single media slice definition;
//

// NB: do not remove, contains _USE_GEANT3_MOLIERE_CHC_ definition!;
//#include <htclib.h>

#ifndef _MEDIA_SLICE_
#define _MEDIA_SLICE_

#include <MediaLayer.h>

class MediaSlice {
public:
 MediaSlice(const MediaLayer *mlayer, double z0, double thickness): mMediaLayer(mlayer), mZ0(z0), 
    mThickness(thickness), Cms(0.0) {
      //#ifndef _USE_GEANT3_MOLIERE_CHC_
    mReducedRadiationLength = thickness/mlayer->GetMaterial()->GetRadLen();
    //#endif

    memset(_RCxx, 0x00, sizeof(_RCxx));
    memset(_RCyy, 0x00, sizeof(_RCyy));
    memset(_RCxy, 0x00, sizeof(_RCxy));
  };
  ~MediaSlice() {};

  const MediaLayer *GetMediaLayer()  const { return mMediaLayer; };
  double GetZ0()                     const { return mZ0; };
  double GetThickness()              const { return mThickness; };
  //#ifndef _USE_GEANT3_MOLIERE_CHC_
  double GetReducedRadiationLength() const { return mReducedRadiationLength; };
  //#endif

  // This is momentum- and slope-independent part of the well-known formula 
  // for the multiple scattering angle variance;                             
  double Cms;
  // "Raw" part of the process noise covariance matrix for this slice; 
  double _RCxx[2][4][4], _RCyy[2][4][4], _RCxy[2][4][4];

 private:
  const MediaLayer *mMediaLayer;
  
  double mZ0, mThickness;
  //#ifndef _USE_GEANT3_MOLIERE_CHC_
  double mReducedRadiationLength;
  //#endif
};

#endif
