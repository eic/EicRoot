#ifndef FAIRGEOTUBE_H
#define FAIRGEOTUBE_H

#include "FairGeoBasicShape.h"

class FairGeoTransform;
class FairGeoVolume;

class FairGeoTube : public FairGeoBasicShape
{
  public:
    FairGeoTube();
    ~FairGeoTube();
    TArrayD* calcVoluParam(FairGeoVolume*);
    void calcVoluPosition(FairGeoVolume*,
                          const FairGeoTransform&,const FairGeoTransform&);
    Int_t readPoints(std::fstream*,FairGeoVolume*);
    Bool_t writePoints(std::fstream*,FairGeoVolume*);
    void printPoints(FairGeoVolume* volu);
    ClassDef(FairGeoTube,0) // class for geometry shape TUBE
};

#endif  /* !FAIRGEOTUBE_H */
