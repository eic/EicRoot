
void reconstruction()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for track reconstruction;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->AddFriend    ("digitization.root");
  fRun->SetOutputFile("reconstruction.root");

  // Invoke and configure "ideal" PandaRoot tracking code wrapper; 
  EicIdealTrackingCode* idealTracker = new EicIdealTrackingCode();
  idealTracker->AddDetectorGroup("FST");
  idealTracker->AddDetectorGroup("VST");
  idealTracker->AddDetectorGroup("FGT");
  idealTracker->AddDetectorGroup("FFG");
  idealTracker->AddDetectorGroup("TPC");
  idealTracker->AddDetectorGroup("RICH");
  idealTracker->SetRelativeMomentumSmearing(0.1);
  idealTracker->SetVertexSmearing(0.01, 0.01, 0.01);
  fRun->AddTask(idealTracker);

  // Invoke and configure PandaRoot Kalman filter code wrapper;
  EicRecoKalmanTask *kftask = new EicRecoKalmanTask(idealTracker);
  kftask->StoreTrackParameterization();
  fRun->AddTask(kftask);

  // This call here just performs track backward propagation to the beam line; 
  fRun->AddTask(new PndPidCorrelator());

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()

