
void simulation(Int_t nEvents = 10)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create the simulation run manager; 
  EicRunSim *fRun = new EicRunSim("TGeant3");
  fRun->SetCaveFileName("cave-40m.geo");
  fRun->SetOutputFile("simulation.root");

  // Well, do not need secondaries in this simulation; 
  fRun->SuppressSecondaries();

  // A fraction of main detector volumes (for visualization purposes mainly);
  fRun->AddModule(new EicTpc (                    "TPC/tpc-v01.0-ns.root"));  
  fRun->AddModule(new EicMaps(           "VST",   "../geometry/vst-v02.0-ns.root", qVST));

  // Low Q^2 tagger tracker mockup;
  fRun->AddModule(new EicDetector(       "LQST",  "../geometry/lqst.root",qDUMMY, qMergeStepsInOneHit));

  // Relevant part of the vacuum system;
  {
    fRun->AddModule(new EicDummyDetector("VP.CENTER",        "pCDR-2018/geometry/vacuum.system/vp.center.root"));
    fRun->AddModule(new EicDummyDetector("VP.E-GOING",       "pCDR-2018/geometry/vacuum.system/vp.e-going.root"));

    fRun->AddModule(new EicDummyDetector("VP.E-GOING.GHOST", "pCDR-2018/geometry/vacuum.system/vp.e-going.ghost.root"));
  }

  // Event Generator;
  {
    TString evFile = "/data/pythia.ep.18x275.5Mevents.1.RadCor=0.Q2.all.1M-lines.txt";

    EicEventGenerator* evtGen = new EicEventGenerator(evFile.Data());

    // Select primary protons only; ignore all the rest;
    evtGen->SelectPdgCode(11);
    evtGen->SelectLeadingParticle();

    fRun->AddGenerator(evtGen);
  }

  // Magnetic field; NB: re-scale properly!;
  {
    EicMagneticField *fField = new EicMagneticField();

    fField->AddBeamLineElementGrads("IR/pCDR-2018/madx/E.E-GOING", 1.8, kBlue);
    //fField->SuppressYokeCreation("DB2");
    fField->CreateYokeVolumes(kTRUE);

    fRun->SetField(fField);
  }

  // Initialize and run the simulation; exit at the end;
  fRun->Run(nEvents);
} // simulation()
