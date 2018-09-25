#ifndef CBMRICHCONTFACT_H
#define CBMRICHCONTFACT_H

#include "FairContFact.h"

class FairContainer;

class CbmRichContFact : public FairContFact {
private:
  void setAllContainers();
public:
  CbmRichContFact();
  ~CbmRichContFact() {}
  FairParSet* createContainer(FairContainer*);
  ClassDef( CbmRichContFact,0) // Factory for all RICH parameter containers
};

#endif  /* !CBMRICHCONTFACT_H */
