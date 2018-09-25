#ifndef Cave_H
#define Cave_H

#include "FairDetector.h"
#include "FairModule.h"


class PndCave : public FairModule {
  public:
    PndCave(const char * name, const char *Title="Exp Cave");
    PndCave();
    virtual ~PndCave();
    virtual void ConstructGeometry();
    

private:
     Double_t world[3];
     ClassDef(PndCave,1) //PNDCaveSD
};

#endif //Cave_H

