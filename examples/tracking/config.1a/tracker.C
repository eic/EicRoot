
//
//  Example tracker: 10x 200um thick layers of silicon in H+ beam direction; 
//  output file (fwdst.root) can be used as geometry input for 
//  simulation.C->digitization.C->reconstruction.C scripts in this directory; 
//
//  resolutions are specified at a later stage (see digitization.C); 
//
//  Prefer to declare dimensions in [mm]; convert to [cm] when calling ROOT shape 
//  definition routines only;
//

void tracker()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Detector name will be "FWDST" (Forward Silicon Tracker); should be consistent through 
  // all the simulation.C->digitization.C->reconstruction.C chain; see calorimetry/calorimeter.C
  // for the rationale behind using EicGeoParDataHelper wrapper class;
  EicGeoParData *fst = new EicGeoParData("FwdST", 0, 0);
  fst->SetFileName("fwdst.root");

  // So 10x 250x250mm^2 layers of 200um thick silicon, 100mm apart from each other, at an "average"
  // distance of 1m from the IP; layers orthogonal to the beam line direction;
  UInt_t waferNum                =             10;
  Double_t waferThickness        =  200 * eic::um;
  // Rectangular wafers; beam pipe will not be used in simulation.C anyway, 
  // so why bother about more realistic shapes; 
  Double_t waferWidth            =  250 * eic::mm;
  Double_t waferSpacing          =  100 * eic::mm;
  Double_t beamLineOffset        = 1000 * eic::mm;
  // Give it +/-5mm on all sides;
  Double_t containerVolumeLength = (waferNum-1) * waferSpacing + waferThickness + (10 * eic::mm);
  Double_t containerVolumeWidth  = waferWidth                                   + (10 * eic::mm);

  // Create a "container" volume;
  TGeoBBox *container = new TGeoBBox("ContainerVolume", 
				     containerVolumeWidth/2,
				     containerVolumeWidth/2,
				     containerVolumeLength/2);
  // Make sure media names are listed in geometry/media.geo;
  TGeoVolume *vcontainer = new TGeoVolume("ContainerVolume", container, fst->GetMedium("air"));

  // Silicon wafer;
  TGeoBBox *wafer = new TGeoBBox("SiliconWafer", 
				 waferWidth/2,
				 waferWidth/2,
				 waferThickness/2);
  TGeoVolume *vwafer = new TGeoVolume("SiliconWafer", wafer, fst->GetMedium("silicon"));

  EicGeoMap *fgmap = fst->CreateNewMap();
  fgmap->AddGeantVolumeLevel("SiliconWafer", waferNum);
  fgmap->SetSingleSensorContainerVolume("SiliconWafer");

  // Easiest option: encode wafer ID along beam axis as Z-index; XY-indices are not needed;
  // alternatively could define 10 separate groups without XYZ-substructure;
  fst->AddLogicalVolumeGroup(0, 0, waferNum);

  // And place all wafers into the container volume;
  for(unsigned wf=0; wf<waferNum; wf++)
  {
    double offset = (wf - (waferNum-1)/2.)*waferSpacing;

    UInt_t geant[1] = {wf}, group = 0, logical[3] = {0, 0, wf};
    
    if (fst->SetMappingTableEntry(fgmap, geant, group, logical)) {
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    } //if

    vcontainer->AddNode(vwafer, wf, new TGeoCombiTrans(0.0, 0.0, offset, new TGeoRotation()));
  } // for wf

  // Place container volume;
  fst->GetTopVolume()->AddNode(vcontainer, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));

  fst->SetTopVolumeTransformation(new TGeoTranslation(0.0, 0.0, beamLineOffset));

  fst->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  fst->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  fst->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // tracker()

