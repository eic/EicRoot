
#include <simulation.C>

void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for digitization;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  EicTrackingDigiHitProducer* vstHitProducer = new EicTrackingDigiHitProducer("VST", EicDigiHitProducer::Smear);
  vstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(vstHitProducer); 
  EicTrackingDigiHitProducer* fstHitProducer = new EicTrackingDigiHitProducer("FST", EicDigiHitProducer::Smear);
  fstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(fstHitProducer); 
  EicTrackingDigiHitProducer* bstHitProducer = new EicTrackingDigiHitProducer("BST", EicDigiHitProducer::Smear);
  bstHitProducer->DefineKfNodeTemplateXY(0.00058, 0.00058);
  fRun->AddTask(bstHitProducer); 

  EicTrackingDigiHitProducer* fgtHitProducer = new EicTrackingDigiHitProducer("FGT", EicDigiHitProducer::Smear);
  fgtHitProducer->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(fgtHitProducer); 
  EicTrackingDigiHitProducer* bgtHitProducer = new EicTrackingDigiHitProducer("BGT", EicDigiHitProducer::Smear);
  bgtHitProducer->DefineKfNodeTemplateXY(0.0050, 0.0050);
  fRun->AddTask(bgtHitProducer); 

#ifdef _WITH_MUMEGAS_
  //EicTrackingDigiHitProducer* mmgHitProducer = new EicTrackingDigiHitProducer("MMT", EicDigiHitProducer::Smear);
  //mmgHitProducer->DefineKfNodeTemplateAxial3D(0.1400, 0.0100);
  //fRun->AddTask(mmgHitProducer); 
#endif

  // "Ideal" TPC digitizer;
  EicTpcDigiHitProducer* tpcHitProducer = new EicTpcDigiHitProducer();  
  // So assume ~250um single-point resolution at the CM (and ~200um at the readout plane); 
  tpcHitProducer->importTpcDigiParameters("tpc-digi-250um.root");
  tpcHitProducer->Print();
  fRun->AddTask(tpcHitProducer);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()

