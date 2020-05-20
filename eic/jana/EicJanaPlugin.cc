
#include <cstring>

#include <TROOT.h>

#include <JANA/JApplication.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <JANA/JEvent.h>

#include <FairRun.h>

#include <EicJEventProcessor.h>
#include <EicJEventSource.h>

extern "C" {
  void InitPlugin(JApplication *app) { 
    // Does not have real effect at present (threads do not access any ROOT
    // data concurrently);
    ROOT::EnableThreadSafety();

    auto pm = app->GetJParameterManager();
    assert(pm->Exists("libeicroot:macro"));

    // Disable the fRun->Run() call in the macro for EicRunAna;
    FairRun::JanaPluginMode(true);
    
    //+++gROOT->Macro(pm->GetParameterValue<string>("libeicroot:macro").c_str());
    //gROOT->Macro((pm->GetParameterValue<string>("libeicroot:macro") + "(3)").c_str());
    
    auto fRun = FairRun::Instance();
    // FIXME: this is a hack to let EicRunAna go through, but block EicRunSim;
    if (fRun && fRun->JanaLoopPossible()) fRun->RunCoreStart();
    
    InitJANAPlugin(app);
    
    // Add source generator
    app->Add( new JEventSourceGeneratorT<EicJEventSource>() );
    
    // Add event processor
    app->Add( new EicJEventProcessor() );
  } // InitPlugin()
} // "C"

