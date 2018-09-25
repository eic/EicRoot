
#include <simulation.C>

void digitization(unsigned seed = 0x12345678)
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");  
    
  // Create generic analysis run manager; configure it for digitization;
  EicRunAna *fRun = new EicRunAna();
  fRun->SetInputFile ("simulation.root");
  fRun->SetOutputFile("digitization.root");
  fRun->SetSeed(seed); 

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
    
  // "Ideal" TPC digitizer;
  EicTpcDigiHitProducer* tpcHitProducer = new EicTpcDigiHitProducer();  
  tpcHitProducer->importTpcDigiParameters("tpc-digi-250um.root");
  fRun->AddTask(tpcHitProducer);

  // So tune the parameters here once want to use these calorimeters; in fact one should better
  // play with fast simulation (fake MoCa points) first; add HCal stuff as well;
#ifdef _WITH_CALORIMETERS_
  {
    // Declare and configure a *standard* calorimeter task; name here should match
    // calorimeter names in calorimeter.C & simulation.C;
    EicCalorimeterDigiHitProducer* bemc = new EicCalorimeterDigiHitProducer("BEMC");

    // Crystals will be used as sensitive volumes; one can actually add a similar 
    // line to add alveoles to sensitive detector list (here it makes no sense of course);
    // FIXME: consider no Birk's correction here, add later;
    bemc->DeclareDigiSensitiveVolume("BemcCrystal");

    bemc->ConfigureDownstreamSensorGroup(CalorimeterSensorGroup::Reflection, 0.0);
    bemc->ConfigureUpstreamSensorGroup(CalorimeterSensorGroup::Sensor);
    bemc->SetSensorType(CalorimeterDigiParData::SiPM);
    
    // In [cm]; large value basically means "no attenuation" mode;
    bemc->SetAttenuationLength(0);

    bemc->SetCleanupThreshold(1);
  
    // Average number of photons which are supposed to be produced per GeV of incoming 
    // particle; in case of a crystal calorimeter with large attenuation length and
    // 100% registration efficiency (see settings above) this will be roughly the 
    // registered photon count; should be tuned to match typically measured one; 
    //bemc->SetLightYield(20000., 1.);
    bemc->SetPrimaryLightYield(20000.);
    
    // Put some realistic timing numbers here; times in [ns]; light velocity in [cm/ns];
    bemc->SetTimingGate(0., 100.);
    bemc->SetDecayConstant(6.5);
    bemc->SetLightPropagationVelocity(100./7);
    
    fRun->AddTask(bemc);
  }
  {
    // Declare and configure a *standard* calorimeter task; name here should match
    // calorimeter names in calorimeter.C & simulation.C;
    EicCalorimeterDigiHitProducer *femc = new EicCalorimeterDigiHitProducer("FEMC");

    // Crystals will be used as sensitive volumes; one can actually add a similar 
    // line to add alveoles to sensitive detector list (here it makes no sense of course);
    femc->DeclareDigiSensitiveVolumePrefix("FemcFiberCore", 12.6);
    
    // Number of "primary" photons is calculated based on expected (say, experimentally measured) 
    // light output per GeV of deposited energy; it is assumed that light can be collected on 
    // upstream and downstream ends of "long objects" (like typical calorimeter matrix crystals), 
    // so that attenuation length can be accounted; either end can be a 100% efficient sensor or 
    // a mirror with reflection in the range [0.0 .. 1.0]; sensor (and other types of) 
    // inefficiencies are accounted effectively via light output setting (see below); 
    femc->ConfigureDownstreamSensorGroup(CalorimeterSensorGroup::Reflection, 0.0);
    femc->ConfigureUpstreamSensorGroup(CalorimeterSensorGroup::Sensor);
    // Either qAPD or qSiPM; digitization options differ a bit (see below);
    femc->SetSensorType(CalorimeterDigiParData::SiPM);
    
    // In [cm]; large value basically means "no attenuation" mode;
    femc->SetAttenuationLength(150.);
    //femc->SetAttenuationLength(75.);
    
    // Energy deposits in all GEANT-sensitive volumes (say "caloCrystal" and "caloCellAlveole", 
    // as defined in calorimeter.C fle) will be calculated separately for inspection purposes;
    femc->RequestEnergyDepositAccounting(1.0, 1000);
    
    // Energy deposit plots can be constructed in time and Z ("long" crystal coordinate) bins
    // and dumped into ROOT output file;
    //femc->requestTimeSpectra(200., 200);
    femc->SetZBinning(20);
    
    // In [GeV] assuming known light yield (see below); this threshold is used just to 
    // remove very low energy cells; set to ~1MeV for now;
    //femc->setCleanupThreshold(1E-3);
    femc->SetCleanupThreshold(1);
    
    // APD-related section; see other sources (like PANDA EMC TDR) for exact meaning 
    // and typical values of ENF and ENC; 
    //femc->setApdGainFactor(50);
    //femc->setApdExcessNoiseFactor(1.);
    //femc->setApdEquivalentNoiseCharge(4200.);
    //
    // SiPM-related section;
    // Noise, in counts per [ns]; setTimingGate() setting matter (see below);
    //cemc->setSipmNoiseLevel(40E-3);
    
    // Average number of photons which are supposed to be registered per GeV of incoming 
    // particle; 2-d argument is expected sampling fraction (can be on a few percent
    // level for sampling fiber calorimeters; for crystal ones perhaps always close to 
    // 1.0); these settings are a bit misleading, in a sense that sampling fraction should 
    // be tuned *by hand* to get absolute normalization of reconstructed particle energy 
    // correct; reconstructed light yield should match the expectation (so if one side 
    // is a 70% efficient mirror and the other side is a sensor, this number should better
    // be renormalized by a factor of 0.85; even if attenuation length in very large); 
    // should perhaps think how to do it better later;
    //femc->setLightYield(130./((1.0+0.0)/2.0), 1.2E-2);
    //femc->SetPrimaryLightYield(5.2E4);
    femc->SetPrimaryLightYield(3.70E4);
    
    // Put some realistic timing numbers here; times in [ns]; light velocity in [cm/ns];
    femc->SetTimingGate(0., 100.);
    femc->SetDecayConstant(6.5);
    femc->SetLightPropagationVelocity(100./7);

    fRun->AddTask(femc);
  }
  {
    // Declare and configure a *standard* calorimeter task; name here should match
    // calorimeter names in calorimeter.C & simulation.C;
    EicCalorimeterDigiHitProducer *cemc = new EicCalorimeterDigiHitProducer("CEMC");

    // Crystals will be used as sensitive volumes; one can actually add a similar 
    // line to add alveoles to sensitive detector list (here it makes no sense of course);
    cemc->DeclareDigiSensitiveVolumePrefix("CemcFiberCore", 12.6);
    
    // Number of "primary" photons is calculated based on expected (say, experimentally measured) 
    // light output per GeV of deposited energy; it is assumed that light can be collected on 
    // upstream and downstream ends of "long objects" (like typical calorimeter matrix crystals), 
    // so that attenuation length can be accounted; either end can be a 100% efficient sensor or 
    // a mirror with reflection in the range [0.0 .. 1.0]; sensor (and other types of) 
    // inefficiencies are accounted effectively via light output setting (see below); 
    cemc->ConfigureDownstreamSensorGroup(CalorimeterSensorGroup::Reflection, 0.0);
    cemc->ConfigureUpstreamSensorGroup(CalorimeterSensorGroup::Sensor);
    // Either qAPD or qSiPM; digitization options differ a bit (see below);
    cemc->SetSensorType(CalorimeterDigiParData::SiPM);
    
    // In [cm]; large value basically means "no attenuation" mode;
    cemc->SetAttenuationLength(150.);
    //cemc->SetAttenuationLength(75.);
    
    // Energy deposits in all GEANT-sensitive volumes (say "caloCrystal" and "caloCellAlveole", 
    // as defined in calorimeter.C fle) will be calculated separately for inspection purposes;
    cemc->RequestEnergyDepositAccounting(1.0, 1000);
    
    // Energy deposit plots can be constructed in time and Z ("long" crystal coordinate) bins
    // and dumped into ROOT output file;
    //cemc->requestTimeSpectra(200., 200);
    cemc->SetZBinning(20);
    
    // In [GeV] assuming known light yield (see below); this threshold is used just to 
    // remove very low energy cells; set to ~1MeV for now;
    //cemc->setCleanupThreshold(1E-3);
    cemc->SetCleanupThreshold(1);
    
    // APD-related section; see other sources (like PANDA EMC TDR) for exact meaning 
    // and typical values of ENF and ENC; 
    //cemc->setApdGainFactor(50);
    //cemc->setApdExcessNoiseFactor(1.);
    //cemc->setApdEquivalentNoiseCharge(4200.);
    //
    // SiPM-related section;
    // Noise, in counts per [ns]; setTimingGate() setting matter (see below);
    //cemc->setSipmNoiseLevel(40E-3);
    
    // Average number of photons which are supposed to be registered per GeV of incoming 
    // particle; 2-d argument is expected sampling fraction (can be on a few percent
    // level for sampling fiber calorimeters; for crystal ones perhaps always close to 
    // 1.0); these settings are a bit misleading, in a sense that sampling fraction should 
    // be tuned *by hand* to get absolute normalization of reconstructed particle energy 
    // correct; reconstructed light yield should match the expectation (so if one side 
    // is a 70% efficient mirror and the other side is a sensor, this number should better
    // be renormalized by a factor of 0.85; even if attenuation length in very large); 
    // should perhaps think how to do it better later;
    //cemc->setLightYield(130./((1.0+0.0)/2.0), 1.2E-2);
    //cemc->SetPrimaryLightYield(5.2E4);
    cemc->SetPrimaryLightYield(3.70E4);
    
    // Put some realistic timing numbers here; times in [ns]; light velocity in [cm/ns];
    cemc->SetTimingGate(0., 100.);
    cemc->SetDecayConstant(6.5);
    cemc->SetLightPropagationVelocity(100./7);

    fRun->AddTask(cemc);
  }
#endif

  // Initialize and run digitization; exit at the end;
  fRun->Run();
} // digitization()
