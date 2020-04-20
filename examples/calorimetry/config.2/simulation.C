
//
//  Hadronic calorimeter simulation script; 
//

void simulation(unsigned nEvents = 1000)
{
  // Create simulation run manager; use GEANT4 here;
  auto fRun = new EicRunSim("TGeant4");
  fRun->SetOutputFile("simulation.root");

  // *Standard* BeAST forward HCal geometry (full structure);
  auto fhac = new EicCalorimeter("FHAC", "HCAL/fhac-v00.0-fs.root", qDUMMY);
  fRun->AddModule(fhac);

  // Create and set up (Box) Event Generator;
  {
    int      PDG = 211;                   // pion
    double  momentum = 10.0, theta = 5.0; // 10 GeV/c @ 5 degrees

    auto boxGen = new EicBoxGenerator(PDG);
    boxGen->SetMomentum(momentum);
    boxGen->SetTheta(theta);

    fRun->AddGenerator(boxGen);
  }

  // Initialize and run the simulation; exit at the end;
  fRun->Init();
  // NB: for realistic simulation need to set this value to 0.001 (1um shower 
  // particle range); it takes a long then;
  ((TGeant4*)gMC)->ProcessGeantCommand("/mcPhysics/rangeCuts 0.1 mm");
  fRun->Run(nEvents);
} // simulation()
