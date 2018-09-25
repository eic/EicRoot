//
// Trivial routines which are shared between T1018 test run geometry builder and
// complete EIC model detector builder;
//

// Again, do this pointer passing better later;
void *define_basic_parameters(const char *detName, int version, int subVersion)
{
  // '1': yes, need only one map for now; may want to crete the other ones 
  // if ever want to paerform a complete energy deposit analysis;
  HcalGeoParData *gpar = new HcalGeoParData(detName, version, subVersion);

  // Let it be qSandwich even if no internal structure is wanted;
  gpar->mType                       = CalorimeterGeoParData::Sandwich;

  //
  // Declare basic parameters; ignore parameters relevant for a full-scale configuration;
  //

  // Take present design numbers; so 100x100mm^2 square volume of sufficient 
  // length to hold all the stuff;
  gpar->mCellFaceSizeX              =  100.0;
  // Yes, assume square section and just use gpar->mCellFaceSizeX in all 'critical' 
  // places like radius calculation; NB: code modifications will be needed if XY-sizes
  // ever become different!;
  gpar->mCellFaceSizeY              = gpar->mCellFaceSizeX;

  // This stuff is needed no matter internal tower geometry is defined or not;
  gpar->mSubCellLength              =   13.1;
  gpar->mSubCellNum                 =     64;
  // 63x13.1mm "Pb cells" and the last 1x13.1mm "Fe cell";
  gpar->mCellLength                 = gpar->mSubCellNum*gpar->mSubCellLength;
  cout << "Cell length: " << gpar->mCellLength << " mm" << endl;

  // Otherwise no sense to store this stuff in the ROOT file;
#ifndef _NO_STRUCTURE_GEOMETRY_
  gpar->mLeadPlateWidth             =   96.0;
  // So 98+2=100mm cell height;
  gpar->mLeadPlateHeight            =   98.0;
  gpar->mSteelSpacerThickness       =    2.0;
  gpar->mLeadPlateThickness         =   10.0;

  // Scintillator parameters;
  gpar->mScintillatorPlateThickness =    2.5;
  gpar->mScintillatorPlateWidth     =   95.0;
  gpar->mScintillatorPlateHeight    =   97.0;

  // WLS parameters; 
  gpar->mWlsPlateThickness          =    3.0;
  // Assume other parameters match WLS;
  gpar->mMylarThickness             =    0.1;
  // Does it actually match Oleg's drawings?;
  gpar->mWlsPlateLength             = gpar->mSubCellNum*gpar->mSubCellLength;
  gpar->mWlsPlateHeight             =   97.0;

  // Pin parameters; *full* pin length is given here; see fraction of pin 
  // contained in a given lead plate calculation below;
  gpar->mPinLength                  =   20.0;
  gpar->mPinDiameter                =    5.0;
  // Pin-to-pin distance in X-direction; see details below in the code;
  gpar->mPinToPinDistance           =   50.0;
#endif

  // Well, assume no-gap packing;
  gpar->mInterCellGap               =    0.0; 

  return gpar;
} // define_basic_parameters()


