
//
//  Think twice before changing anything here; and always check overlaps in GEANT 
//  geometry after doing modifications;
//

// May want to fix X- and/or Y-spacings to a fixed value (June'2013 5x5cm^2 design);
#define _FIBER_X_SPACING_ (0.0376 * 25.4)
#define _FIBER_Y_SPACING_ (0.0328 * 25.4)

// May want to move outer fiber inwards; this value determines min. distance 
// from fiber *center* to the mesh edge in either X or Y; June'2013 design: 
// 0.0220" diameter holes and min. metal width 0.0108" at the edge;
#define _MIN_DISTANCE_TO_EDGE_  ((0.0110 + 0.0108)*25.4)

// Towers will be put individually in the geometry; however if only 2-tower
// assemblies are produces (or 2x2 ones), one does not need to move inwards 
// holes on the "right" tower side; see also an extra 180 degree Z-rotation 
// for odd in X towers in the code; 
#define _ASSUME_TWO_TOWER_ASSEMBLIES_

// Again, do this pointer passing better later;
void *define_basic_parameters(const char *detName, int version, int subVersion)
{
  // NB: number of maps will be defined later (depending on tower segmentation);
  FemcGeoParData *gpar = new FemcGeoParData(detName, version, subVersion);
  
  gpar->mType                  = CalorimeterGeoParData::Fiber;

  //
  //   For automatic mesh parameters calculation assume that a small difference 
  // between 26*U = 26.00 units (X) and 30*sqrt(3)/2*U ~ 25.98 units (Y) does 
  // not matter, so one can still cook towers with square cross-section 
  // without destroying honeycomb pattern too much;
  //
  //   Indeed the parameters can also be fixed to some predefined numbers 
  // (like mesh spacing in X & Y rounded to 0.0001");
  //
  
  // Present Oleg's design of a 5x5cm^2 mesh shared between 2x2 towers;
  gpar->mCellFaceSizeX         =  1.9684 * 25.4 / 2; 
  // Yes, assume square section and just use gpar->mCellFaceSizeX in all 'critical' 
  // places like radius calculation; NB: code modifications will be needed if XY-sizes
  // ever become different!;
  gpar->mCellFaceSizeY         = gpar->mCellFaceSizeX;

  // Well, assume no-gap packing;
  gpar->mInterCellGap          =    0.0; 

#ifndef _NO_STRUCTURE_GEOMETRY_
  FiberParData *fptr = gpar->mFiber = new FiberParData();

  // Triangular honeycomb grid;
  fptr->mFiberNumPerRow         =     26; 
  fptr->mFiberRowNum            =     30;
  // Fiber parameters; 470um core diameter; 
  fptr->mFiberCoreDiameter      =    0.47;
  fptr->mFiberCladdingThickness =    0.03/2;
  fptr->mOuterDiameter          = fptr->mFiberCoreDiameter + 2*fptr->mFiberCladdingThickness;

#ifdef _FIBER_X_SPACING_
  fptr->mFiberSpacingX          = _FIBER_X_SPACING_;
#else
  fptr->mFiberSpacingX          = gpar->mCellFaceSizeX / fptr->mFiberNumPerRow;
#endif
  // NB: want a ~honeycomb structure -> even and odd rows fill be offset 
  // by fptr->mFiberSpacingX/2; 
  fptr->mFiberX0offset          = -gpar->mCellFaceSizeX/2 + 
    (gpar->mCellFaceSizeX - (fptr->mFiberSpacingX/2)*(2*fptr->mFiberNumPerRow - 1))/2;

#ifdef _FIBER_Y_SPACING_
  fptr->mFiberSpacingY          = _FIBER_Y_SPACING_;
#else
  fptr->mFiberSpacingY          = fptr->mFiberSpacingX*sqrt(3.)/2;
#endif
  fptr->mFiberY0offset          = -gpar->mCellFaceSizeY/2 + 
    (gpar->mCellFaceSizeY - fptr->mFiberSpacingY*(fptr->mFiberRowNum - 1))/2;
#endif

  TowerParData *tptr = gpar->mTower = new TowerParData();

  tptr->mLightGuideUpstreamWidth =  13.0;
  tptr->mLightGuideLength        =  25.4;
  // This should sort of match the internal structure as well as the alu box (for T1018 case);
  tptr->mTowerShellLength        = 202.0;//200.0;
  tptr->mSiliconPadThickness     =   1.7;

  tptr->mSensorWidth             =   3.0;
      // Sensor thickness should nto matter here I guess?;
  tptr->mSensorThickness         =   0.1;
  tptr->mSensorToSensorDistance  =   0.275 * 25.4;

  tptr->mG10Thickness            =   1.6;
  tptr->mG10Width                =   0.787 * 25.4;

  return gpar;
} // define_basic_parameters()


