
//
//  Converts MC points to hits in calorimeter cells; allows to account 
//  for light output per GeV of deposited energy, noise, attenuation length, etc.;
//

void digitization()
{
  // Create generic analysis run manager; configure it for digitization;
  auto fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Declare and configure a *standard* calorimeter digitization task; detector name here 
  // should match calorimeter names in calorimeter.C & simulation.C;
  auto calo = new EicCalorimeterDigiHitProducer("CALORIMETER");

  // Crystals will be used as sensitive volumes; one can actually add a similar 
  // line to add alveoles to sensitive detector list (here it makes no sense of course);
  // FIXME: consider no Birk's correction here, add later;
  calo->DeclareDigiSensitiveVolume("CaloCrystal");
  //calo->DeclareDigiSensitiveVolume("CaloCellAlveole");

  // Number of "primary" photons is calculated based on expected (say, experimentally measured) 
  // light output per GeV of deposited energy; it is assumed that light can be collected on 
  // upstream and downstream ends of "long objects" (like typical calorimeter matrix crystals), 
  // so that attenuation length can be accounted; either end can be a 100% efficient sensor or 
  // a mirror with reflection in the range [0.0 .. 1.0]; sensor (and other types of) 
  // inefficiencies are accounted effectively via light output setting (see below); 
  calo->ConfigureUpstreamSensorGroup  (CalorimeterSensorGroup::Reflection, 1.0);
  calo->ConfigureDownstreamSensorGroup(CalorimeterSensorGroup::Sensor);
  // Either qAPD or qSiPM; digitization options differ a bit (see below);
  calo->SetSensorType(CalorimeterDigiParData::APD);

  // In [cm]; 0 or large value basically means "no attenuation" mode;
  calo->SetAttenuationLength(0);

  // Energy deposits in all GEANT-sensitive volumes (say "CaloCrystal" and "CaloCellAlveole", 
  // as defined in calorimeter.C file) will be calculated separately for inspection purposes;
  calo->RequestEnergyDepositAccounting();

  // Energy deposit plots can be constructed in time and Z ("long" crystal coordinate) bins
  // and dumped into ROOT output file;
  //calo->RequestTimeSpectra(200., 200);
  //calo->SetZCoordBinning(20);
  
  // In photons; this threshold is used just to remove very low energy cells; 
  calo->SetCleanupThreshold(1);
  
  // APD-related section; see other sources (like PANDA EMC TDR) for exact meaning 
  // and typical values of ENF and ENC; 
  //calo->SetApdGainFactor(50);
  //calo->SetApdExcessNoiseFactor(1.);
  //calo->SetApdEquivalentNoiseCharge(4200.);
  //
  // SiPM-related section;
  // Noise, in counts per [ns]; setTimingGate() setting matter (see below);
  //cemc->SetSipmNoiseLevel(40E-3);
  
  // Average number of photons which are supposed to be produced per GeV of dE/dx energy
  // deposited by incoming particle; in case of a crystal calorimeter with large attenuation 
  // length and 100% registration efficiency at upstream and downstream sensor ends (see 
  // settings above) this will per definition be roughly the registered photon count for 
  // 1 GeV energy electron; the number should be tuned to match typically *measured* one; 
  calo->SetPrimaryLightYield(20000.);

  // Put some realistic timing numbers here; times in [ns]; light velocity in [cm/ns];
  calo->SetTimingGate(0., 100.);
  calo->SetDecayConstant(6.5);
  calo->SetLightPropagationVelocity(100./7);

  fRun->AddTask(calo);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
