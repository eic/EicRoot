//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  FairRoot-style EIC container factory definitions; eventually will need 
//  to figure out how to make real use of all this stuff;
//

#include <iostream>
#include <iomanip>

#include "FairRuntimeDb.h"
#include "FairParRootFileIo.h"
#include "FairParAsciiFileIo.h"

#include "EicContFact.h"
#include "EicDetector.h"

// ---------------------------------------------------------------------------------------

EicContFact::EicContFact(EicDetector *det, const char *_fName, const char *_fTitle, 
			 const char* name, const char* title, const char* context) {
  fName  = strdup(_fName);
  fTitle = strdup(_fTitle);

  FairContainer* p= new FairContainer(name, title, context);
  containers->Add(p);
  
  fGeoParName    = strdup(name);
  backDoorDetPtr = det;

  FairRuntimeDb::instance()->addContFactory(this);
} // EicContFact::EicContFact()

// ---------------------------------------------------------------------------------------

FairParSet* EicContFact::createContainer(FairContainer* c) {
  const char* name=c->GetName();
  FairParSet* p = NULL;

  if (strcmp(name, fGeoParName) == 0) p = backDoorDetPtr->EicGeoParAllocator(c);

  return p;
} // EicContFact::createContainer()

// ---------------------------------------------------------------------------------------

ClassImp(EicContFact)
