
//
//  Example tracker: 5x XY GEM stations, 0.86% rad.length each (use carbon of appropriate 
//  thickness); output file (FwdGT.root) can be used as geometry input for 
//  simulation.C->digitization.C->reconstruction.C scripts in this directory; 
//  resolutions are specified at a later stage (see digitization.C); 
//
//  Prefer to declare dimensions in [mm]; convert to [cm] when calling ROOT shape 
//  definition routines only;
//

// 5 GEM XY-stations at predefined locations;
#define _GNUM_  5

void tracker()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Detector name will be "FwdGT" (Forward GEM Tracker); should be consistent through 
  // all the simulation.C->digitization.C->reconstruction.C chain; see calorimetry/calorimeter.C
  // for the rationale behind using EicGeoParDataHelper wrapper class;
  //EicGeoParDataHelper *helper = new EicGeoParDataHelper(NULL, "FwdGT", "FwdGT.root");
  EicGeoParData *fwdgt = new EicGeoParData("FwdGT", 0, 0);
  fwdgt->SetFileName("fwdgt.root");

  // 5 XY-planes at predefined locations; 
  Double_t zGem[_GNUM_]       = {170., 600., 1500., 2000., 3000.};
  Double_t rMax               =   500.0;
  
  // GEM module thickness and RICH entrance window thickness (use carbon layers);
  Double_t carbonX0           =   188.0;
  Double_t gemThickness       =     0.86 * carbonX0 / 100.0;
  // Assume 1% rad.length (HERMES ~1.0% alu, LHC-B ~1.4% carbon fiber);
  Double_t wndThickness       =     1.00 * carbonX0 / 100.0;
  Double_t wndLocation        =  2005.0;
  Double_t cf4Thickness       =   980.0;
  Double_t cf4Location        =  2500.0;

  // Give it +/-5mm on all sides; use 5-th GEM as a reference; fix later;
  Double_t holderVolumeLength = 2.*zGem[_GNUM_-1] + 10.0;
  Double_t holderVolumeRadius =  rMax +  5.0;
  //cout << holderVolumeLength << " " << holderVolumeRadius << endl; exit(0);

  // Create a "holder" volume;
  TGeoTube *holder = new TGeoTube("holderVolume", 
  				  0.1 * 0.0,
  				  0.1 * holderVolumeRadius,
  				  0.1 * holderVolumeLength/2);
  TGeoVolume *vholder = new TGeoVolume("holderVolume", holder, fwdgt->GetMedium("air"));

  // RICH entrance window;
  TGeoTube *wnd = new TGeoTube("richWnd", 
			       0.1 * 0.0,
			       0.1 * rMax,
			       0.1 * wndThickness/2);
  TGeoVolume *vwnd = new TGeoVolume("richWnd", wnd, fwdgt->GetMedium("carbon"));
  vholder->AddNode(vwnd, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * wndLocation, new TGeoRotation()));

  // RICH gas volume;
  TGeoTube *cf4 = new TGeoTube("richVolume", 
			       0.1 * 0.0,
			       0.1 * rMax,
			       0.1 * cf4Thickness/2);
  TGeoVolume *vcf4 = new TGeoVolume("richVolume", cf4, fwdgt->GetMedium("CF4"));
  vholder->AddNode(vcf4, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * cf4Location, new TGeoRotation()));

  TGeoTube *gem = new TGeoTube("gemVolume",
  			       0.1 *   0.0,
  			       0.1 * rMax,
  			       0.1 * gemThickness/2);
  TGeoVolume *vgem = new TGeoVolume("gemVolume", gem, fwdgt->GetMedium("carbon"));

  EicGeoMap *fgmap = fwdgt->CreateNewMap();
  fgmap->AddGeantVolumeLevel("gemVolume",  _GNUM_);
  fgmap->SetSingleSensorContainerVolume("gemVolume");
  fwdgt->AddLogicalVolumeGroup(0, 0, _GNUM_);

  // Silicon "plates"; just place them one by one;
  for(unsigned gm=0; gm<_GNUM_; gm++) {
    UInt_t geant[1] = {gm}, group = 0, logical[3] = {0, 0, gm};
    
    if (fwdgt->SetMappingTableEntry(fgmap, geant, group, logical)) {
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    } //if

    vholder->AddNode(vgem, gm, new TGeoCombiTrans(0.0, 0.0, 0.1 * zGem[gm], new TGeoRotation()));
  } //for gm

  // Place holder volume;
  fwdgt->GetTopVolume()->AddNode(vholder, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));

  fwdgt->GetColorTable()->AddPatternMatch       ("gemVolume", kOrange);
  fwdgt->GetTransparencyTable()->AddPatternMatch("gemVolume", 50);

  fwdgt->GetColorTable()->AddPatternMatch       ("richWnd", kMagenta);
  fwdgt->GetTransparencyTable()->AddPatternMatch("richWnd", 50);

  fwdgt->FinalizeOutput();
  exit(0);
} // tracker()

