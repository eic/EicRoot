//
// AYK (ayk@bnl.gov), 2015/08/04
//
//  A derived class with the only purpose to expand functionality 
// of FairMCApplication::Stepping() call;
//

#include <FairMCApplication.h>

#ifndef _EIC_MC_APPLICATION_
#define _EIC_MC_APPLICATION_

class EicMCApplication : public FairMCApplication
{
 public:
  EicMCApplication() {};
 EicMCApplication(const char* name,   const char* title, TObjArray* ModList, const char* MatName):
  FairMCApplication(name, title, ModList, MatName) {};
  ~EicMCApplication() {};
  
  void Stepping();

  ClassDef(EicMCApplication,1)
};

#endif
