//*-- AUTHOR : Denis Bertini
//*-- Created : 21/06/2005
#include "PndGeoPassivePar.h"
#include "FairParamList.h"
#include <iostream>
#include <iomanip>
#include "TObjArray.h"
using namespace std;
ClassImp(PndGeoPassivePar)

PndGeoPassivePar::PndGeoPassivePar(const char* name,const char* title,const char* context)
           : FairParGenericSet(name,title,context), fGeoSensNodes(), fGeoPassNodes() {

               fGeoSensNodes = new TObjArray();
               fGeoPassNodes = new TObjArray();
}

PndGeoPassivePar::~PndGeoPassivePar(void) {
}

void PndGeoPassivePar::clear(void) {
    if(fGeoSensNodes) delete fGeoSensNodes;
    if(fGeoPassNodes) delete fGeoPassNodes;
}

void PndGeoPassivePar::putParams(FairParamList* l) {
  if (!l) return;
   l->addObject("FairGeoNodes Sensitive List", fGeoSensNodes);
   l->addObject("FairGeoNodes Passive List", fGeoPassNodes);
}

Bool_t PndGeoPassivePar::getParams(FairParamList* l) {
    if (!l) return kFALSE;
    if (!l->fillObject("FairGeoNodes Sensitive List", fGeoSensNodes)) return kFALSE;
    if (!l->fillObject("FairGeoNodes Passive List", fGeoPassNodes)) return kFALSE;

  return kTRUE;
}