TGeoVolume *make_single_tower(void *qpar)
{
  // FIXME: do it better than passing "void*" later; in fact should not hurt as long
  // as dereference the same class;
  FemcGeoParData *gpar = (FemcGeoParData*) qpar;
  //EicGeoParDataHelper *helper = (EicGeoParDataHelper*)qelper;
  const TString& cname = gpar->GetDetName()->Name();

  //
  // So try to build a precise composition; this includes several tiny volumes, so 
  // it totally inappropriate for a real physics simulation; but there it should 
  // not matter, since most likely dummy volumes will be used anyway + fast digi of
  // whatever sort;
  //

  TowerParData *tptr = gpar->mTower;
  
#ifdef _NO_STRUCTURE_GEOMETRY_
  // Yes, prefer to fix cell length in this case;
  gpar->mCellLength = 200.0;
#else
  FiberParData *fptr = gpar->mFiber;

  // Do not be completely pedantic; assume outer brass meshes are right at the 
  // epoxy-to-absorber border; thickness is 0.012" ~ 300um; also assume, that 
  // exact locations of meshes inside the tower do not matter, but I should arrive 
  // at at total length of 3.4+165+2.6 = 171mm; since brass mesh thickness and 
  // two pure epoxy layer thickness is given correctly, this should give correct
  // rad.length which is the only thing which matters;
  fptr->AddLayer(3.40,         "Epoxy"); 
#ifdef _IGNORE_BRASS_LAYERS_
  // Just a single WEpoxyMix layer with the overall length matching real setup;
  fptr->AddLayer(6.498 * 25.4, "WEpoxyMix");
#else
  fptr->AddLayer(0.012 * 25.4, "brass");
  fptr->AddLayer(1.690 * 25.4, "WEpoxyMix");
  fptr->AddLayer(0.012 * 25.4, "brass");
  fptr->AddLayer(3.380 * 25.4, "WEpoxyMix");
  fptr->AddLayer(0.012 * 25.4, "brass");
  fptr->AddLayer(1.380 * 25.4, "WEpoxyMix");
  fptr->AddLayer(0.012 * 25.4, "brass");
  // Add these two layers to become ~200mm thick (?);
  //fptr->AddLayer(1.328 * 25.4, "WEpoxyMix");
  //fptr->AddLayer(0.012 * 25.4, "brass");
#endif
  fptr->AddLayer(2.60,         "Epoxy");

  // Calculate layer offsets; this gives overall cell length at the end;
  for(unsigned iq=0; iq<fptr->GetLayerNum(); iq++) {
    FiberTowerLayer *layer = fptr->GetLayer(iq);

    layer->mOffset     = gpar->mCellLength;
    gpar->mCellLength += layer->mThickness;
  } //for iq
#endif
  printf("Cell length: %7.1f mm\n", gpar->mCellLength);

  // Single tower, filled with layers of material with fibers;
  TGeoBBox *tower = new TGeoBBox(cname + "Tower", 
				 0.1 * gpar->mCellFaceSizeX/2,
				 0.1 * gpar->mCellFaceSizeY/2,
				 0.1 * gpar->mCellLength/2);

  // Unless a dummy geometry wanted, populate tower with material layers and fibers;
  // NB: optical photon propagation does not make much sense for spaghetti calorimeter 
  // (attenuation length and average photon yield per GeV are *measured*, the rest 
  // can be simulated on energy deposit level), so cutting fibers in pieces does not hurt;
  // NB: light guide optical simulation indeed makes sense, but one can do it with 
  // a last short fiber piece as a light source;
  TGeoVolume *vtower = 
#ifdef _NO_STRUCTURE_GEOMETRY_
    // Explicitely put SciFiber/W/epoxy mix with approximately correct density;
    new TGeoVolume(cname + "Tower", tower, gpar->GetMedium("WEpoxySciMix"));
#else
    // Material does not really matter here (the whole volume will be filled 
    // by other stuff anyway); prefer to put just air for clarity;
    new TGeoVolume(cname + "Tower", tower, gpar->GetMedium("air"));

  for(unsigned iq=0; iq<fptr->GetLayerNum(); iq++) {
    FiberTowerLayer *layer = fptr->GetLayer(iq);

    char layerName[128], claddingName[128], coreName[128];

    sprintf(layerName,    "%sTowerLayer%02d",    cname.Data(), iq);
    sprintf(claddingName, "%sFiberCladding%02d", cname.Data(), iq);
    sprintf(coreName,     "%sFiberCore%02d",     cname.Data(), iq);

    layer->SetLayerNames(layerName, claddingName, coreName);

    // Indeed assume holes in brass layers are equal to fiber diameter; 
    TGeoBBox *ltower = new TGeoBBox(layerName,
				    0.1 * gpar->mCellFaceSizeX/2,
				    0.1 * gpar->mCellFaceSizeY/2,
				    0.1 * layer->mThickness/2);
    TGeoVolume *vltower = 
      new TGeoVolume(layerName, ltower, gpar->GetMedium(layer->mMedia.Data()));

    vtower->AddNode(vltower, 0, 
		    new TGeoCombiTrans(0.0, 0.0, 
				       0.1 * (layer->mOffset + layer->mThickness/2 - gpar->mCellLength/2), 0));

    TGeoTube *fiber = new TGeoTube(claddingName, 0.0, 0.1 * fptr->mOuterDiameter/2, 
				   0.1 * layer->mThickness/2); 
    TGeoVolume *vfiber = new TGeoVolume(claddingName, fiber, gpar->GetMedium("PMMA"));

    for(int ix=0; ix<fptr->mFiberNumPerRow; ix++)
      for(int iy=0; iy<fptr->mFiberRowNum; iy++) {
	double xx = fptr->mFiberX0offset + (ix+(iy%2)/2.)*fptr->mFiberSpacingX;
	double yy = fptr->mFiberY0offset + iy*fptr->mFiberSpacingY;

	// In fact would not hurt to always do this check and correct edge fiber locations;
#ifdef _MIN_DISTANCE_TO_EDGE_
	double x2edge = gpar->mCellFaceSizeX/2 - fabs(xx), y2edge = gpar->mCellFaceSizeY/2 - fabs(yy);
#ifdef _ASSUME_TWO_TOWER_ASSEMBLIES_
	// In this case fix "left" side only;
	if (x2edge < _MIN_DISTANCE_TO_EDGE_ && xx < 0.0) 
#else
	if (x2edge < _MIN_DISTANCE_TO_EDGE_) 
#endif
	  xx = (xx < 0. ? -1. : 1.)*(gpar->mCellFaceSizeX/2 - _MIN_DISTANCE_TO_EDGE_);
	if (y2edge < _MIN_DISTANCE_TO_EDGE_) 
	  yy = (yy < 0. ? -1. : 1.)*(gpar->mCellFaceSizeY/2 - _MIN_DISTANCE_TO_EDGE_);
#endif
	//printf("%02d, %02d -> %8.4f %8.4f ... %8.4f %8.4f ... %8.4f %8.4f\n", ix, iy, xx, yy, 
	//   (xx + gpar->cellFaceSize/2)/25.4,
	//   (yy + gpar->cellFaceSize/2)/25.4,
	//   (xx - gpar->cellFaceSize/2)/25.4,
	//   (yy - gpar->cellFaceSize/2)/25.4);
	
	vltower->AddNode(vfiber, ix*fptr->mFiberRowNum + iy, 
			 new TGeoCombiTrans(0.1 * xx, 0.1 * yy, 0.0, new TGeoRotation()));
      } //for ix..iy
    
    TGeoTube *fcore = new TGeoTube(coreName, 0.0, 0.1 * fptr->mFiberCoreDiameter/2, 
				   0.1 * layer->mThickness/2);
    TGeoVolume *vfcore = new TGeoVolume(coreName, fcore, gpar->GetMedium("polystyrene"));
    vfiber->AddNode(vfcore, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));
  } //for iq
