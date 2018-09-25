
//
// Perform clustering of digitized file;
//

void reconstruction()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for calorimeter reconstruction;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->AddFriend    ("digitization.root");
  fRun->SetOutputFile("reconstruction.root");

  // Declare *standard* calorimeter reconstruction; 
  EicCalorimeterReconstruction *calo = new EicCalorimeterReconstruction("CALORIMETER");
  // 1-st: cluster seed threshold (cells below that can not initiate clustering);
  // 2-d : neighbour search threshold (cells below that can not initiate neighbour
  //       attachment for the cluster they are associated with);
  // 3-d : cell cutoff threshold (cells below that are ignored);
  calo->SetClusterAlgorithmThresholds(0.3, 0.03, 0.005);
  // Well, sampling fraction is ~100% here, except for the losses in alveoles,
  // leaks and unattached hits; those contribute a lot however -> need to apply 
  // lower primary light yield than used in digitization.C in order to match 1 GeV energy;
  calo->SetPhotonToEnergyConversionFactor(1/18100.);
  // 1E5 photons scale for the plot;
  calo->RequestLightYieldPlot(100000);
  fRun->AddTask(calo);

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()
