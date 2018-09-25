
void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for digitization;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Use gaussian resolution matching 20x20um^2 pixel size;
  EicTrackingDigiHitProducer* vstHitProducer = new EicTrackingDigiHitProducer("VST", EicDigiHitProducer::Smear);
  vstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(vstHitProducer); 

  EicTrackingDigiHitProducer* fstHitProducer = new EicTrackingDigiHitProducer("FST", EicDigiHitProducer::Smear);
  fstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(fstHitProducer); 

  EicTrackingDigiHitProducer* bstHitProducer = new EicTrackingDigiHitProducer("BST", EicDigiHitProducer::Smear);
  bstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(bstHitProducer); 

  // Assume 50um resolution in both X&Y-projections for the GEMs;
  EicTrackingDigiHitProducer* fgtHitProducer = new EicTrackingDigiHitProducer("FGT", EicDigiHitProducer::Smear);
  fgtHitProducer->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(fgtHitProducer); 

  EicTrackingDigiHitProducer* bgtHitProducer = new EicTrackingDigiHitProducer("BGT", EicDigiHitProducer::Smear);
  bgtHitProducer->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(bgtHitProducer); 
    
  // "Ideal" TPC digitizer (and medium-resolution device assumed);
  EicTpcDigiHitProducer* tpcHitProducer = new EicTpcDigiHitProducer();  
  tpcHitProducer->importTpcDigiParameters("tpc-digi-250um.root");
  fRun->AddTask(tpcHitProducer);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