TGeoVolume *make_single_tower(void *gpar)
{
  // FIXME: do it better than passing "void*" later; in fact should not hurt as long
  // as dereference the same class;
  HcalGeoParData *hcal = (HcalGeoParData*) gpar;
  //EicGeoParDataHelper *helper = (EicGeoParDataHelper*)qelper;
  const TString& cname = hcal->GetDetName()->Name();

  TGeoBBox *tower = new TGeoBBox(cname + "Tower", 
				 0.1 * hcal->mCellFaceSizeX/2,
				 0.1 * hcal->mCellFaceSizeY/2,
				 0.1 * hcal->mCellLength/2);

  // _NO_STRUCTURE_GEOMETRY_ can be defined in the macro where from hcal-lib.C is included;
#ifdef _NO_STRUCTURE_GEOMETRY_
  TGeoVolume *vtower = new TGeoVolume(cname + "Tower", tower, hcal->GetMedium("PbSciMix"));
#else
  TGeoVolume *vtower = new TGeoVolume(cname + "Tower", tower, hcal->GetMedium("air"));

  // Fill the tower with the essential stuff;
  {
    // Lead absorber plates;
    {
      TGeoBBox *leadPlate = new TGeoBBox(cname + "LeadPlate", 
					 0.1 * hcal->mLeadPlateWidth/2,
					 0.1 * hcal->mLeadPlateHeight/2,
					 0.1 * hcal->mLeadPlateThickness/2);
      TGeoVolume *vleadPlate = 
	new TGeoVolume(cname + "LeadPlate", leadPlate, hcal->GetMedium("lead"));

      // 2x2 mounting pins in the lead absorber;
      {
	// Yes, subtract steel divider plate thickness and share between two 
	// plates which this pin connects;
	double length = (hcal->mPinLength - hcal->mSteelSpacerThickness)/2;

	TGeoRotation *rpin = new TGeoRotation();
	rpin->RotateX(90.0);

	TGeoTube *steelPin = 
	  new TGeoTube(cname + "SteelPin", 0.0, 0.1 * hcal->mPinDiameter/2, 0.1 * length/2); 
	TGeoVolume *vsteelPin = 
	  new TGeoVolume(cname + "SteelPin", steelPin, hcal->GetMedium("iron"));
	
	for(unsigned lr=0; lr<2; lr++) {
	  double x = (lr ? 1.0 : -1.0) * hcal->mPinToPinDistance / 2;

	  for(unsigned tb=0; tb<2; tb++) {
	    double y = (tb ? 1.0 : -1.0) * (hcal->mLeadPlateHeight - length) / 2;

	    vleadPlate->AddNode(vsteelPin, lr*2 + tb, 
				new TGeoCombiTrans(0.1 * x, 0.1 * y,  0.0, rpin));
	  } //for tb
	} //for lr
      }
      
      // Prefer to decouple single last (steel) plate completely; pins are obviously 
      // not needed here :-)
      TGeoBBox *steelPlate = new TGeoBBox(cname + "RearSteelPlate", 
					  0.1 * hcal->mLeadPlateWidth/2,
					  0.1 * hcal->mLeadPlateHeight/2,
					  0.1 * hcal->mLeadPlateThickness/2);
      TGeoVolume *vsteelPlate = 
	new TGeoVolume(cname + "RearSteelPlate", steelPlate, hcal->GetMedium("iron"));

      // Assume shifts to-the-right and to-the-bottom;
      double offsetX = -(hcal->mCellFaceSizeX - hcal->mLeadPlateWidth)/2.;
      double offsetY = -(hcal->mCellFaceSizeY - hcal->mLeadPlateHeight)/2.;
	  
      // There are N-1 lead plates (and the last one is made of steel);
      for(unsigned pt=0; pt<hcal->mSubCellNum; pt++) {
	double offsetZ = -hcal->mCellLength/2. + 
	  (pt+1)*hcal->mSubCellLength - hcal->mLeadPlateThickness/2;
	
	if (pt == hcal->mSubCellNum-1)
	  vtower->AddNode(vsteelPlate, 0, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
							     0.1 * offsetZ, 0));
	else
	  vtower->AddNode(vleadPlate, pt, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
							     0.1 * offsetZ, 0));
      } //for pt
    }

    // Scintillator plates;
    {
      TGeoBBox *sciPlate = new TGeoBBox(cname + "ScintillatorPlate", 
					0.1 * hcal->mScintillatorPlateWidth/2,
					0.1 * hcal->mScintillatorPlateHeight/2,
					0.1 * hcal->mScintillatorPlateThickness/2);
      TGeoVolume *vsciPlate = 
	new TGeoVolume(cname + "ScintillatorPlate", sciPlate, hcal->GetMedium("polystyrene"));
      
      // Assume shifts to-the-center-gap (loose) and to-the-bottom;
      double offsetX = -(hcal->mCellFaceSizeX - hcal->mScintillatorPlateWidth)/2.;
      double offsetY = -(hcal->mCellFaceSizeY - hcal->mScintillatorPlateHeight)/2.;
      
      // There are N-1 lead plates (and the last one is made of steel);
      for(unsigned pt=0; pt<hcal->mSubCellNum; pt++) {
	// Assume shifts to-the-center-gap (loose);
	double offsetZ = -hcal->mCellLength/2. +
	  pt*hcal->mSubCellLength + (hcal->mSubCellLength - hcal->mLeadPlateThickness)/2;
	
	vtower->AddNode(vsciPlate, pt, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
							  0.1 * offsetZ, 0));
      } //for pt
    }
    
    // WLS plate, two mylar films and steel cover plate (long pieces with the same length);
    {
      TGeoBBox *wlsPlate = new TGeoBBox(cname + "WlsPlate", 
					0.1 * hcal->mWlsPlateThickness/2,
					0.1 * hcal->mWlsPlateHeight/2,
					0.1 * hcal->mWlsPlateLength/2);
      TGeoVolume *vwlsPlate = 
	// Assume polystyrene for now?;
	new TGeoVolume(cname + "WlsPlate", wlsPlate, hcal->GetMedium("polystyrene"));
      
      // Two mylar layers;
      TGeoBBox *mylarFilm = new TGeoBBox("mylarFilm", 
					 0.1 * hcal->mMylarThickness/2,
					 0.1 * hcal->mWlsPlateHeight/2,
					 0.1 * hcal->mWlsPlateLength/2);
      TGeoVolume *vmylarFilm = 
	// Assume kapton for now?;
	new TGeoVolume(cname + "MylarFilm", mylarFilm, hcal->GetMedium("kapton"));
      
      TGeoBBox *steelSpacer = new TGeoBBox(cname + "SteelSpacer", 
					   0.1 * hcal->mCellFaceSizeX/2,
					   0.1 * hcal->mSteelSpacerThickness/2,
					   0.1 * hcal->mWlsPlateLength/2);
      TGeoVolume *vsteelSpacer = 
	// Assume iron for now?;
	new TGeoVolume(cname + "SteelSpacer", steelSpacer, hcal->GetMedium("iron"));
      
      // Assume shifts to-the-center-gap (loose) and to-the-bottom;
      double offsetX = hcal->mCellFaceSizeX/2. - (hcal->mCellFaceSizeX - hcal->mLeadPlateWidth)/2.;
      double offsetY = -(hcal->mCellFaceSizeY - hcal->mWlsPlateHeight)/2.;
      double offsetZ = (hcal->mCellLength - hcal->mWlsPlateLength)/2.;
      
      vtower->AddNode(vwlsPlate, 0, new TGeoCombiTrans(0.1 * offsetX, 0.1 *offsetY, 
						       0.1 * offsetZ, 0));
      
      for(unsigned lr=0; lr<2; lr++) {
	// Well, may want to introduce an air gap later;
	double dx = (lr ? -1. : 1.)*(hcal->mWlsPlateThickness + hcal->mMylarThickness)/2;
	
	vtower->AddNode(vmylarFilm, lr, new TGeoCombiTrans(0.1 * (offsetX + dx), 
							   0.1 *offsetY, 0.1 * offsetZ, 0));
      } //for lr
      
      vtower->AddNode(vsteelSpacer, 0, 
		      new TGeoCombiTrans(0.0,
					 0.1 * (hcal->mCellFaceSizeY/2 - hcal->mSteelSpacerThickness/2),
					 0.1 * offsetZ, 0));
    }
  }
#endif

  return vtower;
} // make_single_tower()
