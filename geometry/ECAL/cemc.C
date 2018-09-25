
// Meaningless numbers for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

//
// FIXME: rather arbitrary tower construction; will have to fix later;
//


#define _NO_STRUCTURE_GEOMETRY_

//
// -> for now it is "ideal" for narrow side fiber-to-fiber spacing, but 
//    no "safety" correction; prefer not to mess up with ../FEMC/femc-lib.C;
//    once geometry gets well defined, unify similar parts;
//

{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  //
  // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
  // definition routines only;
  //

  // Allocate as many logical mapping tables as needed;
  CemcGeoParData *cemc = new CemcGeoParData(_VERSION_, _SUBVERSION_);

  // "No structure" & "simple structure" configurations (compared to FEMC, 
  // so so brass meshes, etc);
#ifdef _NO_STRUCTURE_GEOMETRY_
  cemc->SetGeometryType(EicGeoParData::NoStructure);
#else
  cemc->SetGeometryType(EicGeoParData::SimpleStructure);
#endif

  //
  // Declare basic parameters;
  //

  cemc->mType                   = CalorimeterGeoParData::Fiber;

  cemc->mCellFaceSizeX          =   1.049 * 25.4;
  // Well, at present force XY-sizes to be equal;
  double cellFaceSize           = cemc->mCellFaceSizeY = cemc->mCellFaceSizeX;
  cemc->mCellLength             =  200.0;//166.0;
  // Well, assume no-gap packing;
  cemc->mInterCellGap           =    0.0;

  // No shift along the beam line for now;
  double beamLineOffset         =    1.049 * 25.4 * 2;//0.0;

  // Default is kFALSE (no fibers, so tower material is a sensitive volume);
#ifndef _NO_STRUCTURE_GEOMETRY_
  FiberParData *fptr = cemc->mFiber = new FiberParData();

  // Triangular honeycomb grid;
  fptr->mFiberNumPerRow         =     26; 
  fptr->mFiberRowNum            =     30;
  // Fiber parameters; 470um core diameter; 
  fptr->mFiberCoreDiameter      =    0.47;
  fptr->mFiberCladdingThickness =    0.03/2;
  double outerDiameter         = fptr->mFiberCoreDiameter + 2*fptr->mFiberCladdingThickness;

  // Define fiber X-spacing; 
#ifdef _FIBER_X_SPACING_
  fptr->mFiberSpacingX          = _FIBER_X_SPACING_;
#endif
  // Yes, this is the "ideal" number; use it only if not assigned yet;
  if (!fptr->mFiberSpacingX)
    fptr->mFiberSpacingX        = cemc->mCellFaceSizeX / fptr->mFiberNumPerRow;

  // NB: want a ~honeycomb structure -> even and odd rows fill be offset 
  // by fptr->mFiberSpacingX/2; 
  double fiberX0offset = -cemc->mCellFaceSizeX/2 + 
    (cemc->mCellFaceSizeX - (fptr->mFiberSpacingX/2)*(2*fptr->mFiberNumPerRow - 1))/2;

  // Define fiber Y-spacing; also tricky;
#ifdef _FIBER_Y_SPACING_
  fptr->mFiberSpacingY = _FIBER_Y_SPACING_;
#endif
  // Unless predefined, calculate as ideal honeycomb;
  if (!fptr->mFiberSpacingY)
    fptr->mFiberSpacingY = fptr->mFiberSpacingX*sqrt(3.)/2;
  //cout << fptr->mFiberSpacingY / 25.4 << " " << fptr->mFiberSpacingY << endl;
  double fiberY0offset = -cemc->mCellFaceSizeY/2 + 
    (cemc->mCellFaceSizeY - fptr->mFiberSpacingY*(fptr->mFiberRowNum - 1))/2;

  // Fibers will be a bit rotated inside tower volumes in order to follow
  // tower wedge shape; want to avoid idiotic problems with few um overlapping 
  // volumes; so make fibers shorter; in fact ideally would need to have CTUB 
  // volumes of a bit different length (inclination-dependent); think later;
  double fiberLength   = sqrt(cemc->mCellLength*cemc->mCellLength - outerDiameter*outerDiameter/4);
#endif

  // Tune this number in such a way that the whole calorimeter fit in the radial
  // distance range [1120..1320]mm or so; assume one tower per sector for now (no grouping); 
  // may want to change later;
  double sectorNum             =    268;//192;
  // Tune this number to cover a certain length along the beam line;
  double sliceNum              =    120-4;// 92;
  // Prefer to work with sectorNum and sliceNum variables; so just define the 
  // XZ-dimensions and forget about them;
  //cemc->SetDimX(sectorNum); cemc->SetDimZ(sliceNum);
  //cemc->SetLogicalDimensions(0, sectorNum, 0, sliceNum);//2*rowsNumMax, 2*rowsNumMax);
  cemc->AddLogicalVolumeGroup(sectorNum, 0, sliceNum);

  // Calculate the inner radius of sector container volume;
  const Double_t sectorAlpha          = 360. / sectorNum;
  const Double_t sectorMinR           = ((cellFaceSize + cemc->mInterCellGap)/2) * 
    tan((90 - sectorAlpha / 2) * TMath::Pi() / 180.);
  cout << sectorMinR << endl;
  // Outer radius of sector container volume;
  const Double_t sectorMaxR           = (sectorMinR + cemc->mCellLength) / 
    cos((sectorAlpha / 2) * TMath::Pi() / 180.);  
  cout << sectorMaxR << endl;

  // Sector length along the beam line;
  const Double_t sliceStep            = cellFaceSize + cemc->mInterCellGap;
  const Double_t sectorLength         = sliceNum * sliceStep;

  // Tower rear side length in azimuthal direction; assume side slope is 
  // just alfa/2, for simplicity; NB: dimension in Z-direction is of course
  // equal to cemc->mCellFaceSize (non-projective geometry, no tricks for now);
  const Double_t towerRearSideSize    = cellFaceSize + 2.*cemc->mCellLength* 
    tan((sectorAlpha / 2) * TMath::Pi() / 180.); 
  cout << towerRearSideSize << endl;
#ifndef _NO_STRUCTURE_GEOMETRY_
  // Otherwise this variable is of no use; fiber XY-coordinates will be 
  // calculated on a short (26x26mm^2) front side; one of these coordinates 
  // need to be stretched a bit in order to account for wedge tower shape;
  double scalingFactor = (towerRearSideSize + cellFaceSize)/(2*cellFaceSize);
  cout << scalingFactor << endl;
#endif

  // Air container volume; will hold one slice in "phi" with the full Z-length;
  TGeoTubeSeg *tubs = new TGeoTubeSeg("CemcSector", 
  				      0.1 * sectorMinR, 
  				      0.1 * sectorMaxR, 
  				      0.1 * sectorLength/2., 
				      -sectorAlpha/2., sectorAlpha/2.);
  TGeoVolume *vtubs = new TGeoVolume("CemcSector", tubs, cemc->GetMedium("air"));

  // A single tower; prefer the easiest (TRD1) shape and GEANT3 rot.matrix declaration;
  {
    TGeoTrd1 *tower = new TGeoTrd1("CemcTower", 
				   0.1 * cellFaceSize/2, 
				   0.1 * towerRearSideSize/2, 
				   0.1 * cellFaceSize/2, 
				   0.1 * cemc->mCellLength/2);
    TGeoVolume *vtower = 
      new TGeoVolume("CemcTower", tower, cemc->GetMedium(cemc->mFiber ? "WEpoxyMix" : "WEpoxySciMix"));
    TGeoRotation *rr = new TGeoRotation();
    rr->SetAngles(90., 90., 0., 0., 90., 0.);
    for(int tw=0; tw<sliceNum; tw++)
      vtubs->AddNode(vtower, tw, 
		     new TGeoCombiTrans(0.1 * (sectorMinR + cemc->mCellLength/2), 0., 
					0.1 * sliceStep * (tw - (sliceNum-1)/2.), rr));

#ifndef _NO_STRUCTURE_GEOMETRY_
    // Populate tower with fibers; assume they can all be taken with the same length
    // for simplicity (which would mean some are few hundred um short); fix later if needed;
    // eventually will have to take individual fiber slopes;
    TGeoTube *fiber = new TGeoTube("CemcFiberCladding", 0.0, 0.1 * outerDiameter/2, 0.1 * fiberLength/2); 
    TGeoVolume *vfiber = new TGeoVolume("CemcFiberCladding", fiber, cemc->GetMedium("PMMA"));

    for(int ix=0; ix<fptr->mFiberNumPerRow; ix++)
    //for(int ix=1; ix<cemc->mFiberNumPerRow-1; ix++)
      for(int iy=0; iy<fptr->mFiberRowNum; iy++)
      {
	//printf("%02d, %02d -> %7.2f %7.2f\n", ix, iy, 
	//     fiberX0offset + (ix+(iy%2)/2.)*cemc->mFiberSpacing, fiberY0offset + iy*fiberYpitch);

	// Calculate the rotation angle wrt the Y axis;
	TGeoRotation *frr = new TGeoRotation();
	double x0 = fiberX0offset + (ix+(iy%2)/2.)*fptr->mFiberSpacingX;
	double angle = atan(x0*(scalingFactor-1.)/(fiberLength/2.)) * 180. / TMath::Pi();
	//cout << angle << endl;
	frr->SetAngles(90.+angle, 0., 90., 90., angle, 0.);
	
	vtower->AddNode(vfiber, ix*fptr->mFiberRowNum + iy, 
			new TGeoCombiTrans(0.1 * x0 * scalingFactor,
					   0.1 * (fiberY0offset + iy*fptr->mFiberSpacingY),
					   0.0, frr));
      } //for ix..iy

    TGeoTube *fcore = new TGeoTube("CemcFiberCore", 0.0, 0.1 * fptr->mFiberCoreDiameter/2, 
				   0.1 * fiberLength/2); 
    TGeoVolume *vfcore = new TGeoVolume("CemcFiberCore", fcore, cemc->GetMedium("polystyrene"));
    vfiber->AddNode(vfcore, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, new TGeoRotation()));
#endif
  }

  // Configure tower map;
  EicGeoMap *fgmap = cemc->CreateNewMap();
