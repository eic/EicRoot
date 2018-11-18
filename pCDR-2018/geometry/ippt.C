
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

void ippt()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Detector name will be "IPPT" (IP Point); 
  EicGeoParData *ippt = new EicGeoParData("IPPT", 0, 0);
  ippt->SetFileName("ip-point.root");

  Double_t fakeThickness        =    0.1;
  //Double_t wndThickness         =    0.3;
  // Rectangular wafers; beam pipe will not be used in simulation.C anyway, 
  // so why bother about more realistic shapes; 
  //Double_t fakeWidth            =   250.0; 
  Double_t fakeRadius           =    20.0;
  

  // Fake registering station at the IP;
  TGeoTube *ipspot = new TGeoTube("IpPoint",
				  0.0,
				  0.1 * fakeRadius,
				  0.1 * fakeThickness/2);
  TGeoVolume *vippt = new TGeoVolume("IpPoint", ipspot, ippt->GetMedium("silicon"));

  EicGeoMap *fgmap = ippt->CreateNewMap();
  fgmap->AddGeantVolumeLevel("IpPoint", 1);
  fgmap->SetSingleSensorContainerVolume("IpPoint");

  ippt->AddLogicalVolumeGroup(0, 0, 1);

  {
    UInt_t geant[1] = {0}, group = 0, logical[3] = {0, 0, 0};
    
    if (ippt->SetMappingTableEntry(fgmap, geant, group, logical)) {
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    } //if

    ippt->GetTopVolume()->AddNode(vippt, 0, new TGeoCombiTrans(0.0, 0.0, 0.1, new TGeoRotation()));
  } // for wf

#if _LATER_
  TGeoBBox *wndbox = new TGeoBBox("ExitWindow", 
				  0.1 * fakeWidth/2,
				  0.1 * fakeWidth/2,
				  0.1 * wndThickness/2);
  TGeoVolume *vwnd = new TGeoVolume("ExitWindow", wndbox, ippt->GetMedium("iron"));
  ippt->GetTopVolume()->AddNode(vwnd, 0, new TGeoCombiTrans(0.0, 0.0, 0.1*4900.0, new TGeoRotation()));
#endif

  ippt->GetColorTable()->AddPatternMatch       ("IpPoint", kBlue);
  ippt->GetTransparencyTable()->AddPatternMatch("IpPoint", 50);
  //ippt->GetColorTable()->AddPatternMatch       ("Exit", kGreen);
  //ippt->GetTransparencyTable()->AddPatternMatch("Exit", 50);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  ippt->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // tracker()

