//
// AYK (ayk@bnl.gov), 2014/10/14
//
//  Bare digi class;
//

#include <TFile.h>

#include <FairRun.h>

#include "EicDigiParData.h"

// -----------------------------------------------------------------------------------------------

int EicDigiParData::mergeIntoOutputFile(TString name)
{
  FairRun *fRun = FairRun::Instance();

  // I guess there is no need to save/restore current directory here?;
  fRun->GetOutputFile()->cd();
  
  // Yes, save under detector-specific pre-defined name;
  //digi->Write(dname->cname() + "DigiParData");
  Write(name);

  return 0;
} // EicDigiParData::mergeIntoOutputFile()

// -----------------------------------------------------------------------------------------------

ClassImp(EicDigiParData)
