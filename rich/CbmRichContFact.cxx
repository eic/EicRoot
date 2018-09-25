//*-- AUTHOR : Denis Bertini
//*-- Created : 21/06/2005


/////////////////////////////////////////////////////////////
//
//  CbmRichContFact
//
//  Factory for the parameter containers in libRich
//
/////////////////////////////////////////////////////////////

#include "CbmGeoRichPar.h"

#include "CbmRichContFact.h"
#include "FairRuntimeDb.h"

#include <iostream>
#include <iomanip>

ClassImp(CbmRichContFact)

static CbmRichContFact gCbmRichContFact;

CbmRichContFact::CbmRichContFact() {
  // Constructor (called when the library is loaded)
  fName="CbmRichContFact";
  fTitle="Factory for parameter containers in libRich";
  setAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}

void CbmRichContFact::setAllContainers() {
  /** Creates the Container objects with all accepted contexts and adds them to
   *  the list of containers for the STS library.*/

    FairContainer* p= new FairContainer("CbmGeoRichPar",
                                          "Rich Geometry Parameters",
                                          "TestDefaultContext");
    p->addContext("TestNonDefaultContext");

    containers->Add(p);
}

FairParSet* CbmRichContFact::createContainer(FairContainer* c) {
  /** Calls the constructor of the corresponding parameter container.
   * For an actual context, which is not an empty string and not the default context
   * of this container, the name is concatinated with the context. */
  const char* name=c->GetName();
  FairParSet* p=NULL;
  if (strcmp(name,"CbmGeoRichPar")==0) {
    p=new CbmGeoRichPar(c->getConcatName().Data(),c->GetTitle(),c->getContext());
  }
  return p;
}

