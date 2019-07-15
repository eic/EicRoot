
void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for digitization;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Assume 20um pixel MAPS sensors -> 20um/sqrt(12) ~ 5.8um gaussian "equivalent";
  EicTrackingDigiHitProducer* vstHitProducer = new EicTrackingDigiHitProducer("VST", EicDigiHitProducer::Smear);
  vstHitProducer->DefineKfNodeTemplateXY(5.8 * eic::um, 5.8 * eic::um);
  fRun->AddTask(vstHitProducer); 
  EicTrackingDigiHitProducer* fstHitProducer = new EicTrackingDigiHitProducer("FST", EicDigiHitProducer::Smear);
  fstHitProducer->DefineKfNodeTemplateXY(5.8 * eic::um, 5.8 * eic::um);
  fRun->AddTask(fstHitProducer); 

  // Front GEM disks;
  EicTrackingDigiHitProducer* fgtHitProducer = new EicTrackingDigiHitProducer("FGT", EicDigiHitProducer::Smear);
  fgtHitProducer->DefineKfNodeTemplateXY(50 * eic::um, 50 * eic::um);
  fRun->AddTask(fgtHitProducer);
  // Rear GEM disks;
  EicTrackingDigiHitProducer* ffgHitProducer = new EicTrackingDigiHitProducer("FFG", EicDigiHitProducer::Smear);
  ffgHitProducer->DefineKfNodeTemplateXY(100 * eic::um, 100 * eic::um);
  fRun->AddTask(ffgHitProducer); 

  // RICH profiler;
  EicTrackingDigiHitProducer* richHitProducer = new EicTrackingDigiHitProducer("RICH", EicDigiHitProducer::Smear);
  // Assign very big values, so that tracking resolution is not affected; 
  richHitProducer->DefineKfNodeTemplateXY(1 * eic::cm, 1 * eic::cm);
  fRun->AddTask(richHitProducer); 
  
  // "Ideal" TPC digitizer;
  EicTpcDigiHitProducer* tpcHitProducer = new EicTpcDigiHitProducer();  
  // So assume ~250um single-point resolution at the CM (and ~200um at the readout plane); 
  tpcHitProducer->importTpcDigiParameters("tpc-digi-250um.root");
  tpcHitProducer->Print();
  fRun->AddTask(tpcHitProducer); 

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()

