//
// AYK (ayk@bnl.gov), 2013/06/13
//
// Need this nonsense derived class only to be able to split 
// FairEventManager::Init() method into two (Init() & Run())
// and be able to access gGeoManager from within eventDisplay.C;
//

#include <FairEventManager.h>

#ifndef _EIC_EVENT_MANAGER_
#define _EIC_EVENT_MANAGER_

class EicEventManager: public FairEventManager 
{
 public:
  EicEventManager();
  ~EicEventManager() {};

  void SetInputFile(TString fname);

  //
  // Prefer to keep Init() -> Run() splitting, even that in default mode 
  // both calls should happen one after the other; user may still want to 
  // modify graphics settings in eventDisplay.C scripts (say change 
  // transparency or visibility of some of the volumes);
  //

  void Init();
  // NB: default behaviour of TEveGeoTopNode() is (1,3); choose (0,6) 
  // and allow for full configurability in this call if needed;
  void Run(Int_t visopt = 0, Int_t vislvl = 6, Int_t maxvisnds = 10000);

 private:
  Bool_t mInitCallHappened;          //!

  ClassDef(EicEventManager,2);
};

#endif
