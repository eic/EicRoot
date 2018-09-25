#ifndef PIPE_H
#define PIPE_H

#include "TNamed.h"
#include "TArrayI.h"
#include "TClonesArray.h"
#include "FairDetector.h"
#include "FairModule.h"


class PndPipe : public FairModule {
  public:
    PndPipe(const char * name, const char *Title="PND Pipe");
    PndPipe();

    virtual ~PndPipe();
    virtual void ConstructGeometry();
    Bool_t CheckIfSensitive(std::string name);   
  ClassDef(PndPipe,1) //PNDPIPE

};

#endif //PIPE_H

