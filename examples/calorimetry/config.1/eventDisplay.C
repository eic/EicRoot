
//
//  Simulated data file browser;
//

void eventDisplay()
{
  // Create visualization manager; 
  auto fMan = new EicEventManager();
  fMan->SetInputFile("simulation.root");

  // Want just tracks and calorimeter hits; 
  fMan->AddTask(new FairMCTracks   ("Monte-Carlo Tracks")); 
  fMan->AddTask(new FairMCPointDraw("CalorimeterMoCaPoint", kRed, kFullSquare));

  // Initialize and run visualization manager;
  fMan->Run();                  

  //
  // Once graphics window starts up, in ROOT browser follow the sequence:
  //   "Fair Event Manager" -> "Info" -> increment "Current Event";
  //
} // eventDisplay()
  

