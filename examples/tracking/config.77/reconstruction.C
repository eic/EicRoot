
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
  // Default in this example is to use "FWDST" detector hits; more than 
  // one custom detector may be added here via AddDetectorGroup() call(s);
  idealTracker->AddDetectorGroup("FWDST");
  // Add a bit of fairness to the reconstruction procedure; smear "ideal"
  // momenta by 10% relative before giving hit collection over to KF fitter;
  idealTracker->SetRelativeMomentumSmearing(0.1);
  // Also smear a bit "ideal" vertex;
  idealTracker->SetVertexSmearing(100 * eic::um, 100 * eic::um, 100 * eic::um);
  // Assume 100% hit association efficiency;
  idealTracker->SetTrackingEfficiency(1.);
  fRun->AddTask(idealTracker);

  // Invoke and configure PandaRoot Kalman filter code wrapper;
  fRun->AddTask(new EicRecoKalmanTask(idealTracker));

  // This call here just performs track backward propagation to the beam line; 
  fRun->AddTask(new PndPidCorrelator());

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()
