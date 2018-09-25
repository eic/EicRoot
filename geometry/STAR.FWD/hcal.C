
// Meaningless number for now; fine;
#define _VERSION_     0
#define _SUBVERSION_  0

hcal()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  EicGeoParData *hcal = new EicGeoParData("HCAL", _VERSION_, _SUBVERSION_);
  const TString& cname = hcal->GetDetName()->Name();

  //hcal->SetGeometryType(EicGeoParData::FullStructure);

  // Take present design numbers; so 100x100mm^2 square volume of sufficient 
  // length to hold all the stuff;
  double CellFaceSizeX              =  100.0;
  // Yes, assume square section;
  double CellFaceSizeY              = CellFaceSizeX;

  // 36 Fe+Sci "cells" + 10mm rear steel plate;
  double SubCellLength              =   23.4;
  double SubCellNum                 =     36;
  double RearPlateThickness         =   10.0;
  double CellLength                 = SubCellNum*SubCellLength + RearPlateThickness;
  cout << "Cell length: " << CellLength << " mm" << endl;

  double AbsorberPlateWidth         =   96.0;
  // So 98+2=100mm cell height;
  double AbsorberPlateHeight        =   98.0;
  double SteelSpacerThickness       =    2.0;
  double AbsorberPlateThickness     =   20.0;

  // Scintillator parameters;
  double ScintillatorPlateThickness =    3.0;
  double ScintillatorPlateWidth     =   95.0;
  double ScintillatorPlateHeight    =   97.0;

  // WLS parameters; 
  double WlsPlateThickness          =    2.9;
  double WlsPlateLength             = CellLength;
  double WlsPlateHeight             =   97.0;
  // Assume other parameters match WLS;
  double MylarThickness             =    0.1;

  // Air container volume;
  TGeoBBox *tower = new TGeoBBox(cname + "Tower", 
				 0.1 * CellFaceSizeX/2,
				 0.1 * CellFaceSizeY/2,
				 0.1 * CellLength/2);
  TGeoVolume *vtower = new TGeoVolume(cname + "Tower", tower, hcal->GetMedium("air"));

  // Absorber plates;
  {
    TGeoBBox *absPlate = new TGeoBBox(cname + "AbsorberPlate", 
				       0.1 * AbsorberPlateWidth/2,
				       0.1 * AbsorberPlateHeight/2,
				       0.1 * AbsorberPlateThickness/2);
    TGeoVolume *vabsPlate = 
      // Assume pure iron for now?;
      new TGeoVolume(cname + "AbsorberPlate", absPlate, hcal->GetMedium("iron"));

    // Assume shifts to-the-right and to-the-bottom;
    double offsetX = -(CellFaceSizeX - AbsorberPlateWidth)/2.;
    double offsetY = -(CellFaceSizeY - AbsorberPlateHeight)/2.;
	  
    for(unsigned pt=0; pt<SubCellNum; pt++) {
      double offsetZ = -CellLength/2. + AbsorberPlateThickness/2 + pt*SubCellLength;
      
      vtower->AddNode(vabsPlate, pt, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
							0.1 * offsetZ, 0));
    } //for pt

    // Prefer to decouple single last (thin) plate completely; 
    TGeoBBox *rearPlate = new TGeoBBox(cname + "RearSteelPlate", 
					0.1 * AbsorberPlateWidth/2,
					0.1 * AbsorberPlateHeight/2,
					0.1 * RearPlateThickness/2);
    TGeoVolume *vrearPlate = 
      // Assume pure iron for now?;
      new TGeoVolume(cname + "RearSteelPlate", rearPlate, hcal->GetMedium("iron"));
    {
      double offsetZ = CellLength/2. - RearPlateThickness/2;

      vtower->AddNode(vrearPlate, 0, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
							0.1 * offsetZ, 0));
    }
  }

  // Scintillator plates;
  {
    TGeoBBox *sciPlate = new TGeoBBox(cname + "ScintillatorPlate", 
				      0.1 * ScintillatorPlateWidth/2,
				      0.1 * ScintillatorPlateHeight/2,
				      0.1 * ScintillatorPlateThickness/2);
    TGeoVolume *vsciPlate = 
      new TGeoVolume(cname + "ScintillatorPlate", sciPlate, hcal->GetMedium("polystyrene"));
    
    // Assume shifts to-the-center-gap (loose) and to-the-bottom;
    double offsetX = -(CellFaceSizeX - ScintillatorPlateWidth)/2.;
    double offsetY = -(CellFaceSizeY - ScintillatorPlateHeight)/2.;
    
    for(unsigned pt=0; pt<SubCellNum; pt++) {
      // Assume shifts to-the-center-gap (loose);
      double offsetZ = -CellLength/2. +
	(pt+1)*SubCellLength - (SubCellLength - AbsorberPlateThickness)/2;
      
      vtower->AddNode(vsciPlate, pt, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
							0.1 * offsetZ, 0));
    } //for pt
  }

  // WLS plate, two mylar films and steel cover plate (long pieces with the same length);
  {
    TGeoBBox *wlsPlate = new TGeoBBox(cname + "WlsPlate", 
				      0.1 * WlsPlateThickness/2,
				      0.1 * WlsPlateHeight/2,
				      0.1 * WlsPlateLength/2);
    TGeoVolume *vwlsPlate = 
      // Assume polystyrene for now?;
      new TGeoVolume(cname + "WlsPlate", wlsPlate, hcal->GetMedium("polystyrene"));
    
    // Two mylar layers;
    TGeoBBox *mylarFilm = new TGeoBBox("mylarFilm", 
				       0.1 * MylarThickness/2,
				       0.1 * WlsPlateHeight/2,
				       0.1 * WlsPlateLength/2);
    TGeoVolume *vmylarFilm = 
      // Assume kapton for now?;
      new TGeoVolume(cname + "MylarFilm", mylarFilm, hcal->GetMedium("kapton"));
    
    TGeoBBox *steelSpacer = new TGeoBBox(cname + "SteelSpacer", 
					 0.1 * CellFaceSizeX/2,
					 0.1 * SteelSpacerThickness/2,
					 0.1 * WlsPlateLength/2);
    TGeoVolume *vsteelSpacer = 
      // Assume pure iron for now?;
      new TGeoVolume(cname + "SteelSpacer", steelSpacer, hcal->GetMedium("iron"));
    
    // Assume shifts to-the-center-gap (loose) and to-the-bottom;
    double offsetX = CellFaceSizeX/2. - (CellFaceSizeX - AbsorberPlateWidth)/2.;
    double offsetY = -(CellFaceSizeY - WlsPlateHeight)/2.;
    double offsetZ = (CellLength - WlsPlateLength)/2.;
    
    vtower->AddNode(vwlsPlate, 0, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
						     0.1 * offsetZ, 0));
    
    for(unsigned lr=0; lr<2; lr++) {
      // Well, may want to introduce an air gap later;
      double dx = (lr ? -1. : 1.)*(WlsPlateThickness + MylarThickness)/2;
      
      vtower->AddNode(vmylarFilm, lr, new TGeoCombiTrans(0.1 * (offsetX + dx), 
							 0.1 *offsetY, 0.1 * offsetZ, 0));
    } //for lr
    
    vtower->AddNode(vsteelSpacer, 0, 
		    new TGeoCombiTrans(0.0,
				       0.1 * (CellFaceSizeY/2 - SteelSpacerThickness/2),
				       0.1 * offsetZ, 0));
  }

  // Place this singletower into the top volume;
  hcal->GetTopVolume()->AddNode(vtower, 0, new TGeoCombiTrans());

  // Define colors;
  hcal->GetColorTable()->AddPatternMatch(cname + "Tower", kGray   +1);
  hcal->GetColorTable()->AddPatternMatch(cname + "Wls",   kGreen  +1);
  hcal->GetColorTable()->AddPatternMatch(cname + "Scint", kBlue   +1);
  hcal->GetColorTable()->AddPatternMatch(cname + "Steel", kYellow +1);
  hcal->GetColorTable()->AddPatternMatch(cname + "Abso",  kYellow +1);
    
  hcal->FinalizeOutput();

  // Yes, always exit;
  exit(0);
}

