
//
//  Converts MC points to hits in silicon layers; cell-type and gaussian 
// smearing possible; "ideal" model (one hit per MC point);
//

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
    new EicTrackingDigiHitProducer("FWDST", EicDigiHitProducer::Quantize);
  fwdst->DefineKfNodeTemplateXY(20 * eic::um, 20 * eic::um);
    //new EicTrackingDigiHitProducer("FWDST", EicDigiHitProducer::Smear);
    //fwdst->DefineKfNodeTemplateXY(6 * eic::um, 6 * eic::um);
  fRun->AddTask(fwdst);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
