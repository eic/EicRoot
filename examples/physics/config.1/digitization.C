
void digitization()
{
  // Create generic analysis run manager; configure it for digitization;
  auto fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Use gaussian resolution matching 20x20um^2 pixel size;
  auto vstHitProducer = new EicTrackingDigiHitProducer("VST", EicDigiHitProducer::Smear);
  vstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(vstHitProducer); 

  auto fstHitProducer = new EicTrackingDigiHitProducer("FST", EicDigiHitProducer::Smear);
  fstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(fstHitProducer); 

  auto bstHitProducer = new EicTrackingDigiHitProducer("BST", EicDigiHitProducer::Smear);
  bstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(bstHitProducer); 

  // Assume 50um resolution in both X&Y-projections for the GEMs;
  auto fgtHitProducer = new EicTrackingDigiHitProducer("FGT", EicDigiHitProducer::Smear);
  fgtHitProducer->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(fgtHitProducer); 

  auto bgtHitProducer = new EicTrackingDigiHitProducer("BGT", EicDigiHitProducer::Smear);
  bgtHitProducer->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(bgtHitProducer); 
    
  // "Ideal" TPC digitizer (and medium-resolution device assumed);
  auto tpcHitProducer = new EicTpcDigiHitProducer();  
  tpcHitProducer->importTpcDigiParameters("tpc-digi-250um.root");
  fRun->AddTask(tpcHitProducer);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
