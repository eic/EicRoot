
void lqst()
{
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *lqst = new EicGeoParData("LQST", 0, 0);
  lqst->SetFileName("lqst.root");

  UInt_t waferNum                    =      2;
  Double_t waferThickness            =    0.3;
  // Just very large rectangular pieces of silicon;
  Double_t waferWidth                = 1200.0;
  Double_t waferHeight               = 1000.0;
  Double_t waferSpacing              =   200.0;
  // Move roughly to the beam line axis;
  Double_t beamLineOffsetX           =    0.0;
  Double_t beamLineOffsetZ           = -28000.0;

  // Silicon wafer;
  TGeoBBox *wafer = new TGeoBBox("LqstSiliconWafer", 
				  0.1 * waferWidth/2,
				  0.1 * waferHeight/2,
				  0.1 * waferThickness/2);
  TGeoVolume *vwafer = new TGeoVolume("LqstSiliconWafer", wafer, lqst->GetMedium("silicon"));

  EicGeoMap *fgmap = lqst->CreateNewMap();
  fgmap->AddGeantVolumeLevel("LqstSiliconWafer", waferNum);
  fgmap->SetSingleSensorContainerVolume("LqstSiliconWafer");

  lqst->AddLogicalVolumeGroup(0, 0, waferNum);

  // And place all wafers into the container volume;
  for(unsigned wf=0; wf<waferNum; wf++)
  {
    double offset = 0.1 * (wf - (waferNum-1)/2.)*waferSpacing;

    UInt_t geant[1] = {wf}, group = 0, logical[3] = {0, 0, wf};
    
    if (lqst->SetMappingTableEntry(fgmap, geant, group, logical)) {
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    } //if

    lqst->GetTopVolume()->AddNode(vwafer, wf, new TGeoCombiTrans(0.1 * beamLineOffsetX, 0.0, offset + 0.1 * beamLineOffsetZ, 0));
  } // for wf

  lqst->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  lqst->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  lqst->FinalizeOutput();

  exit(0);
} // lqst()

