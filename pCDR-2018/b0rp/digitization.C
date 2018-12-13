
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

  EicTrackingDigiHitProducer *ippt = 
    new EicTrackingDigiHitProducer("IPPT", EicDigiHitProducer::Smear);
  // pCDR table 3.3, 275x18 GeV, first column;
  ippt->DefineKfNodeTemplateXY(100 * eic::um, 20 * eic::um);
  //ippt->DefineKfNodeTemplateXY(10 * eic::mm, 10 * eic::mm);
  fRun->AddTask(ippt);

  EicTrackingDigiHitProducer *b0tracker = 
    new EicTrackingDigiHitProducer("B0TRACKER", EicDigiHitProducer::Smear);
  b0tracker->DefineKfNodeTemplateXY(20 * eic::um, 20 * eic::um);
  fRun->AddTask(b0tracker);

  EicTrackingDigiHitProducer *rp = 
    new EicTrackingDigiHitProducer("RP", EicDigiHitProducer::Smear);
  rp->DefineKfNodeTemplateXY(20 * eic::um, 20 * eic::um);
  fRun->AddTask(rp);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
