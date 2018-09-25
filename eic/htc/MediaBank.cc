//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Media distribution along Kalman filter node arrangement line;
//

#include <assert.h>

#include <MediaBank.h>

// ---------------------------------------------------------------------------------------

int MediaBank::StartNextLayer(TGeoMaterial *material, TVector3 pt) {
  // Figure out Z coordinate, taking into account coordinate system offset;
  double z = (pt - mAxisLine.x).Dot(mAxisLine.nx);

  // Allow 'z' to be negative; FIXME: do not check, that thickness of the previous 
  // layer matched this pt[]?; yet check monotonous grow (FIXME: but ignore thickness);
  {
    MediaLayer *layer = GetCurrentMediaLayer();

    if (layer && z < layer->GetZ0()) {
      printf("MediaBank::StartNextLayer(): Z(now) is %7.2f, Z(last) was %7.2f\n", 
	     z, layer->GetZ0());
      return -1;
    } //if
  }

  mMediaLayers.push_back(MediaLayer(material, pt[2]));

  return 0;
} // MediaBank::StartNextLayer()

// ---------------------------------------------------------------------------------------

void MediaBank::Print()
{
  printf("--> %d layer(s)\n", GetMediaLayerCount());

  for(int iq=0; iq<GetMediaLayerCount(); iq++) {
    const MediaLayer *mlayer = GetMediaLayer(iq);
    TGeoMaterial *material = mlayer->GetMaterial();

    printf("material --> z0=%8.2fcm (%7.2fcm thick): %s\n", 
	   mlayer->GetZ0(), mlayer->GetThickness(), material->GetName());
  } //for iq
} // MediaBank::Print()

// ---------------------------------------------------------------------------------------
