#ifndef PNDGEOPASSIVEPAR_H
#define PNDGEOPASSIVEPAR_H

#include "FairParGenericSet.h"
#include "TH1F.h"

class PndGeoPassivePar : public FairParGenericSet {
public:
  TObjArray            *fGeoSensNodes; // List of FairGeoNodes for sensitive volumes
  TObjArray            *fGeoPassNodes; // List of FairGeoNodes for sensitive volumes

  PndGeoPassivePar(const char* name="PndGeoPassivePar",
             const char* title="Passive Geometry Parameters",
             const char* context="TestDefaultContext");
  ~PndGeoPassivePar(void);
  void clear(void);
  void putParams(FairParamList*);
  Bool_t getParams(FairParamList*);
  TObjArray             *GetGeoSensitiveNodes(){return fGeoSensNodes;}
  TObjArray             *GetGeoPassiveNodes(){return fGeoPassNodes;}

private:
  PndGeoPassivePar(const  PndGeoPassivePar& L){;}
  PndGeoPassivePar& operator= (const  PndGeoPassivePar&) {return *this;}

   ClassDef(PndGeoPassivePar,1)
};

#endif /* !PNDGEOPASSIVEPAR_H */
