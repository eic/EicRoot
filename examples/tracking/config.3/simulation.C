
//
//  Simulation for BeAST detector tracker reslution estimates;
//

// Uncomment this if want to add 2x2 2D MuMegas barrels to the setup;
// there are also nasty #include's in all other C scripts which allow 
// to propagate this information further down the chain;
#define _WITH_MUMEGAS_

void simulation(Int_t nEvents = 1000)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create simulation run manager; use GEANT3 for tracking excercises here;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Yes, secondaries of no need here;
  fRun->SuppressSecondaries();

  fRun->AddModule(new EicDummyDetector("SOLENOID", "/DATA00/ayk/ZIGZAG/solenoid-v01.0.root"));

  // Vertex tracker;
  fRun->AddModule(new EicMaps   ("VST", "MAPS/vst-v01.0-ss.root", qVST));
  fRun->AddModule(new EicMaps   ("FST", "MAPS/fst-v01.0-ss.root", qFST));
  fRun->AddModule(new EicMaps   ("BST", "MAPS/bst-v01.0-ss.root", qBST));
  
  // GEM disks;
  fRun->AddModule(new EicGem    ("FGT", "GEM/fgt-v01.0.root",     qFGT));
  fRun->AddModule(new EicGem    ("BGT", "GEM/bgt-v01.0.root",     qBGT));

  // Beam pipe; details of IR not known yet -> just a cylinder with thin walls;
  fRun->AddModule(new EicDummyDetector("BEAMPIPE", "BEAMPIPE/beampipe.root"));

  // TPC;
  EicTpc *tpc =   new EicTpc    ("TPC/tpc-v01.0-ns.root");
  //EicEnergyMonitor *monitor = tpc->AddEnergyMonitorVolume("TpcGas", 11, "hist", 0., 25.);
  //monitor->AtVolumeExit(); 
  //monitor->PrimaryOnly();
  fRun->AddModule(tpc);

#ifdef _WITH_MUMEGAS_
  fRun->AddModule(new EicMuMegas("MMT", "MUMEGAS/mmt-v01.0.root",  qDUMMY));
#endif

  // PID detector place holders;
  fRun->AddModule(new EicDetector("FLR", "RICH/flr-v01.0.root", qDUMMY, qMergeStepsInOneHit));
  fRun->AddModule(new EicDetector("BLR", "RICH/blr-v01.0.root", qDUMMY, qMergeStepsInOneHit));
  fRun->AddModule(new EicDetector("CLR", "RICH/clr-v01.0.root", qDUMMY, qMergeStepsInOneHit));

  // RICH objects; NB: ROOT has to be patched to make SetScale() working (!!!);
  // ~/FairRoot/eicroot/ is configured to use such one (?);
#if 1
  CbmRich *frich = new CbmRich("FRICH", kTRUE, 0, 0,  55.);            // Z-offset in [cm];
  frich->SetGeometryFileName("rich_v13a_f.gdml"); frich->SetScale(0.6);//0.7);
  fRun->AddModule(frich);
#endif

  fRun->AddModule(new EicCalorimeter("BEMC", "ECAL/bemc-v01.0-ns.root", qBEMC));
  fRun->AddModule(new EicCalorimeter("CEMC", "ECAL/cemc-v01.0-ns.root", qFEMC));
  fRun->AddModule(new EicCalorimeter("FEMC", "ECAL/femc-v01.0-ns.root", qFEMC));
  fRun->AddModule(new EicCalorimeter("FHAC", "HCAL/fhac-v01.0-ns.root", qFHAC));
  fRun->AddModule(new EicCalorimeter("BHAC", "HCAL/bhac-v01.0-ns.root", qBHAC));

  // Create and set up (Box) Event Generator;
  {
    // Be lazy, hardcode polar angles matching eta [0..4]: 
    // ~0.00,  0.50,  1.00,  1.50,  2.00, 2.50, 3.00, 3.50, 4.00;
    // 89.00, 62.48, 40.40, 25.16, 15.41, 9.39, 5.70, 3.46, 2.10;

    int PDG = 211;                          // pion
    double  momentum = 10.0, theta = 25.16; //
	 
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

