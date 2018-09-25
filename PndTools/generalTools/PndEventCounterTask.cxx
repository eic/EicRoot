// -------------------------------------------------------------------------
// -----                   PndEventCounterTask source file             -----
// -----                  Created 24/09/10  by R. Kliemt               -----
// -------------------------------------------------------------------------

#include "PndEventCounterTask.h"
#include <iostream>

PndEventCounterTask::PndEventCounterTask(const char* name, Int_t nev, Int_t talk) :
FairTask(name),
fInitialiezed(kFALSE),
fEvtCounter(0),
fEvtTalk(talk),
fNEvts(nev),
fTimeOffset(0.),
fTimer()
{
  fTimer.Start();
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
PndEventCounterTask::~PndEventCounterTask()
{
}
// -------------------------------------------------------------------------

void PndEventCounterTask::StartTimer(){
  fTimeOffset=fTimer.RealTime();
  fTimer.Continue();
  fEvtCounter = 0;
  fInitialiezed = kTRUE;
}

// -----   Public method Exec   --------------------------------------------
void PndEventCounterTask::Exec(Option_t* opt)
{
  if (!fInitialiezed) StartTimer();
  fEvtCounter++;
  if(fEvtCounter%fEvtTalk == 0 || fVerbose>1)
  {
    Double_t t=fTimer.RealTime();
    fTimer.Continue();
    printf("Event %i/%i : time %6.1f sec, (%6.0f sec remaining)\n",
           fEvtCounter,fNEvts,t,(t-fTimeOffset)*(fNEvts-fEvtCounter)/fEvtCounter);
  }  
  return;
}

ClassImp(PndEventCounterTask);

