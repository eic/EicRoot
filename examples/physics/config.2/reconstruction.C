
#include <simulation.C>

void reconstruction(unsigned seed = 0x12345678)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for track reconstruction;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->AddFriend    ("digitization.root");
  fRun->SetOutputFile("reconstruction.root");
  fRun->SetSeed(seed);

  EicIdealTrackingCode* idealTracker = new EicIdealTrackingCode();
  idealTracker->AddDetectorGroup("FST");
  idealTracker->AddDetectorGroup("BST");
  idealTracker->AddDetectorGroup("VST");
  idealTracker->AddDetectorGroup("TPC");
  idealTracker->AddDetectorGroup("FGT");
  idealTracker->AddDetectorGroup("BGT");
  idealTracker->SetRelativeMomentumSmearing(0.1);
  idealTracker->SetVertexSmearing(0.01, 0.01, 0.01);
  idealTracker->SetTrackOutBranchName("EicIdealTrack");
  fRun->AddTask(idealTracker);

  // Invoke and configure PandaRoot Kalman filter code wrapper;
  fRun->AddTask(new EicRecoKalmanTask(idealTracker));

  // This call here just performs track backward propagation to the beam line; 
  fRun->AddTask(new PndPidCorrelator());

  // Well, need to tune this stuff and add HCal sections;
#ifdef _WITH_CALORIMETERS_
  {
    EicCalorimeterReconstruction *bemc = 
      new EicCalorimeterReconstruction("BEMC", mcInFile, digiInFile);
    // 1-st: cluster seed threshold (cells below that can not initiate clustering);
    // 2-d : neighbour search threshold (cells below that can not initiate neighbour
    //       attachment for the cluster they are associated with);
    // 3-d : cell cutoff threshold (cells below that are ignored);
    //@@@bemc->SetClusterAlgorithmThresholds(0.3, 0.03, 0.005);
    bemc->SetClusterAlgorithmThresholds(0.100, 0.010, 0.005);
    // Well, sampling fraction is ~100% here, except for the losses in alveoles,
    // leaks and unattached hits; those contribute ~20% however -> need to apply 
    // not exactly the primary light yield used in digitization.C in order to match 
    // 1 GeV energy;
    //@@@bemc->SetPhotonToEnergyConversionFactor(1/18400.);
    bemc->SetPhotonToEnergyConversionFactor(1/8500.);
    // 1E5 photons scale;
    bemc->RequestLightYieldPlot(100000);
    fRun->AddTask(bemc);
  }
  {
    // Declare custom calorimeter reconstruction; detector choice is done by 
    // name encoded in calorimeter.root file;
    EicCalorimeterReconstruction *femc = 
      new EicCalorimeterReconstruction("FEMC", mcInFile, digiInFile);
    // 1-st: cluster seed threshold (cells below that can not initiate clustering);
    // 2-d : neighbour search threshold (cells below that can not initiate neighbour
    //       attachment for the cluster they are associated with);
    // 3-d : cell cutoff threshold (cells below that are ignored);
    femc->SetClusterAlgorithmThresholds(0.100, 0.005, 0.005);
    femc->SetPhotonToEnergyConversionFactor(1/450.);
    femc->RequestLightYieldPlot(1000);
    fRun->AddTask(femc);
  }
  {
    // Declare custom calorimeter reconstruction; detector choice is done by 
    // name encoded in calorimeter.root file;
    EicCalorimeterReconstruction *cemc = 
      new EicCalorimeterReconstruction("CEMC", mcInFile, digiInFile);
    // 1-st: cluster seed threshold (cells below that can not initiate clustering);
    // 2-d : neighbour search threshold (cells below that can not initiate neighbour
    //       attachment for the cluster they are associated with);
    // 3-d : cell cutoff threshold (cells below that are ignored);
    cemc->SetClusterAlgorithmThresholds(0.100, 0.005, 0.005);
    cemc->SetPhotonToEnergyConversionFactor(1/400.);
    cemc->RequestLightYieldPlot(1000);
    fRun->AddTask(cemc);
  }
#endif

  EicEventAssembler *eea = new EicEventAssembler();
#ifdef _WITH_CALORIMETERS_
  // NB: e/m calorimeters are expected to be ordered in rapidity range, 
  // otherwise sanity check in th ecode may fail;
  eea->AddEmCal("FEMC");
  eea->AddEmCal("CEMC");
  eea->AddEmCal("FEMC");
  eea->AddEmCal("BEMC");

  eea->AddHCal ("FHAC");
  eea->AddHCal ("BHAC");
#endif
  fRun->AddTask(eea);

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()
