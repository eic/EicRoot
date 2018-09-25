// -------------------------------------------------------------------------
// -----                     PndBlackBoxTask source file             -----
// -----                  Created 08/06/10  by R. Kliemt               -----
// -------------------------------------------------------------------------

#include "PndBlackBoxTask.h"

PndBlackBoxTask::PndBlackBoxTask(const char* name) :
FairTask(name)
{
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
PndBlackBoxTask::~PndBlackBoxTask()
{
}
// -------------------------------------------------------------------------

// -----   Initialization  of Parameter Containers -------------------------
void PndBlackBoxTask::SetParContainers()
{
}

InitStatus PndBlackBoxTask::ReInit()
{
  return kSUCCESS;
}

// -------------------------------------------------------------------------

void PndBlackBoxTask::SetVerbose(Int_t iVerbose)
{
  fVerbose = iVerbose;
  TList* thistasks = this->GetListOfTasks();
  for(Int_t i=0;i<thistasks->GetEntries();i++)
  {
    ((FairTask*)thistasks->At(i))->SetVerbose(fVerbose);
  }
}

// -----   Public method Init   --------------------------------------------
InitStatus PndBlackBoxTask::Init()
{
  return kSUCCESS;
}
// -------------------------------------------------------------------------

// -----   Public method Exec   --------------------------------------------
void PndBlackBoxTask::Exec(Option_t* opt)
{
  return;
}

ClassImp(PndBlackBoxTask);

