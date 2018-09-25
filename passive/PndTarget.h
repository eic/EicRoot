#ifndef Target_H
#define Target_H

#include "FairDetector.h"
#include "FairModule.h"

class PndTarget : public FairModule {
  public:
    PndTarget(const char * name, const char *Title="PND Target");
    PndTarget();
    virtual ~PndTarget();
    virtual void ConstructGeometry();
    ClassDef(PndTarget,1) 
  
};

#endif //Target_H

