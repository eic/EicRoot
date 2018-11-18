
//
//  Simulated data file browser;
//

void eventDisplay()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create visualization manager; 
  EicEventManager *fMan = new EicEventManager();
  fMan->SetInputFile("simulation.root");
 
  // Define objects to be displayed;
  fMan->AddTask(new FairMCTracks   ("Monte-Carlo Tracks"));
  fMan->AddTask(new FairMCPointDraw("LqstMoCaPoint",     kRed,  kFullSquare));

  // Initialize and run visualization manager;
  fMan->Run();                

  //
  // Once graphics window starts up, in ROOT browser follow the sequence:
  //   "Fair Event Manager" -> "Info" -> increment "Current Event";
  //
} // eventDisplay()

