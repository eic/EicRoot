//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Interface to eic-smear package;
//

#ifndef _EIC_SMEAR_TASK_
#define _EIC_SMEAR_TASK_

#include <TClonesArray.h>

#include <FairTask.h>

#include "eicsmear/erhic/EventFactory.h"
#include "eicsmear/smear/EventDisFactory.h"

class TTree;
class TFile;
class TBranch;

class EicSmearTask: public FairTask {
 public:
  EicSmearTask() {};
 EicSmearTask(TString _inFileName, Smear::Detector *rDetector = 0): 
  inFileName(_inFileName), mcTree(0), builder(0), detector(rDetector) {};
  ~EicSmearTask() {};

  InitStatus Init();

  void SetParContainers();
  void Exec(Option_t* opt);

  void FinishTask();

 private:
  TString inFileName;
  Smear::Detector *detector;

  TFile *inFile, *outFile;
  TTree* mcTree, *smearedTree;
  TBranch* eventbranch;
  erhic::VirtualEventFactory* builder;

  TClonesArray *fMCTracks, *fPidChargedCand;

  ClassDef(EicSmearTask,12);
};

#endif
