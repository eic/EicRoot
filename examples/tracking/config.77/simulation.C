
//
//  Tracker simulation script; all parameters hardcoded for simplicity;
//

void simulation(Int_t nEvents  = 1)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create the simulation run manager; use GEANT4 transport;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Well, do not need secondaries in this simulation; 
  fRun->SuppressSecondaries();

  // "FWDST" name here (case-insensitive) should match the respective name 
  // in tracker.C script used to create the detector; one can actually 
  // create more than one tracking detector this way, and as long as their names differ
  // (and physical locations do not overlap), all the simulation/digitization/reconstruction 
  // scheme will work (including Kalman filter track fitting); qMergeStepsInOneHit as a 
  // last parameter means that if particle makes more than one step in silicon, they will 
  // be merged together and yield only one hit;
  fRun->AddModule(new EicDetector("FWDST", "./fwdst.root", qDUMMY, qMergeStepsInOneHit));

  // Create and set up (Box) Event Generator;
  {
    int PDG = 211;                              // pion
    double pmin = 5.0, pmax = 5.0, theta = 3.0; // [9..11] GeV/c @ 5 degrees
 
    EicBoxGenerator* boxGen = new EicBoxGenerator(PDG); 
    boxGen->SetMomentumRange(pmin, pmax);
    boxGen->SetTheta(theta);
    boxGen->SetPhi(45.0);

    fRun->AddGenerator(boxGen);
  }

  // Create and set up Elmer-based Apr'2016 solenoid field (binary file "input/SolenoidMap8.root");
  //fRun->AddField(new PndSolenoidMap("SolenoidMap8", "R"));
  fRun->AddField(new PndDipoleMap("DipoleMap3", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()
