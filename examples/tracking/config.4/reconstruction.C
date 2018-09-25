
void reconstruction()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for track reconstruction;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->AddFriend    ("digitization.root");
  fRun->SetOutputFile("reconstruction.root");

  // Call "ideal" hit-to-track associator routine; 
  EicIdealTrackingCode* idealTracker = new EicIdealTrackingCode();
  idealTracker->AddDetectorGroup("FWDGT");
  // Add a bit of fairness to the reconstruction procedure; smear "ideal"
  // momenta by 10% relative before giving hit collection over to KF fitter;
  idealTracker->SetRelativeMomentumSmearing(0.1);
  // Also smear a bit "ideal" vertex;
  idealTracker->SetVertexSmearing(0.01, 0.01, 0.01);
  fRun->AddTask(idealTracker);

  // Invoke and configure PandaRoot Kalman filter code wrapper;
  fRun->AddTask(new EicRecoKalmanTask(idealTracker));

  // This call here just performs track backward propagation to the beam line; 
  fRun->AddTask(new PndPidCorrelator());

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run(); 
} // reconstruction()
