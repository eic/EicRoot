
//
// Perform clustering of digitized file;
//

void reconstruction()
{
  // Create generic analysis run manager; configure it for calorimeter reconstruction;
  auto fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->AddFriend    ("digitization.root");
  fRun->SetOutputFile("reconstruction.root");

  // Declare custom calorimeter reconstruction; detector choice is done by 
  // name encoded in calorimeter.root file;
  auto calo = new EicCalorimeterReconstruction("FHAC");
  // 1-st: cluster seed threshold (cells below that can not initiate clustering);
  // 2-d : neighbour search threshold (cells below that can not initiate neighbour
  //       attachment for the cluster they are associated with);
  // 3-d : cell cutoff threshold (cells below that are ignored);
  calo->SetClusterAlgorithmThresholds(0.3, 0.03, 0.005);
  // This conversion factor is basically *measured* in the test experiment; to be 
  // specific, one observed roughly 130 p.e. per GeV of incoming hadron particle
  // energy; value of 12000 in digitization.C SetPrimaryLightYield() call is then
  // tuned in such a way to really provide a correct energy scale, given specified
  // (relative) efficiency of the sensors and the attenuation length;
  calo->SetPhotonToEnergyConversionFactor(1/130.);
  // 1E5 photons scale for the plot;
  calo->RequestLightYieldPlot(10000);
  fRun->AddTask(calo);

  // Initialize and run the reconstruction; exit at the end;
  fRun->Run();
} // reconstruction()
