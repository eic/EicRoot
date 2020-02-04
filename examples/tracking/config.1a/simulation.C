
//
//  Tracker simulation script; 
//

void simulation(Int_t nEvents  = 1000)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create the simulation run manager; use GEANT4 transport; may want to use 
  // "TGeant3" for comparison;
  EicRunSim *fRun = new EicRunSim("TGeant4");
  // Define the output file with the simulated tracks and GEANT hits;
  fRun->SetOutputFile("simulation.root");

  // Well, do not need secondaries in this simulation; 
  fRun->SuppressSecondaries();

  // "FWDST" name here (case-insensitive) should match the respective name 
  // in the tracker.C script used to create the detector; one can actually 
  // create more than one tracking detector this way, and as long as their names differ
  // (and physical locations do not overlap), all the simulation/digitization/reconstruction 
  // scheme will work (including Kalman filter track fitting); qMergeStepsInOneHit as a 
  // last parameter means that if particle makes more than one step in silicon, they will 
  // be merged together and yield only one hit; be aware that the simulation.root output
  // file will contain 1) a full description of the actual geometry, 2) reference to a 
  // magnetic field map, therefore bookkeeping of the detector configuration is done 
  // "automatically" and no other manual input in either digitization.C or reconstruction.C 
  // scripts is required;
  fRun->AddModule(new EicDetector("FWDST", "./fwdst.root", qDUMMY, qMergeStepsInOneHit));

  // Create and set up (Box) Event Generator;
  {
    int PDG = 211;                               // pi-
    double pmin = 9.0, pmax = 11.0, theta = 5.0; // [9..11] GeV/c @ 5 degrees polar angle
 
    EicBoxGenerator* boxGen = new EicBoxGenerator(PDG); 
    // Define momentum range and theta; the rest is default: 2pi range in phi, 
    // primary vertex location (0,0,0) with no smearing;
    boxGen->SetMomentumRange(pmin, pmax);
    boxGen->SetTheta(theta);

    fRun->AddGenerator(boxGen);
  }

  // Create and set up Elmer-based Apr'2016 solenoid magnetic field (binary 
  // file "input/SolenoidMap8.root");
  fRun->AddField(new PndSolenoidMap("SolenoidMap8", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()
