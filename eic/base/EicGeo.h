//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  EIC FairRoot-style (dummy) geometry class; keep at the very minumum 
//  until figure out hwo to use it efficiently;
//

#ifndef _EIC_GEO_
#define _EIC_GEO_

#include "FairGeoSet.h"

//
// Well, for now existence of this class is justified by a fact, that 
// FairGeoSet::FairGeoSet() constructor is protected, so can not be 
// called from EicDet::ConstructGeometry() directly; let it be, perhaps 
// fill out with some sense later;
//

class EicGeo : public FairGeoSet {
public:
  EicGeo() { maxSectors = maxModules = 0;};

  EicGeo(const char *_fName) {
    fName = strdup(_fName);
    
    maxSectors = 0;
    // This should not be 0, otherwise something fails to work in PandaRoot;
    maxModules = 1;
  };

  ~EicGeo() {};

  ClassDef(EicGeo,6)
};

#endif