#endif

  // And a container shell which also includes lucite pyramid, PCB with sensors, etc;
  {
    // Single tower, filled with layers of material with fibers;
    TGeoBBox *shell = new TGeoBBox(cname + "TowerShell", 
				   0.1 * gpar->mCellFaceSizeX/2,
				   0.1 * gpar->mCellFaceSizeY/2,
				   0.1 * tptr->mTowerShellLength/2);
    TGeoVolume *vshell = new TGeoVolume(cname + "TowerShell", shell, gpar->GetMedium("air"));

#ifndef _NO_STRUCTURE_GEOMETRY_
    double lightGuideOffset, siliconPadOffset, sensorOffset, g10Offset;

    // Add light guide;
    {
      TGeoTrd2 *guide = new TGeoTrd2(cname + "TowerLightGuide", 
				     0.1 * tptr->mLightGuideUpstreamWidth/2,
				     0.1 * gpar->mCellFaceSizeX/2,
				     0.1 * tptr->mLightGuideUpstreamWidth/2,
				     0.1 * gpar->mCellFaceSizeY/2,
				     0.1 * tptr->mLightGuideLength/2);

      TGeoVolume *vguide = new TGeoVolume(cname + "TowerLightGuide", guide, gpar->GetMedium("lucite"));

      lightGuideOffset = (tptr->mTowerShellLength - 2 * gpar->mCellLength - tptr->mLightGuideLength)/2;

      vshell->AddNode(vguide, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * lightGuideOffset, 0));
    }

    // Silicon layer between light guide and sensors;
    {
      // Single tower, filled with layers of material with fibers;
      TGeoBBox *spad = new TGeoBBox(cname + "TowerSiliconPad", 
				    0.1 * tptr->mLightGuideUpstreamWidth/2,
				    0.1 * tptr->mLightGuideUpstreamWidth/2,
				    0.1 * tptr->mSiliconPadThickness/2);
      TGeoVolume *vspad = 
	new TGeoVolume(cname + "TowerSiliconPad", spad, gpar->GetMedium("SiliconeResinD"));

      siliconPadOffset = lightGuideOffset - tptr->mLightGuideLength/2 - tptr->mSiliconPadThickness/2;

      vshell->AddNode(vspad, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * siliconPadOffset, 0));
    }
    
    // 2x2 sensor volumes (in case somebody wants to do optical calculations later :-);
    {
      // Single tower, filled with layers of material with fibers;
      TGeoBBox *sensor = new TGeoBBox(cname + "SiliconSensor", 
				     0.1 * tptr->mSensorWidth/2,
				     0.1 * tptr->mSensorWidth/2,
				     0.1 * tptr->mSensorThickness/2);
      TGeoVolume *vsensor = 
	new TGeoVolume(cname + "SiliconSensor", sensor, gpar->GetMedium("silicon"));

      sensorOffset = siliconPadOffset - tptr->mSiliconPadThickness/2 - tptr->mSensorThickness/2;
      
      for(unsigned iqx=0; iqx<2; iqx++) {
	double xOffset = (iqx ? -1.0 : 1.0) * tptr->mSensorToSensorDistance/2;

	for(unsigned iqy=0; iqy<2; iqy++) {
	  double yOffset = (iqy ? -1.0 : 1.0) * tptr->mSensorToSensorDistance/2;

	  vshell->AddNode(vsensor, iqx*2+iqy, 
			  new TGeoCombiTrans(0.1 * xOffset, 0.1 * yOffset, 0.1 * sensorOffset, 0));
	} //for iqy
      } //for iqx
    }  

    // Add G10 assembly (sort of equivalent thickness);
    {
      // Single tower, filled with layers of material with fibers;
      TGeoBBox *g10 = new TGeoBBox(cname + "SiliconSensorG10", 
				   0.1 * tptr->mG10Width/2,
				   0.1 * tptr->mG10Width/2,
				   0.1 * tptr->mG10Thickness/2);
      TGeoVolume *vg10 = 
	new TGeoVolume(cname + "SiliconSensorG10", g10, gpar->GetMedium("G10"));

      g10Offset = sensorOffset - tptr->mSensorThickness/2 - tptr->mG10Thickness/2;

      vshell->AddNode(vg10, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * g10Offset, 0));
    } 
#endif

    // Add tower itself;
    double towerOffset = (tptr->mTowerShellLength - gpar->mCellLength)/2;
    vshell->AddNode(vtower, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * towerOffset, 0));
  } 

  return vshell;
} // make_single_tower()
