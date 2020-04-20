
//
//  Crystal calorimeter simulation script; 
//

void simulation(unsigned nEvents = 1000)
{
  // Create simulation run manager; use GEANT4 here;
  auto fRun = new EicRunSim("TGeant4");
  fRun->SetOutputFile("simulation.root");

  // "CALORIMETER" name here (case-insensitive) should match the respective name 
  // in calorimeter.C script used to create the detector; one can actually 
  // create more than one calorimeter this way, and as long as their names differ
  // (and physical locations do not overlap), all the simulation/digitization/reconstruction 
  // scheme will work (clustering scheme will be limited to a given calorimeter though);
  auto calo = new EicCalorimeter("CALORIMETER", (char*)"./calorimeter.root", qDUMMY);
  // Declare both crystals and alveoles as GEANT sensitive volumes if want to 
  // check energy deposit leak into alveole material; volumes which will actually 
  // participate in clustering algorithm may be selected later in digitization script;
  calo->DeclareGeantSensitiveVolume("CaloCrystal");
  //calo->DeclareGeantSensitiveVolume("CaloCellAlveole");
  fRun->AddModule(calo);

  // Create and set up (Box) Event Generator;
  {
    int      PDG = 11;                   // electron
    double  momentum = 1.0, theta = 1.0; // 1 GeV/c @ 1 degree

    auto boxGen = new EicBoxGenerator(PDG);
    boxGen->SetMomentum(momentum);
    boxGen->SetTheta(theta);

    fRun->AddGenerator(boxGen);
  }

  // Initialize and run the simulation; exit at the end;
  fRun->Init();
  // 10um shower particle range must be Ok for crystal calorimeter?;
  ((TGeant4*)gMC)->ProcessGeantCommand("/mcPhysics/rangeCuts 0.01 mm");
  fRun->Run(nEvents);
} // simulation()
