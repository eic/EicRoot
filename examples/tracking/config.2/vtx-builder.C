
// Number of barrel layers;
#define _LAYER_NUM_  4

// Used volume names;
#define _STAVE_NAME_ "VtxStave"
#define _CHIP_NAME_  "VtxChip"

void vtx_builder()
{
  // Detector name will be "VTX"; should be consistent through 
  // all the simulation.C->digitization.C->reconstruction.C chain;
  auto vtx = new EicGeoParData("VTX");

  // Hardcode TGeo file name (no versioning, etc);
  vtx->SetFileName("vtx.root");

  //
  //   Hardcode all the geometry parameters for simplicity; NB: for most part of 
  // EicRoot detector types respective variables (like chip dimensions below) 
  // are EicGeoParData inherited class variables which get saved in the 
  // output file containing TGeo information; units are [cm] everywhere;
  //

  // Layer installation radii, number of staves per layer; 
  double     radius[_LAYER_NUM_] = { 4.0, 6.0, 8.0, 10.0};
  unsigned staveNum[_LAYER_NUM_] = {  26,  39,  52,   65};
  unsigned staveNumMax = staveNum[_LAYER_NUM_-1], staveNumSum;

  // Silicon chip size;
  double chipWidth               =  1.000;
  double chipLength              =  2.000;
  double chipThickness           =  0.002;

  int chipsPerStave              = 10;

  // Stave size; take more or less realistic fill factor;
  double staveWidth              = chipWidth + 0.100;
  double staveLength             = chipsPerStave*(chipLength + 0.010);
  // Will yield ~0.3% rad.length per layer (a la ALICE MAPS);
  double staveThickness          =  0.030;

  // Assume can live with the same extra Z-slope angle (in degrees) for all layers;
  double staveSlope              = 10.0;

  // Create stave volume; use artificial carbon-like material with X0=10cm; 
  auto stave = new TGeoBBox(_STAVE_NAME_, staveWidth/2, staveLength/2, staveThickness/2);
  auto vstave = new TGeoVolume(_STAVE_NAME_, stave, vtx->GetMedium("X0=10cm"));

  // Create chip volume; 
  auto chip = new TGeoBBox(_CHIP_NAME_, chipWidth/2, chipLength/2, chipThickness/2);
  auto vchip = new TGeoVolume(_CHIP_NAME_, chip, vtx->GetMedium("silicon"));

  // Place chips into stave volume;
  {
    double step = chipLength + (staveLength - chipsPerStave*chipLength)/chipsPerStave;
    for(int ichip=0; ichip<chipsPerStave; ichip++) { 
      double yOffset = (ichip - (chipsPerStave-1)/2.0)*step;
      
      vstave->AddNode(vchip, ichip, new TGeoCombiTrans(0.0, yOffset, 0.0, 0));    
    } //for ichip
  }    

  // Running variable to count stave copies placed into the MASTER volume;
  unsigned staveCounter = 0;

  // Let it be a single logical group (all the staves are identical in all layers);
  // ~3D indices for all chips are [stave,layer,chip] per convention;
  unsigned group = vtx->AddLogicalVolumeGroup(_LAYER_NUM_, staveNumMax, chipsPerStave);

  // Calculate total number of staves;
  for(unsigned ilayer=0; ilayer<_LAYER_NUM_; ilayer++) 
    staveNumSum += staveNum[ilayer];

  // And then there is a single map needed which encodes GEANT->logical id conversion;
  auto xmap = vtx->CreateNewMap();
  // Volumes are defined in this order (from innermost to upmost); specify max expected 
  // copy number on every level; innermost volumes are assumed sensitive (in GEANT sense)
  // per default in the simulation.C script;
  xmap->AddGeantVolumeLevel(_CHIP_NAME_,  chipsPerStave);
  xmap->AddGeantVolumeLevel(_STAVE_NAME_, staveNumSum);
  xmap->SetSingleSensorContainerVolume(_CHIP_NAME_);

  // Loop through all barrel layers;
  for(unsigned ilayer=0; ilayer<_LAYER_NUM_; ilayer++) {
    // Loop through all the staves in this layer;
    for(unsigned istave=0; istave<staveNum[ilayer]; istave++) {
      // Loop through all chips in this stave and add mapping table entry for each of them;
      for(int ichip=0; ichip<chipsPerStave; ichip++) { 
	// So GEANT indices (copy numbers) [chip,stave] are mapped onto 3D logical 
	// indices [layer,stave,chip]; 
	UInt_t geant[2] = {ichip, staveCounter}, logical[3] = {ilayer, istave, ichip};

	if (vtx->SetMappingTableEntry(xmap, geant, group, logical)) {
	  cout << "Failed to set mapping table entry!" << endl;
	  exit(0);
	} //if
      } //for ichip
	  
      // Calculate rotation angle and TGeo rotation matrix;
      {
	double degAngle = istave*360.0/staveNum[ilayer];
	double radAngle = degAngle*TMath::Pi()/180.0;
	
	auto rw = new TGeoRotation();
	
	double fullAngle = degAngle + staveSlope;
	rw->SetAngles(90.0, 0.0 - fullAngle, 180.0,  0.0, 90.0, 90.0 - fullAngle);

	// Place volume copy in the master assembly container;
	vtx->GetTopVolume()->AddNode(vstave, staveCounter++, 
				     new TGeoCombiTrans(radius[ilayer]*sin(radAngle), 
							radius[ilayer]*cos(radAngle), 0.0, rw));
      }
    } //for istave
  } //for ilayer

  // Define color patterns; 
  vtx->GetColorTable()->AddPatternMatch("VtxStave",     kGray);
  vtx->GetTransparencyTable()->AddPatternMatch("VtxStave", 40);
  vtx->GetColorTable()->AddPatternMatch("VtxChip",      kYellow);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  vtx->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // vtx_builder()

