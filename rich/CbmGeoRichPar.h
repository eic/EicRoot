/**
* \file CbmGeoRichPar.h
*
* \brief
*
* \author Denis Bertini
* \date 2005
**/

#ifndef CBM_GEO_RICH_PAR
#define CBM_GEO_RICH_PAR

#include "FairParGenericSet.h"

class TObjArray;

/**
* \class CbmGeoRichPar
*
* \brief
*
* \author Denis Bertini
* \date 2005
**/
class CbmGeoRichPar : public FairParGenericSet
{
public:

   TObjArray* fGeoSensNodes; // List of FairGeoNodes for sensitive  volumes
   TObjArray* fGeoPassNodes; // List of FairGeoNodes for sensitive  volumes

   CbmGeoRichPar(
         const char* name="CbmGeoRichPar",
         const char* title="Rich Geometry Parameters",
         const char* context="TestDefaultContext");

   ~CbmGeoRichPar(void);

   void clear(void);

   void putParams(
         FairParamList*);

   Bool_t getParams(
         FairParamList*);

   TObjArray* GetGeoSensitiveNodes(){return fGeoSensNodes;}

   TObjArray* GetGeoPassiveNodes(){return fGeoPassNodes;}

private:
   /**
    * \brief Copy constructor.
    */
   CbmGeoRichPar(const CbmGeoRichPar&);

   /**
    * \brief Assignment operator.
    */
   CbmGeoRichPar& operator=(const CbmGeoRichPar&);

   ClassDef(CbmGeoRichPar,1)
};

#endif
