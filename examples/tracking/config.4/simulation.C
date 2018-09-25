
//
//  Tracker simulation script; 
//

void simulation(Int_t nEvents  = 1000)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create the simulation run manager; use GEANT3 transport;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Well, do not need secondaries in this simulation; 
  fRun->SuppressSecondaries();

  // Declare detector; 
  fRun->AddModule(new EicDetector("FWDGT", "./fwdgt.root", qDUMMY, qMergeStepsInOneHit));

  // Create and set up (Box) Event Generator;
  {
    int PDG = 211;                       // pion
    double momentum = 20.0, theta = 2.10; // 20 GeV/c @ eta=4
 
    EicBoxGenerator* boxGen = new EicBoxGenerator(PDG); 
    boxGen->SetMomentum(momentum);
    boxGen->SetTheta(theta);

    fRun->AddGenerator(boxGen);
  }

  // Create and set up either homogeneous 1.5T field (SolenoidMap2) or more or 
  // less realistic BaBaR magnet field (SolenoidMap3 or SolenoidMap5); see
  // input/README for more details;
  fRun->AddField(new PndSolenoidMap("SolenoidMap2", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()
