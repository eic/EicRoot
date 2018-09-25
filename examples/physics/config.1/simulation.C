

void simulation(int nEvents = 2000)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create simulation run manager; use GEANT3 for tracking excercises here;
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetOutputFile("simulation.root");

  // Suppress secondary particle production (work with primary electrons); 
  fRun->SuppressSecondaries();

  // Beam pipe; 
  fRun->AddModule(new EicDummyDetector("BEAMPIPE", "BEAMPIPE/beampipe.root"));

  // Silicon detectors;
  fRun->AddModule(new EicMaps("VST", "MAPS/vst-v01.0-ss.root", qVST));
  fRun->AddModule(new EicMaps("FST", "MAPS/fst-v01.0-ss.root", qFST));
  fRun->AddModule(new EicMaps("BST", "MAPS/bst-v01.0-ss.root", qBST));
  
  // GEM disks;
  fRun->AddModule(new EicGem ("FGT", "GEM/fgt-v01.0.root",  qFGT));
  fRun->AddModule(new EicGem ("BGT", "GEM/bgt-v01.0.root",  qBGT));

  // TPC;
  fRun->AddModule(new EicTpc (       "TPC/tpc-v01.0-ns.root"));

  // Create and set up physics Event Generator;
  {
    // Input ASCII file name (PYTHIA 20x250 GeV, Q^2>0.8 cut applied;
    TString evFile = "../pythia.ep.20x250.5Mevents.1.RadCor=0.Q2-0.8..100k-lines.txt";

    // An interface to eic-smear (which is able to import PYTHIA, DJANGOH, ... files);
    EicEventGenerator* evtGen = new EicEventGenerator(evFile.Data());
    // Select primary electrons only; ignore all the rest;
    evtGen->SelectPdgCode(11);
    evtGen->SelectLeadingParticle();

    fRun->AddGenerator(evtGen);
  }

  // Create and set up the Apr'2016 solenoid field;
  fRun->AddField(new PndSolenoidMap("SolenoidMap8", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()
