
void simulation(Int_t nEvents = 1000)
{  
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create simulation run manager; use GEANT3 for tracking excercises here;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Well, do not need secondaries in this simulation; 
  fRun->SuppressSecondaries();

  // "VTX" & "TPC" names here (case-insensitive) should match the respective names 
  // in vtx-builder.C and tpc-builder.C scripts used to create the detectors; 
  // qMergeStepsInOneHit as a last parameter means that if particle makes more than one 
  // step in silicon, they will be merged together and yield only one hit;
  fRun->AddModule(new EicDetector("VTX", "./vtx.root", qDUMMY, qMergeStepsInOneHit));
  // qOneStepOneHit as a last parameter means that hits will be produced for each 
  // GEANT step; gas medium has max.step of 1cm in media.geo, which makes sense for
  // a *simplistic* TPC example (remember, we are only interested to obtain reasonable 
  // track fits from some combination of hits);
  fRun->AddModule(new EicDetector("TPC", "./tpc.root", qDUMMY, qOneStepOneHit));

  // Create and set up (Box) Event Generator;
  {
    int PDG =  211;                        // pion
    double  momentum = 10.0, theta = 75.0; // 10 GeV/c @ 75 degrees
	 
    EicBoxGenerator *boxGen = new EicBoxGenerator(PDG); 
    boxGen->SetMomentum(momentum);
    boxGen->SetTheta(theta);

    fRun->AddGenerator(boxGen);
  }

  // Hook up 3T constant field aligned with Z axis;
  fRun->AddField(new PndSolenoidMap("SolenoidMap1", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()
