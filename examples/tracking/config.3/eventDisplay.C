
#include <simulation.C>

void eventDisplay()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create visualization manager; 
  EicEventManager *fMan = new EicEventManager();
  fMan->SetInputFile("simulation.root");
 
  // Define objects to be displayed;
  fMan->AddTask(new FairMCTracks ("Monte-Carlo Tracks"));
  fMan->AddTask(new FairMCPointDraw ("VstMoCaPoint", kRed,   kFullSquare));
  fMan->AddTask(new FairMCPointDraw ("FstMoCaPoint", kRed,   kFullSquare));
  fMan->AddTask(new FairMCPointDraw ("FgtMoCaPoint", kBlue,  kFullSquare));
  fMan->AddTask(new FairMCPointDraw ("BstMoCaPoint", kRed,   kFullSquare));
  fMan->AddTask(new FairMCPointDraw ("BgtMoCaPoint", kBlue,  kFullSquare));
  fMan->AddTask(new FairMCPointDraw ("TpcMoCaPoint", kGreen, kFullSquare));
#ifdef _WITH_MUMEGAS_
  fMan->AddTask(new FairMCPointDraw ("MmtMoCaPoint", kBlue,  kFullSquare));
#endif

  // Initialize and run visualization manager;
  fMan->Run();                       

  //
  // Once graphics window starts up, in ROOT browser follow the sequence:
  //   "Fair Event Manager" -> "Info" -> increment "Current Event";
  //
} // eventDisplay()
  
