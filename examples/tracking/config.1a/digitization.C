
//
//  This script converts MC points to hits in the silicon layers; both discrete
// and gaussian smearing possible; "ideal" model (one hit per MC point);
//

void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
  
  // Create generic analysis run manager; configure it for MC hit digitization;
  EicRunAna *fRun = new EicRunAna();
  // Import the file with simulated events;
  fRun->SetInputFile ("simulation.root");
  // Define the output file with the digitized hits;
  fRun->SetOutputFile("digitization.root");

  // Call standard digitizing routine for our FWDST detector; assume 20x20um^2 
  // discrete pixels; may add more EicTrackingDigiHitProducer routines here 
  // if want to use your own custom set of detectors described a la tracker.C and
  // included in the simulation.C script;
  EicTrackingDigiHitProducer *fwdst = 
    new EicTrackingDigiHitProducer("FWDST", EicDigiHitProducer::Quantize);
  fwdst->DefineKfNodeTemplateXY(20 * eic::um, 20 * eic::um);
  // Commented out: gaussian smearing with sigma ~ 20um/sqrt(12) is expected 
  // to produce similar results;
  //new EicTrackingDigiHitProducer("FWDST", EicDigiHitProducer::Smear);
  //fwdst->DefineKfNodeTemplateXY(6 * eic::um, 6 * eic::um);
  fRun->AddTask(fwdst);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
