
//
// Simplistic scheme: perform ideal hit-to-track association (this is Monte-Carlo, 
// so it is perfectly known which hit belogned to which track) and Kalman filter 
// tracking fit on digitized hits;
//

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
  //idealTracker->AddDetectorGroup("IPPT");
  idealTracker->AddDetectorGroup("B0TRACKER");
  //idealTracker->AddDetectorGroup("RP");
  // Add a bit of fairness to the reconstruction procedure; smear "ideal"
  // momenta by a few % relative before giving hit collection over to KF fitter;
  idealTracker->SetRelativeMomentumSmearing(0.10);
  // Also smear a bit "ideal" vertex;
  idealTracker->SetVertexSmearing(0.01, 0.01, 0.01);
  // Assume 100% hit association efficiency;
  idealTracker->SetTrackingEfficiency(1.);
  fRun->AddTask(idealTracker);

  // Invoke and configure PandaRoot Kalman filter code wrapper;
  fRun->AddTask(new EicRecoKalmanTask(idealTracker));

#if _LATER_
  EicHtcTask *htc = new EicHtcTask(idealTracker, TrKalmanFilter::WithField);  
  htc->SetTrackOutBranchName("EicIdealGenTrack");
  //+htc->SetMediaScanThetaPhi(0.022*180.0/TMath::Pi(), 0.0);
  htc->SetMediaScanThetaPhi(0.030*180.0/TMath::Pi(), 0.0);
  //htc->SetParticleHypothesis("proton", 275.0);
  htc->SetParticleHypothesis("proton", 100.0);
  fRun->AddTask(htc);
#endif

  // This call here just performs track backward propagation to the beam line; 
  fRun->AddTask(new PndPidCorrelator());

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()