#ifndef _NO_STRUCTURE_GEOMETRY_
  fgmap->AddGeantVolumeLevel("CemcFiberCore",     0);
  fgmap->AddGeantVolumeLevel("CemcFiberCladding", 0);
#endif
  fgmap->AddGeantVolumeLevel("CemcTower",         sliceNum);
  fgmap->AddGeantVolumeLevel("CemcSector",        sectorNum);

  fgmap->SetSingleSensorContainerVolume("CemcTower");
  cemc->AddBlackHoleVolume("CemcSector");

  for(UInt_t sc=0; sc<sectorNum; sc++)
    for(UInt_t iz=0; iz<sliceNum; iz++)
    {
      // Yes, in this order (iz,sc);
      UInt_t geant[4] = {0, 0, iz, sc}, group = 0, logical[3] = {sc, 0, iz};

      if (cemc->SetMappingTableEntry(fgmap, geant + (cemc->mFiber ? 0 : 2), group, logical))
      {
	cout << "Failed to set mapping table entry!" << endl;
	exit(0);
      } //if
    } //for ix..iz

  // Put all sector copies;
  for(UInt_t sc=0; sc<sectorNum; sc++)
  {
    TGeoRotation *qr = new TGeoRotation();
    qr->SetAngles(sc * sectorAlpha, 0., 0.);

    cemc->GetTopVolume()->AddNode(vtubs, sc, new TGeoCombiTrans(0., 0., 0., qr));
  } //for sc

  cemc->SetTopVolumeTransformation(new TGeoTranslation(0.0, 0.0, 0.1 * beamLineOffset));

  //cemc->GetColorTable()->AddPatternMatch("CemcTower", kBlue);
  cemc->GetColorTable()->AddPatternMatch("CemcSector", kBlue);

  // And put this stuff as a whole into the top volume; account Z-shift;
  cemc->FinalizeOutput();

  // Yes, always exit;
  exit(0);
}
