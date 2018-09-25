
void eventDisplay()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create visualization manager; 
  EicEventManager *fMan = new EicEventManager();
  fMan->SetInputFile("simulation.root");
 
  // Define objects to be displayed;
  fMan->AddTask(new FairMCTracks   ("Monte-Carlo Tracks"));    
  fMan->AddTask(new FairMCPointDraw("FwdgtMoCaPoint",kRed, kFullSquare));

  // Initialize and run visualization manager;
  fMan->Run();       
} // eventDisplay()
  
