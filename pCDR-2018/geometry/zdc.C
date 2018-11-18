
void zdc()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *zdc = new EicGeoParData("ZDC", 0, 0);
  zdc->SetFileName("zdc.root");

  // Do not care much about precise description;
  Double_t zdcWidth            =  600.0;
  Double_t zdcHeight           =  600.0;
  Double_t zdcLength           = 1000.0;
  Double_t beamLineOffset      =28800.0;
  Double_t waferThickness      =    0.3;
  

  TGeoRotation *rw = new TGeoRotation();
  double angle = 0.022, dx = 110.0 + 30.0;
  rw->RotateY(angle * TMath::RadToDeg());

  TGeoBBox *bzdc = new TGeoBBox("ZdcBox", 
				0.1 * zdcWidth/2,
				0.1 * zdcHeight/2,
				0.1 * zdcLength/2);
  TGeoVolume *vzdc = new TGeoVolume("ZdcBox", bzdc, zdc->GetMedium("iron"));
  {
    // NB: it looks like it is ~190mm proton beam line separation; give another 30mm as 
    // a ten signa estimate for now;
    double xx = beamLineOffset*sin(angle) - dx, zz = beamLineOffset*cos(angle);

    zdc->GetTopVolume()->AddNode(vzdc, 0, new TGeoCombiTrans(0.1 * xx, 0.0, 0.1 * zz, rw));
  }

  // Fake silicon wafer in front of it;
  TGeoBBox *bwafer = new TGeoBBox("ZdcSiliconWafer", 
				  0.1 * zdcWidth/2,
				  0.1 * zdcHeight/2,
				  0.1 * waferThickness/2);
  TGeoVolume *vwafer = new TGeoVolume("ZdcSiliconWafer", bwafer, zdc->GetMedium("silicon"));
  {
    double z0 = beamLineOffset - 530;
    double xx = z0*sin(angle) - dx, zz = z0*cos(angle);

    zdc->GetTopVolume()->AddNode(vwafer, 0, new TGeoCombiTrans(0.1 * xx, 0.0, 0.1 * zz, rw));
  }
  zdc->AddWantedParticle("ZdcSiliconWafer", 2112);
  EicGeoMap *fgmap = zdc->CreateNewMap();
  fgmap->AddGeantVolumeLevel("ZdcSiliconWafer", 1);//waferNum);
  fgmap->SetSingleSensorContainerVolume("ZdcSiliconWafer");

  // Easiest option: encode wafer ID along beam axis as Z-index; XY-indices are not needed;
  // alternatively could define 10 separate groups without XYZ-substructure;
  zdc->AddLogicalVolumeGroup(0, 0, 1);//waferNum);

  UInt_t geant[1] = {0}, group = 0, logical[3] = {0, 0, 0};
    
  if (zdc->SetMappingTableEntry(fgmap, geant, group, logical)) {
    cout << "Failed to set mapping table entry!" << endl;
    exit(0);
  } //if
 
  zdc->GetColorTable()->AddPatternMatch       ("ZdcBox", kRed);
  zdc->GetTransparencyTable()->AddPatternMatch("ZdcBox", 0);
  zdc->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  zdc->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  zdc->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // zdc()

