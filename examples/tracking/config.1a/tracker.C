
void tracker()
{
  // Detector name will be "FWDST" (Forward Silicon Tracker); should be consistent through 
  // all the [simulation.C -> digitization.C -> reconstruction.C] chain; 
  EicGeoParData *fst = new EicGeoParData("FwdST", 0, 0);
  // Output ROOT file name;
  fst->SetFileName("fwdst.root");

  // So 10x 250x250mm^2 layers of 200um thick silicon, 100mm apart from each other, at 
  // an "average" distance of 1m from the IP; layers orthogonal to the beam line direction;
  UInt_t waferNum                =             10;
  Double_t waferThickness        =  200 * eic::um;
  // Square wafers; beam pipe will not be used in the simulation.C anyway, 
  // so why bother about more realistic shapes; 
  Double_t waferWidth            =  250 * eic::mm;
  Double_t waferSpacing          =  100 * eic::mm;
  Double_t beamLineOffset        = 1000 * eic::mm;
  // Wafers will be placed inside a "container" air volume; give it +/-5mm size margin on all sides;
  Double_t containerVolumeLength = (waferNum-1) * waferSpacing + waferThickness + (10 * eic::mm);
  Double_t containerVolumeWidth  = waferWidth                                   + (10 * eic::mm);

  // Create a "container" TGeo box volume;
  TGeoBBox *container = new TGeoBBox("ContainerVolume", 
				     containerVolumeWidth/2,
				     containerVolumeWidth/2,
				     containerVolumeLength/2);
  // Media "air" and "silicon" are defined in geometry/media.geo;
  TGeoVolume *vcontainer = new TGeoVolume("ContainerVolume", container, fst->GetMedium("air"));

  // Silicon wafer;
  TGeoBBox *wafer = new TGeoBBox("SiliconWafer", waferWidth/2, waferWidth/2, waferThickness/2);
  TGeoVolume *vwafer = new TGeoVolume("SiliconWafer", wafer, fst->GetMedium("silicon"));

  // Black magic related to the sensitive volume mapping; can be safely ignored at
  // the beginning except for the vcontainer->AddNode() call, which places the silicon 
  // wafers into the air container one by one;
  {
    EicGeoMap *fgmap = fst->CreateNewMap();
    fgmap->AddGeantVolumeLevel("SiliconWafer", waferNum);
    fgmap->SetSingleSensorContainerVolume("SiliconWafer");
    
    // The easiest option: encode wafer ID along beam axis as Z-index; XY-indices are not needed;
    // alternatively could define 10 separate groups without XYZ-substructure;
    fst->AddLogicalVolumeGroup(0, 0, waferNum);
    
    // And place all wafers into the container volume;
    for(unsigned wf=0; wf<waferNum; wf++) {
      double offset = (wf - (waferNum-1)/2.)*waferSpacing;
      
      UInt_t geant[1] = {wf}, group = 0, logical[3] = {0, 0, wf};
      
      if (fst->SetMappingTableEntry(fgmap, geant, group, logical)) {
	cout << "Failed to set mapping table entry!" << endl;
	exit(0);
      } //if
      
      // The actual placement call;
      vcontainer->AddNode(vwafer, wf, new TGeoCombiTrans(0.0, 0.0, offset, new TGeoRotation()));
    } // for wf
  }

  // Place container volume into the detector assembly at a Z-offset of beamLineOffset;
  fst->GetTopVolume()->AddNode(vcontainer, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));
  fst->SetTopVolumeTransformation(new TGeoTranslation(0.0, 0.0, beamLineOffset));

  // Define color attributes of the silicon wafers (NB: volumes which are not 
  // described this way will NOT be visible in the event display;
  fst->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  fst->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  fst->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // tracker()

