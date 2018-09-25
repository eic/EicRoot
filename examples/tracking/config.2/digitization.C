
// Needed to figure out whether _USE_BEAST_TPC_INTERFACE_ is defined;
#include "tpc-builder.C"

void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for digitization;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Vertex detector digitizer; assume 20x40um^2 pixels; 
  EicTrackingDigiHitProducer *vtxHitProducer = new EicTrackingDigiHitProducer("VTX", EicDigiHitProducer::Quantize);
  vtxHitProducer->DefineKfNodeTemplateXY(0.0020, 0.0040);
  fRun->AddTask(vtxHitProducer); 

#ifdef _USE_BEAST_TPC_DIGITIZER_
  // BeAST TPC digitizer; still naive, but certainly suffices for resolution studies;
  EicTpcDigiHitProducer* tpcHitProducer = new EicTpcDigiHitProducer();  
  // Import default medium-resolution TPC parameters and print them out;
  tpcHitProducer->importTpcDigiParameters("tpc-digi.root");
  tpcHitProducer->Print(); 

  // May want to change any of them here; "intrinsic" resolutions are those at 
  // the GEM pad plane (D=0, see below) in [um]; dispersion coefficients are in [um/sqrt(D)] 
  // (where D is drift distance in [cm]), their contributions are added in quadrature;
  //tpcHitProducer->setTransverseDispersion           ( 50.);
  //tpcHitProducer->setLongitudinalDispersion         (200.);
  //tpcHitProducer->setTransverseIntrinsicResolution  (100.);
  //tpcHitProducer->setLongitudinalIntrinsicResolution(500.);
  // This one defines how many hits will be generated;
  //tpcHitProducer->setGemVerticalPadSize             ( 1.0);
#else
  // Very naive TPC digitizer; assume 1 hit per MC step, 100um (transverse) 
  // and 500um (longitudinal) resolution per hit;
  EicTrackingDigiHitProducer *tpcHitProducer = new EicTrackingDigiHitProducer("TPC", EicDigiHitProducer::Smear);
  tpcHitProducer->DefineKfNodeTemplateOrth3D(0.0100, 0.0100, 0.0500);
#endif
  fRun->AddTask(tpcHitProducer); 

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
