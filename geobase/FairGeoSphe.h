#ifndef FAIRGEOSPHE_H
#define FAIRGEOSPHE_H

#include "FairGeoBasicShape.h"

class FairGeoTransform;
class FairGeoVolume;

/**
 * class for geometry shape SPHE
 * @author Ilse koenig
 */
class FairGeoSphe : public FairGeoBasicShape
{
  public:
    FairGeoSphe();
    ~FairGeoSphe();
    TArrayD* calcVoluParam(FairGeoVolume*);
    void calcVoluPosition(FairGeoVolume*,
                          const FairGeoTransform&,const FairGeoTransform&);
    Int_t readPoints(std::fstream*,FairGeoVolume*);
    Bool_t writePoints(std::fstream*,FairGeoVolume*);
    void printPoints(FairGeoVolume* volu);
    ClassDef(FairGeoSphe,0) //
};

#endif  /* !FAIRGEOSPHE_H */
