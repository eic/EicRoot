
#include <mutex>

#include <JANA/JEventProcessor.h>

#ifndef _EIC_JEVENT_PROCESSOR_
#define _EIC_JEVENT_PROCESSOR_

#include <FairRun.h>

class EicJEventProcessor:public JEventProcessor {
  //protected:
  //std::mutex mymutex;

 public:
  void Init(void) {
    printf("EicJEventProcessor::Init()     ... %u\n\n", (unsigned)pthread_self());
  };

  void Process(const std::shared_ptr<const JEvent>& aEvent) {
    printf("EicJEventProcessor::Process() #1 ... %u\n", (unsigned)pthread_self());
    
    //auto fRun = FairRun::Instance();
    //if (fRun) fRun->RunCoreProcessNextEvent();

    // If your application needs to do something serially, like write
    // to a file, then use a lock to do that here.
    //std::lock_guard<std::mutex> lck(mymutex);
  };

  void Finish(void) {
    printf("EicJEventProcessor::Finish() ...%u\n\n", (unsigned)pthread_self());
      
    auto fRun = FairRun::Instance();
    //fRun->RunCoreImportNextEvent(); fRun->RunCoreProcessNextEvent();
    //while (fRun->RunCoreImportNextEvent()) fRun->RunCoreProcessNextEvent();
    
    if (fRun) fRun->RunCoreFinish();
  };
};

#endif

