
void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
  
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Call standard digitizing routine for our FWDST detector; assume 20x20um^2 pixels;
  // NB: may add more EicTrackingDigiHitProducer routines here if want to use your 
  // own custom set of detectors; 
  EicTrackingDigiHitProducer *fwdst = 
    new EicTrackingDigiHitProducer("FWDGT", EicDigiHitProducer::Smear);
  fwdst->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(fwdst);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
