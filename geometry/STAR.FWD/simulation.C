
void simulation(Int_t nEvents = 1)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create simulation run manager; use GEANT3 for tracking excercises here;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Yes, secondaries of no need here;
  fRun->SuppressSecondaries();

  fRun->AddModule(new EicDummyDetector("HCAL", "./hcal-v00.0.root"));

  // Create and set up (Box) Event Generator;
  {
    // Be lazy, hardcode polar angles matching eta [0..4]: 
    // ~0.00,  0.50,  1.00,  1.50,  2.00, 2.50, 3.00, 3.50, 4.00;
    // 89.00, 62.48, 40.40, 25.16, 15.41, 9.39, 5.70, 3.46, 2.10;

    int PDG = 211;                          // pion
    //double  momentum = 10.0, theta = 25.16; //
    double  momentum = 50.0, theta = 3.46; //
	 
    EicBoxGenerator *boxGen = new EicBoxGenerator(PDG); 
    boxGen->SetMomentum(momentum);
    boxGen->SetTheta(theta);

    fRun->AddGenerator(boxGen);
  }

  // Create and set up Apr'2016 solenoid field (binary file "input/SolenoidMap8.root");
  fRun->AddField(new PndSolenoidMap("SolenoidMap8", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()

