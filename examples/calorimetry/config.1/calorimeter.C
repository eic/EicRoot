
//
//  Example NxN crystal calorimeter creation script; produces "calorimeter.root"
//  file with a 7x7 crystal matrix of specified size, at a given location; 
//  can be used as geometry input for simulation.C->digitization.C->reconstruction.C 
//  scripts in this directory; "calorimeter.root" will also contain a mapping table
//  needed to perform clustering in reconstruction.C;
//
//  Prefer to declare dimensions in [mm]; convert to [cm] when calling ROOT shape 
//  definition routines only;
//

void calorimeter()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Internals, in particular all the famous FairRoot dances around media interface, are 
  // of no interest for end user here; just hide them all in this class; there are certain naming 
  // conventions inside EicRoot framework; selected here (more or less arbitrary) detector name will 
  // be used in various I/O routines in a "mixed" writing ('Calorimeter' in this particular case); 
  // mapping table block will be called CalorimeterGeoParData (you may want to inspect calorimeter.root 
  // file), raw MC points will be called CalorimeterMoCaPoint, etc.; string ('calorimeter' in this case) 
  // and capital letters ('CALORIMETER' here) writings will be mostly used for printouts;
  EndcapGeoParData *calo = new EndcapGeoParData("CALORIMETER");

  // Create a 7x7 matrix; 25x25mm^2 square cells, 200mm long, enclosed in 200um thick 
  // carbon fiber alveoles; 
  UInt_t cellNum              = 7;
  Double_t cellFaceSize       =  25.00;
  Double_t cellLength         = 200.0;
  Double_t alveoleThickness   =   0.2;
  // Yes, prefer to associate carbon material with its particular cell;
  Double_t cellEnvelopeWidth  = cellFaceSize + 2*alveoleThickness;
  // Assume wrapper volume thickness is the same on all sides;
  Double_t cellEnvelopeLength = cellLength   + 2*alveoleThickness;

  // Choose whatever distance from the IP; calorimeter matrix will be 
  // "ortogonal" to Z (beam) direction;
  Double_t beamLineOffset     = 2000.0;

  calo->SetTopVolumeTransformation(new TGeoTranslation(0.0, 0.0, 0.1 * beamLineOffset));

  // Let mapper know overall (logical 2D) matrix size; these lines are required 
  // for proper clustering in reconstruction.C;
  unsigned lgroup = calo->AddLogicalVolumeGroup(cellNum, cellNum);

  // Declare single cell pack volume; volume names are arbitrary, but should be 
  // consistent here and through simulation/digitization/reconstruction sequence;
  TGeoBBox *alveole = new TGeoBBox("CaloCellAlveole", 
				   0.1 * cellEnvelopeWidth/2,
				   0.1 * cellEnvelopeWidth/2,
				   0.1 * cellEnvelopeLength/2);
  // Make sure media names are listed in geometry/media.geo;
  TGeoVolume *valveole = new TGeoVolume("CaloCellAlveole", alveole, calo->GetMedium("CarbonFiber"));

  // Crystal volume inside this pack;
  TGeoBBox *crystal = new TGeoBBox("CaloCrystal", 
				   0.1 * cellFaceSize/2,
				   0.1 * cellFaceSize/2,
				   0.1 * cellLength/2);
  TGeoVolume *vcrystal = new TGeoVolume("CaloCrystal", crystal, calo->GetMedium("pwo"));

  // Place crystal into the alveole;
  valveole->AddNode(vcrystal, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));

  //
  // Mapping table will contain 2 separate sections: crystals and their respective alveoles;
  // it hardcodes one-to-one correspondence between GEANT crystal nodes in the geometry 
  // volume hierarchy and XY-indices of a given crystal in calorimeter matrix;
  //

  // Crystal (sensitive volume) map; 2 levels deep (crystal itself and its alveole);
  EicGeoMap *gmapCr = calo->CreateNewMap();
  // At most 1 crystal per alveole and N*N alveole packs;
  gmapCr->AddGeantVolumeLevel("CaloCrystal",                     1);
  gmapCr->AddGeantVolumeLevel("CaloCellAlveole", cellNum * cellNum);
  gmapCr->SetSingleSensorContainerVolume("CaloCellAlveole");

  // Perhaps want to make alveoles GEANT sensitive volumes as well (say, to check 
  // energy losses there) -> create a separate map; 
  EicGeoMap *gmapAl = calo->CreateNewMap();
  // N*N alveole packs;
  gmapAl->AddGeantVolumeLevel("CaloCellAlveole", cellNum * cellNum);
  gmapAl->SetSingleSensorContainerVolume("CaloCellAlveole");

  // Now place all N*N alveole packs into the calorimeter volume and create mapping tables;
  // this variable will serve as a running node counter;
  unsigned tcounter = 0;
  for(unsigned ix=0; ix<cellNum; ix++)
  {
    double xx = cellEnvelopeWidth*(ix - (cellNum-1)/2.);

    for(unsigned iy=0; iy<cellNum; iy++)
    {
      double yy = cellEnvelopeWidth*(iy - (cellNum-1)/2.);

      // Fill both crystal and alveole mapping table entries; geometry is very simple
      // in this case (just a single NxN matrix) -> linear indices are {ix,iy}; 
      UInt_t id[2] = {0, tcounter}, logical[2] = {ix, iy};
      if (calo->SetMappingTableEntry(gmapCr, id+0, lgroup, logical) ||
	  calo->SetMappingTableEntry(gmapAl, id+1, lgroup, logical)) {
	cout << "Failed to set mapping table entry!" << endl;
	exit(0);
      } //if

      // Well, in this easy case (a single NxN matrix) one could probably use XY-index
      // as node ID in the below call and be happy; the problem is that this simple 
      // scheme does not work for real-life configurations (for instance if calorimeter 
      // was composed of one 2x5 and one 3x5 matrix); then one has to go through a nasty 
      // excercise of associating logical node IDs and XY-indices on the calorimeter matrix
      // anyway, and the best place to do the job is right here, during GEANT geometry 
      // creation (so logical mapping will be always available together with the GEANT volume 
      // scheme); see geometry/BEMC/bemc.C, which cooks such a file for EIC backward electromagnetic
      // calorimeter (4 quadrants filled with 2x2 crystal packs, mapped onto a single 
      // "flat" XY per-crystal matrix);
      calo->GetTopVolume()->AddNode(valveole, tcounter++, 
				    new TGeoCombiTrans(0.1 * xx, 0.1 * yy, 0.0, new TGeoRotation()));
    } //for iy
  } //for ix

  calo->GetColorTable()->AddExactMatch       ("CaloCrystal", kBlue);
  calo->GetTransparencyTable()->AddExactMatch("CaloCrystal", 50);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into ROOT TGeo volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  calo->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // calorimeter()

