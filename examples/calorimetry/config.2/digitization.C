
//
//  Converts MC points to hits in calorimeter cells; allows to account 
//  for light output per GeV of deposited energy, noise, attenuation length, etc.;
//

void digitization()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for digitization;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");

  // Declare and configure a *standard* calorimeter digitization task; 
  EicCalorimeterDigiHitProducer* calo = new EicCalorimeterDigiHitProducer("FHAC");
  // Scintillating plates will be used as sensitive volumes; 
  // 12.6 here is the Birk's constant for polystyrene;
  calo->DeclareDigiSensitiveVolume("FhacScintillatorPlate", 12.6);

  // Number of "primary" photons is calculated based on expected (say, experimentally measured) 
  // light output per GeV of deposited energy; it is assumed that light can be collected on 
  // upstream and downstream ends of "long objects" (like typical calorimeter matrix crystals), 
  // so that attenuation length can be accounted; either end can be a 100% efficient sensor or 
  // a mirror with reflection in the range [0.0 .. 1.0]; sensor (and other types of) 
  // inefficiencies are accounted effectively via light output setting (see below); 
  calo->ConfigureUpstreamSensorGroup(CalorimeterSensorGroup::Reflection, 0.0);
  calo->ConfigureDownstreamSensorGroup(CalorimeterSensorGroup::Sensor);
  // Either qAPD or qSiPM; digitization options differ a bit (see below);
  calo->SetSensorType(CalorimeterDigiParData::SiPM);

  // In [cm]; large value basically means "no attenuation" mode;
  calo->SetAttenuationLength(10000.);
  calo->SetZBinning(20);
  
  // In photons; this threshold is used just to remove very low energy cells; 
  calo->SetCleanupThreshold(1);
  
  // APD-related section; see other sources (like PANDA EMC TDR) for exact meaning 
  // and typical values of ENF and ENC; 
  //calo->setApdGainFactor(50);
  //calo->setApdExcessNoiseFactor(1.);
  //calo->setApdEquivalentNoiseCharge(4200.);
  //
  // SiPM-related section;
  // Noise, in counts per [ns]; setTimingGate() setting matter (see below);
  //cemc->setSipmNoiseLevel(40E-3);
  
  // For sampling calorimeters (like HCal in this example): average number of photons 
  // which are supposed to be produced per GeV of dE/dx energy deposited by shower particles 
  // (after Birk's correction) in the *sensitive* material, under assumption that 
  // *all* this light is collected by upstream and downstream sensors and there is 
  // *no* attenuation; the number should be tuned in such a way, that at given 
  // attenuation length and registration efficiency number of photons per GeV of 
  // incoming particle energy is roughly equal to the test-run measured one; this 
  // effectively takes into account sampling fraction for the defined geometry, 
  // capturing efficiency in the fibers (plates), etc; see respective 1D histogram 
  // in the reconstruction.root file (provided RequestLightYieldPlot() is called
  // in the reconstruction.C);
  calo->SetPrimaryLightYield(12000.);

  // Put some realistic timing numbers here; times in [ns]; light velocity in [cm/ns];
  calo->SetTimingGate(0., 100.);
  calo->SetDecayConstant(6.5);
  calo->SetLightPropagationVelocity(100./7);

  fRun->AddTask(calo);

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
