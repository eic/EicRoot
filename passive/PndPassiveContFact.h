#ifndef PNDPASSIVECONTFACT_H
#define PNDPASSIVECONTFACT_H

#include "FairContFact.h"

class FairContainer;

class PndPassiveContFact : public FairContFact {
private:
  void setAllContainers();
public:
  PndPassiveContFact();
  ~PndPassiveContFact() {}
  FairParSet* createContainer(FairContainer*);
  ClassDef( PndPassiveContFact,0) // Factory for all Passive parameter containers
};

#endif  /* !PNDPASSIVECONTFACT_H */
