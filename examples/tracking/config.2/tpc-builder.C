
//
//  This TPC implementation is indeed naive; in particular it digitizes every 
// MC point separately, therefore heavily relies on the max step value for 
// "ArCF4iC4H10" mixture (1cm in media.geo); so is expected to work more 
// or less reasonably for eta~0 tracks;
//

// A more real-life digitization scheme will be invoked if this line is uncommented
// (see digitization.C for more details); ! NB: need to re-run both tpc-builder.C & 
// simulation.C before running digitization.C !;
#define _USE_BEAST_TPC_DIGITIZER_

// Used volume names;
#define _CONTAINER_NAME_  "TpcContainer"
#define _GAS_VOLUME_NAME_ "GasVolume"
#define _FIELD_CAGE_NAME_ "FieldCage"

tpc_builder()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Detector name will be "TPC"; should be consistent through 
  // all the simulation.C->digitization.C->reconstruction.C chain;
#ifdef _USE_BEAST_TPC_DIGITIZER_
  TpcGeoParData *tpc = new TpcGeoParData();
#else
  EicGeoParData *tpc = new EicGeoParData("TPC");
#endif

  // Hardcode TGeo file name (no versioning, etc);
  tpc->SetFileName("tpc.root");

  //
  //   Hardcode all the geometry parameters for simplicity; NB: for most part of 
  // EicRoot detector types respective variables (like gas volume radius below) 
  // are EicGeoParData inherited class variables which get saved in the 
  // output file containing TGeo information; units are [cm] everywhere;
  //

  double innerGasVolumeRadius     =  12.0;
  double outerGasVolumeRadius     =  42.0;
  // Will be two half-length volumes;
  double gasVolumeLength          =  30.0;
#ifdef _USE_BEAST_TPC_DIGITIZER_
  // Well, this is the only parameter required by EicTpcDigiHitProducer::HandleHit();
  // and parameters in TpcGeoParData class are assumed to be in [mm];
  tpc->mTotalGasVolumeLength      = 10.0 * gasVolumeLength;
#endif
  // Well, 2.0% rad.length must be reasonable for IFC (see "X0=10cm" material definition);
  double fieldCageThickness       =   0.2;

  unsigned group = tpc->AddLogicalVolumeGroup(0, 0, 2);
  EicGeoMap *xmap = tpc->CreateNewMap();
  xmap->AddGeantVolumeLevel(_GAS_VOLUME_NAME_, 2);
  xmap->SetSingleSensorContainerVolume(_GAS_VOLUME_NAME_);
  // Well, GEANT4 mode does not acknowledge max step size in VGM; fix later; 
  tpc->AddStepEnforcedVolume(_GAS_VOLUME_NAME_);

  // Inner field cage;
  TGeoTube *fc = new TGeoTube(_FIELD_CAGE_NAME_,
			       innerGasVolumeRadius - fieldCageThickness,
			       innerGasVolumeRadius,
			       gasVolumeLength/2);
  TGeoVolume *vfc = new TGeoVolume(_FIELD_CAGE_NAME_, fc, tpc->GetMedium("X0=10cm"));
  tpc->GetTopVolume()->AddNode(vfc, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

  // Gas volume definition;
  TGeoTube *gv = new TGeoTube(_GAS_VOLUME_NAME_,
			      innerGasVolumeRadius,
			      outerGasVolumeRadius,
			      gasVolumeLength/4);
  // NB: this medium is defined as 1cm max.step (see media.geo); so at eta~0 expect 
  // roughly (rmax-rmin) hits (GEANT steps); fine for demonstration purposes;
  TGeoVolume *vgv = new TGeoVolume(_GAS_VOLUME_NAME_, gv, tpc->GetMedium("ArCF4iC4H10"));

  // Gas volumes (upstream & downstream halves);
  for(unsigned ud=0; ud<2; ud++) {
    // Insert mapping table entry; a trivial one, indeed;
    UInt_t geant[1] = {ud}, logical[3] = {0, 0, ud};
    if (tpc->SetMappingTableEntry(xmap, geant, group, logical)) {
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    } //if

    double zOffset = (ud ? -1. : 1.)*gasVolumeLength/4;

    // Well, the idea behind this rotation for ud=1 half is that local hit coordinates
    // will be obtained in coordinate system which has the same orientation with respect 
    // to the pad plane, no matter ud=0 or ud=1 gas volume was hit; this is indeed helpful
    // if hit resolution depends on drift distance, etc;
    TGeoRotation *rw = 0;
    if (ud) {
      TGeoRotation *rw = new TGeoRotation();
      rw->RotateY(180); 
    } //if

    tpc->GetTopVolume()->AddNode(vgv, ud, new TGeoCombiTrans(0.0, 0.0, zOffset, rw));
  } //for ud

  tpc->GetColorTable()->AddPatternMatch("FieldCage",    kGray);
  tpc->GetColorTable()->AddPatternMatch("GasVolume",    kCyan);
  tpc->GetTransparencyTable()->AddPatternMatch("GasVolume", 40);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  tpc->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // tpc_builder()

