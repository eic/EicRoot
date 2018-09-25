//*-- AUTHOR : Ilse Koenig
//*-- Created : 10/11/2003

/////////////////////////////////////////////////////////////
// PndGeoPipe
//
// Class for geometry of support structure
//
/////////////////////////////////////////////////////////////

#include "PndGeoPipe.h"
#include "FairGeoLoader.h"
#include "FairGeoInterface.h"
    
ClassImp(PndGeoPipe)

PndGeoPipe::PndGeoPipe() {
  // Constructor
  fName="pipe";
  strcpy(modName,"p");
  strcpy(eleName,"p");
  maxSectors=0;
  maxModules=1;
}
Bool_t  PndGeoPipe::create ( FairGeoBuilder * build ) {
    Bool_t rc = FairGeoSet::create( build );
    if ( rc ) {
	FairGeoLoader *loader=FairGeoLoader::Instance();
	FairGeoInterface *GeoInterface =loader->getGeoInterface();
	
	GeoInterface->getMasterNodes()->Add( (TObject*)  getVolume("pipeCentral") );
    }
 return rc;
}

PndGeoPipe::~PndGeoPipe() {

}

