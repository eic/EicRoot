
//
//  Simulation for BeAST detector tracker reslution estimates;
//

void simulation(Int_t nEvents = 1000)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create simulation run manager; use GEANT3 for tracking excercises here;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Yes, secondaries of no need here;
  fRun->SuppressSecondaries();

  // Vertex tracker;
  fRun->AddModule(new EicMaps   ("VST", "MAPS/vst-v01.0-ss.root", qVST));
  fRun->AddModule(new EicMaps   ("FST", "MAPS/fst-v01.0-ss.root", qFST));

  // GEM disks, in front and behind the RICH;
  fRun->AddModule(new EicGem    ("FGT", "./fgt-v01.0.root",       qFGT));
  fRun->AddModule(new EicGem    ("FFG", "./ffg-v01.0.root",       qBGT));

  // TPC;
  fRun->AddModule(new EicTpc    ("TPC/tpc-v01.0-ns.root")); 

  // Beam pipe; details of IR not known yet -> just a cylinder with thin walls;
  //fRun->AddModule(new EicDummyDetector("BEAMPIPE", "BEAMPIPE/beampipe.root"));

  // RICH;
  fRun->AddModule(new EicDetector("RICH", "./rich-v01.0.root", qDUMMY, qOneStepOneHit));

  // Create and set up (Box) Event Generator;
  {
    // Be lazy, hardcode polar angles matching eta [0..4]: 
    // ~0.00,  0.50,  1.00,  1.50,  2.00, 2.50, 3.00, 3.50, 4.00;
    // 89.00, 62.48, 40.40, 25.16, 15.41, 9.39, 5.70, 3.46, 2.10;

    int PDG = 211;                         // pion
    double momentum = 5.0, theta = 15.41; //
    //double momentum = 50.0, theta = 62.48;
	 
    EicBoxGenerator *boxGen = new EicBoxGenerator(PDG); 
    boxGen->SetMomentum(momentum);
    boxGen->SetTheta(theta);
    //boxGen->SetPhi(50.0);

    fRun->AddGenerator(boxGen);
  }

  // Create and set up Apr'2016 solenoid field (binary file "input/SolenoidMap8.root");
  //fRun->AddField(new PndSolenoidMap("SolenoidMap2", "R"));
  // ~3T BeAST central field with the realistic fringe fields;  
  fRun->AddField(new PndSolenoidMap("SolenoidMap8", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()

