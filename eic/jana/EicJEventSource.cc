
#include <TVirtualMC.h>

#include <FairGeane.h>
#include <FairRun.h>

#include <EicJEventSource.h>

// ------------------------------------------------------------------------------------------

void EicJEventSource::Open(void)
{
  //+++SetNumEventsToGetAtOnce(1, 1);
} // EicJEventSource::Open()

// ------------------------------------------------------------------------------------------

void EicJEventSource::GetEvent(std::shared_ptr<JEvent> jevent)
{
  auto fRun = FairRun::Instance();
  if (!fRun || !fRun->JanaLoopPossible() || !fRun->RunCoreImportNextEvent()) 
    throw JEventSource::RETURN_STATUS::kNO_MORE_EVENTS;

  // This is indeed a hack, helping reconstruction.C to initialize 
  // geane-related stuff (and gMC in particular) in every thread;
  if (!gMC) (new FairGeane())->Init();

  printf("\nEicJEventSource::GetEvent() - %4d(+1) [%u]\n", 
	 fRun->GetCurrentEventIndex(), (unsigned)pthread_self());

  fRun->RunCoreProcessNextEvent();
} // EicJEventSource::GetEvent()

// ------------------------------------------------------------------------------------------

