
void rp()
{
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *rp = new EicGeoParData("RP", 0, 0);
  rp->SetFileName("rp.root");

  UInt_t waferNum                    =      2;
  Double_t waferThickness            =    0.3;
  // Just very large rectangular pieces of silicon;
  Double_t waferWidth                = 1200.0;
  Double_t waferHeight               =  500.0;
  Double_t waferSpacing              =   200.0;
  // Move roughly to the beam line axis;
  Double_t beamLineOffsetX           =   815.0;
  //+Double_t beamLineOffsetZ           = 28200.0;
  Double_t beamLineOffsetZ           = 28100.0;
  //Double_t nominal_proton_direction  =     0.022;

  // Silicon wafer;
  TGeoBBox *wafer = new TGeoBBox("RpSiliconWafer", 
				  0.1 * waferWidth/2,
				  0.1 * waferHeight/2,
				  0.1 * waferThickness/2);
  TGeoVolume *vwafer = new TGeoVolume("RpSiliconWafer", wafer, rp->GetMedium("silicon"));

  EicGeoMap *fgmap = rp->CreateNewMap();
  fgmap->AddGeantVolumeLevel("RpSiliconWafer", waferNum);
  fgmap->SetSingleSensorContainerVolume("RpSiliconWafer");

  rp->AddLogicalVolumeGroup(0, 0, waferNum);

  // And place all wafers into the container volume;
  for(unsigned wf=0; wf<waferNum; wf++)
  {
    double offset = 0.1 * (wf - (waferNum-1)/2.)*waferSpacing;

    UInt_t geant[1] = {wf}, group = 0, logical[3] = {0, 0, wf};
    
    if (rp->SetMappingTableEntry(fgmap, geant, group, logical)) {
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    } //if

    rp->GetTopVolume()->AddNode(vwafer, wf, new TGeoCombiTrans(0.1 * beamLineOffsetX, 0.0, offset + 0.1 * beamLineOffsetZ, 0));
  } // for wf

  rp->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  rp->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  rp->FinalizeOutput();

  exit(0);
} // rp()

