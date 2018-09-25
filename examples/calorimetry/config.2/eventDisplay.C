
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
 
  // Want just tracks and calorimeter hits; 
  fMan->AddTask(new FairMCTracks   ("Monte-Carlo Tracks")); 
  fMan->AddTask(new FairMCPointDraw("FhacMoCaPoint", kRed, kFullSquare));

  // Initialize and run visualization manager; tune vis.properties a bit;
  fMan->Init();  
  setColors();              
  fMan->Run();                     

  //
  // Once graphics window starts up, in ROOT browser follow the sequence:
  //   "Fair Event Manager" -> "Info" -> increment "Current Event";
  //
} // eventDisplay()
  

void setColors()
{
  TIter next( gGeoManager->GetListOfVolumes() );

  // Make cells bit transparent (to see showers);
  while ((volume=(TGeoVolume*)next())) {
    TString name = volume->GetName();

    if (name.BeginsWith("FhacQuadrant"))
      volume->SetVisibility(kFALSE);
    else if (name.BeginsWith("FhacTower")) 
      volume->SetTransparency(70);
  } //while
} // setColors()

