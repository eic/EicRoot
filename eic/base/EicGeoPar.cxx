//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC FairRoot-style (dummy) geometry parameter class; 
//

#include "TObjArray.h"
#include "EicGeoPar.h"
#include "FairParamList.h"

// ---------------------------------------------------------------------------------------

EicGeoPar::EicGeoPar(const char* name,const char* title,const char* context)
           : FairParGenericSet(name,title,context)
{
  //fGeoSensNodes = new TObjArray();
  //fGeoPassNodes = new TObjArray();
} // EicGeoPar::EicGeoPar()

// ---------------------------------------------------------------------------------------

EicGeoPar::~EicGeoPar(void) 
{
  //if(fGeoSensNodes) delete fGeoSensNodes;
  //if(fGeoPassNodes) delete fGeoPassNodes;
} // EicGeoPar::~EicGeoPar()

// ---------------------------------------------------------------------------------------

void EicGeoPar::clear(void)
{
  //if(fGeoSensNodes) 
  //delete fGeoSensNodes;
  //if(fGeoPassNodes) 
  //delete fGeoPassNodes;

  //fGeoSensNodes = fGeoPassNodes = 0x0; //AZ
} // EicGeoPar::clear()

// ---------------------------------------------------------------------------------------

void EicGeoPar::putParams(FairParamList* l)
{
  if (!l) return;  

  //l->addObject("FairGeoNodes Sensitive List", fGeoSensNodes);
  //l->addObject("FairGeoNodes Passive List", fGeoPassNodes);
} // EicGeoPar::putParams()

// ---------------------------------------------------------------------------------------

Bool_t EicGeoPar::getParams(FairParamList* l)
{
  if (!l) return kFALSE;

#if _LATER_
  if (!l->fillObject("FairGeoNodes Sensitive List", fGeoSensNodes))
    return kFALSE;

  if (!l->fillObject("FairGeoNodes Passive List", fGeoPassNodes))
    return kFALSE;
#endif

  return kTRUE;
} // EicGeoPar::getParams()

// ---------------------------------------------------------------------------------------

ClassImp(EicGeoPar)
