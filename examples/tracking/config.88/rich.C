//
// Purpose of script is to emulate a simple RICH configuration with
// fake material for approximation of BeAST config.
//

// Units are given explicitly and as such given to the 
// ROOT primitive creation and positioning routines;

// Meaningless numbers for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

void rich()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *rich = new EicGeoParData("RICH", _VERSION_, _SUBVERSION_);

  // Simplify things for the time being; 
  double beam_pipe_radius     =     20.0 * eic::mm;

  Double_t rMin               =    850.0 * eic::mm;
  Double_t rMax               =   1600.0 * eic::mm;
  Double_t wndThickness       =      200 * eic::um;
  Double_t wndEnLocation      =   1250.0 * eic::mm;
  Double_t cf4Thickness       =   1000.0 * eic::mm;
  Double_t cf4Location        =  wndEnLocation + wndThickness/2 + cf4Thickness/2;
  Double_t wndExLocation      =  wndEnLocation + wndThickness + cf4Thickness;

  // Relative to the gas volume center;
  Double_t profilerOffset     =   -200.0 * eic::mm;
  // Should not really matter, but somehow low density material profiler does not 
  // work well (missing hits); just use a very thin silicon layer;
  Double_t profilerThickness  =       10 * eic::um;
  Double_t profilerRadius     =  rMin + (profilerOffset + cf4Thickness/2 - profilerThickness/2)*(rMax-rMin)/cf4Thickness;
    
  // RICH entrance window;
  TGeoTube *wnden = new TGeoTube("richWndEn", 
				 beam_pipe_radius,
				 rMin,
				 wndThickness/2);
  TGeoVolume *vwnden = new TGeoVolume("richWndEn", wnden, rich->GetMedium("carbon"));

  rich->GetTopVolume()->AddNode(vwnden, 0, new TGeoCombiTrans(0.0, 0.0, wndEnLocation, new TGeoRotation()));

  // RICH gas volume;
  TGeoCone *cf4 = new TGeoCone("richVolume",
			       cf4Thickness/2,
			       beam_pipe_radius,
			       rMin,
			       beam_pipe_radius,
			       rMax);
  TGeoVolume *vcf4 = new TGeoVolume("richVolume", cf4, rich->GetMedium("CF4"));
  rich->GetTopVolume()->AddNode(vcf4, 0, new TGeoCombiTrans(0.0, 0.0, cf4Location, new TGeoRotation()));

  // RICH volume tracking profiler plane;
  TGeoTube *prof = new TGeoTube("richProfiler", 
				beam_pipe_radius,
				profilerRadius,
				profilerThickness/2);
  // Yes, want a different material here, otherwise TGeo transport gets confused;
  TGeoVolume *vprof = new TGeoVolume("richProfiler", prof, rich->GetMedium("silicon"));
  vcf4->AddNode(vprof, 0, new TGeoCombiTrans(0.0, 0.0, profilerOffset, new TGeoRotation()));
  EicGeoMap *richmap = rich->CreateNewMap();                   
  richmap->AddGeantVolumeLevel("richProfiler", 1);
  richmap->SetSingleSensorContainerVolume("richProfiler");
  rich->AddLogicalVolumeGroup(0, 0, 1);
  {
    UInt_t geant[1] = {0}, group = 0, logical[3] = {0, 0, 0};
      
    if(rich->SetMappingTableEntry(richmap, geant, group, logical)){
      cout << "Failed to set mapping table entry!" << endl;
      exit(0);
    }//if
  }  
  
  // RICH exit window;
  TGeoTube *wndex = new TGeoTube("richWndEx",
  				 beam_pipe_radius,
  				 rMax,
  				 wndThickness/2);
  TGeoVolume *vwndex = new TGeoVolume("richWndEx", wndex, rich->GetMedium("carbon"));
  rich->GetTopVolume()->AddNode(vwndex, 0, new TGeoCombiTrans(0.0, 0.0, wndExLocation, new TGeoRotation()));
  
  rich->GetColorTable()->AddPatternMatch("richWndEn", kRed);
  rich->GetTransparencyTable()->AddPatternMatch("richWndEn", 50);
  
  rich->GetColorTable()->AddPatternMatch("richVolume", kMagenta);
  rich->GetTransparencyTable()->AddPatternMatch("richVolume", 50);

  rich->GetColorTable()->AddPatternMatch("richProfiler", kCyan);
  rich->GetTransparencyTable()->AddPatternMatch("richProfiler", 50);

  rich->GetColorTable()->AddPatternMatch("richWndEx", kRed);
  rich->GetTransparencyTable()->AddPatternMatch("richWndEx", 50);
 
  rich->FinalizeOutput();
  exit(0);
}//rich()
