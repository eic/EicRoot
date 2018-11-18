
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

  EicTrackingDigiHitProducer *lqst = 
    new EicTrackingDigiHitProducer("LQST", EicDigiHitProducer::Smear);
  lqst->DefineKfNodeTemplateXY(20 * eic::um, 20 * eic::um);
  fRun->AddTask(lqst);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
