
//
//  Suitable for mass production (see condor.sh and script.sh scripts);
//

// May want to hook up the calorimeters later (very slow though);
//#define _WITH_CALORIMETERS_

void simulation(unsigned seed = 0x12345678, int id = 0, int nEvents = 2000)
{
  int nOffset = id * nEvents;
  printf("My ID: %4d -> offset %6d\n", id, nOffset);

  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  TString SimEngine = "TGeant3";

  // Create simulation run manager; use GEANT3 for tracking excercises here;
  EicRunSim *fRun = new EicRunSim(SimEngine);
  fRun->SetOutputFile("simulation.root");
  fRun->SetSeed(seed); 

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

#ifdef _WITH_CALORIMETERS_
  //fRun->AddModule(new EicCalorimeter("BEMC", "ECAL/bemc-v01.0-ns.root", qBEMC));
  fRun->AddModule(new EicCalorimeter("CEMC", "ECAL/cemc-v00.0-ss.root", qFEMC));
  fRun->AddModule(new EicCalorimeter("FEMC", "ECAL/femc-v00.0-ss.root", qFEMC));

  fRun->AddModule(new EicCalorimeter("FHAC", "HCAL/fhac-v00.0-fs.root", qFHAC));
  fRun->AddModule(new EicCalorimeter("BHAC", "HCAL/bhac-v00.0-fs.root", qBHAC));
#endif

  // Create and set up physics Event Generator; in a mass production mode can be 
  // a huge file which will be split between different nodes;
  {
    // Input ASCII file name (PYTHIA 20x250 GeV, Q^2>0.8 cut applied;
    TString evFile = "../pythia.ep.20x250.5Mevents.1.RadCor=0.Q2-0.8..100k-lines.txt";

    // An interface to eic-smear (which is able to import PYTHIA, DJANGOH, ... files);
    EicEventGenerator* evtGen = new EicEventGenerator(evFile.Data());
    // Select primary electrons only; ignore all the rest;
    evtGen->SelectPdgCode(11);
    evtGen->SelectLeadingParticle();

    evtGen->SkipFewEvents(nOffset);
    evtGen->RestrictEventChunkSize(nEvents);

    fRun->AddGenerator(evtGen);
  }

  // Create and set up Apr'2016 solenoid field;
  fRun->AddField(new PndSolenoidMap("SolenoidMap8", "R"));

  // Initialize and run the simulation; exit at the end;
  fRun->Init(); 
  if (SimEngine.EqualTo("TGeant4")) 
    // Assume, that want a (more or less) fair calorimeter simulation in this case;
    ((TGeant4*)gMC)->ProcessGeantCommand("/mcPhysics/rangeCuts 0.01 mm");
  fRun->Run(nEvents);
} // simulation()
