
//
// A small spherical shell located at (0,0,0);
//

void ip()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *sph = new EicGeoParData("IP", 0, 0);
  // Output ROOT file name;
  sph->SetFileName("ip-sphere.root");

  double rmin = 0.020 *eic::mm;
  double rmax = 0.050 *eic::mm;

  // Create a "container" TGeo box volume;
  TGeoSphere *sphere = new TGeoSphere("Sphere", rmin, rmax); 
  TGeoVolume *vsphere = new TGeoVolume("Sphere", sphere, sph->GetMedium("air"));

  {
    EicGeoMap *fgmap = sph->CreateNewMap();
    fgmap->AddGeantVolumeLevel("Sphere", 1);
    fgmap->SetSingleSensorContainerVolume("Sphere");
    
    sph->AddLogicalVolumeGroup(0, 0, 1);

    UInt_t geant[1] = {0}, group = 0, logical[3] = {0, 0, 0};
      
    sph->SetMappingTableEntry(fgmap, geant, group, logical);
    
    sph->GetTopVolume()->AddNode(vsphere, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));
  }

  // Define color attributes of the silicon wafers (NB: volumes which are not 
  // described this way will NOT be visible in the event display;
  sph->GetColorTable()->AddPatternMatch       ("Sphere", kGreen);
  sph->GetTransparencyTable()->AddPatternMatch("Sphere", 50);

  // A unified user call which places assembled detector volume in a proper place in MASTER (top)
  // coordinate system, puts this MASTER (top) volume into GEANT volume tree, and dumps this tree 
  // together with EicRoot mapping table in one file;
  sph->FinalizeOutput();

  // Yes, always exit;
  exit(0);
} // ip()

