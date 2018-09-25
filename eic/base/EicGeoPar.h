//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC FairRoot-style (dummy) geometry parameter class; keep at the very minumum 
//  until figure out hwo to use it efficiently;
//

#ifndef _EIC_GEO_PAR_
#define _EIC_GEO_PAR_

#include "FairParGenericSet.h"

class EicGeoPar : public FairParGenericSet {
public:
  //TObjArray            *fGeoSensNodes; // List of FairGeoNodes for sensitive volumes
  //TObjArray            *fGeoPassNodes; // List of FairGeoNodes for passive volumes

  EicGeoPar(const char* name, const char* title, const char* context);
  // Just in order to avoid 'Warning in <TBufferFile::WriteObjectAny>:';
  EicGeoPar() {};
  ~EicGeoPar(void);

  void clear(void);
  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);
  //TObjArray             *GetGeoSensitiveNodes(){return fGeoSensNodes;}
  //TObjArray             *GetGeoPassiveNodes(){return fGeoPassNodes;}
  
  ClassDef(EicGeoPar,5)
};

#endif
