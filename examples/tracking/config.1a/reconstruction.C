
//
// Track reconstruction: 1) no track finder, namely use "ideal" hit-to-track 
// association, 2) run "real" Kalman filter (KF) fit on digitized hits;
//

void reconstruction()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Create generic analysis run manager; configure it for track reconstruction;
  EicRunAna *fRun = new EicRunAna();
  // Import the files with simulated events and digitized hits;
  fRun->SetInputFile ("simulation.root");
  fRun->AddFriend    ("digitization.root");
  // Define the output file with the reconstructed tracks;
  fRun->SetOutputFile("reconstruction.root");

  // Call "ideal" hit-to-track associator ("track finder") routine; 
  EicIdealTrackingCode* idealTracker = new EicIdealTrackingCode();
  // Default in this example is to use "FWDST" detector hits; more than 
  // one custom detector may be added here via AddDetectorGroup() call(s);
  idealTracker->AddDetectorGroup("FWDST");
  // Add a bit of fairness to the reconstruction procedure; smear "ideal"
  // momenta by 10% (relative) before passing the hit collection over to the KF fitter;
  idealTracker->SetRelativeMomentumSmearing(0.1);
  // Also smear a bit the "ideal" vertex;
  idealTracker->SetVertexSmearing(100 * eic::um, 100 * eic::um, 100 * eic::um);
  // Assume 100% hit association efficiency;
  idealTracker->SetTrackingEfficiency(1.0);
  fRun->AddTask(idealTracker);

  // Invoke and configure PandaRoot Kalman filter code wrapper; use hit-to-track
  // association of the detectors included via idealTracker->AddDetectorGroup() calls
  // during the "ideal track finder" call above;
  EicRecoKalmanTask *kftask = new EicRecoKalmanTask(idealTracker);
  // Store Monte-Carlo (truth) and reconstructed track parameterizations at 
  // every detector plane, where a particular track produced hits; if other 
  // locations along the track are of interest, thin additional "profiling" planes
  // need to be adde to the setup; one can consider to provide interpolation and
  // extrapolation tools right in the analysis.C script, but as of now (02/04/2020)
  // this feature is not implemented;
  kftask->StoreTrackParameterization();
  fRun->AddTask(kftask);

  // Perform track backward propagation to the beam line; 
  fRun->AddTask(new PndPidCorrelator());

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()
