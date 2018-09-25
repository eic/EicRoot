
// Meaningless number for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

#define _NO_STRUCTURE_GEOMETRY_

// Full geometry seems to be impractical for purposes other than small test
// setup like 4x4 matrix; may want to represent the tower just as a single 
// WEpoxyMix layer of a proper thickness inbetween two thin epoxy endcaps;
//#define _IGNORE_BRASS_LAYERS_

#include <./femc-lib.C>

static double offsets[2] = {-1650., 2800.};//3400.+1130., 3400.};

femc()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  //
  // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
  // definition routines only;
  //

  for(unsigned fb=0; fb<2; fb++) {
    FemcGeoParData *emcal = (FemcGeoParData *)define_basic_parameters(fb ? "FEMC" : "BEMC", _VERSION_, _SUBVERSION_);
    const TString& cname = emcal->GetDetName()->Name();

    FiberParData *fptr = emcal->mFiber;
    TowerParData *tptr = emcal->mTower;

    // "No structure", "simple structure" & "full structure" configurations;
#ifdef _NO_STRUCTURE_GEOMETRY_
    emcal->SetGeometryType(EicGeoParData::NoStructure);
#else
#ifdef _IGNORE_BRASS_LAYERS_
    emcal->SetGeometryType(EicGeoParData::SimpleStructure);
#else
    emcal->SetGeometryType(EicGeoParData::FullStructure);
#endif
#endif

    //
    // Declare basic parameters;
    //


    // Endcap is shifted along H+ direction as a whole;
    double beamLineOffset         = offsets[fb];//-1650.0;//2800.;//2700.0;

    // Pick up whatever parameters for now;
    emcal->mEndcapMinR             =   20.0;
    emcal->mEndcapMaxTheta         =   40.0;
    // Do not allow subunits to protrude towards too small and too large radii;
    emcal->mSafetyVolume           =   10.0;
    
    // Well, assume no-gap packing?;
    emcal->mInterQuadrantGap       =    0.0;
    
    const Double_t endcapMaxR   = 
      fabs(beamLineOffset) * tan(emcal->mEndcapMaxTheta * TMath::Pi() / 180.);

    const Double_t envelopeWidth = emcal->mCellFaceSizeX + emcal->mInterCellGap;

    // Estimate an absolutely max number of rows;
    const Int_t rowsNumMax = (int)floor(endcapMaxR / envelopeWidth);
    //cout << rowsNumMax << endl;
    // Do not mind to use just this number in order to calculate XY-dimensions;
    // if the actual matrix will be a bit smaller, this does not really matter;
    //  emcal->SetDimX(2*rowsNumMax); emcal->SetDimY(2*rowsNumMax);
    //emcal->SetLogicalDimensions(0, 2*rowsNumMax, 2*rowsNumMax);
    emcal->AddLogicalVolumeGroup(2*rowsNumMax, 2*rowsNumMax);
    struct FemcRow {
      Double_t yc;
    } *rows = calloc(rowsNumMax, sizeof(FemcRow));
    
    for(int iq=0; iq<rowsNumMax; iq++)
      {
	FemcRow *row = rows + iq;
	
	row->yc = envelopeWidth * (iq + 0.5);
      } //for iq
    
    // Construct a single tower; this part is the same for T1018 and 
    // EIC model detector geometry;
    TGeoVolume *vtower = make_single_tower(emcal);
    
    // Assume the 4 quadrants are identical (like in PANDA); one quadrant is a 
    // 90 degree TUBS volume of known parameters;
    {
      // Yes, seems to be the easiest way to define them :-); NB: used in two places;
      TGeoRotation *_qquad[4];
      for(int ixy=0; ixy<4; ixy++)
	_qquad[ixy] =  new TGeoRotation(); 
      // NB: want [X][Y]-ordering among quadrants and 2x2 subunit crystals and no 
      // coordinate swapping -> reflections in favor of 90-degree rotations; 
      _qquad[0]->RotateZ(180); 
      _qquad[1]->ReflectX(kTRUE); 
      _qquad[2]->ReflectY(kTRUE);
      
      TGeoTubeSeg *quadrant = new TGeoTubeSeg(cname + "Quadrant", 
					      0.1 * emcal->mEndcapMinR,
					      0.1 *        endcapMaxR,
					      0.1 * tptr->mTowerShellLength/2 ,
					      0.0, 90.0);
      TGeoVolume *vquadrant = new TGeoVolume(cname + "Quadrant", quadrant, emcal->GetMedium("air"));
 
#ifdef _NO_STRUCTURE_GEOMETRY_
      EicGeoMap *fgmap = emcal->CreateNewMap();
      fgmap->AddGeantVolumeLevel(cname + "Tower",                0);
      fgmap->AddGeantVolumeLevel(cname + "TowerShell",           rowsNumMax*rowsNumMax);    
      fgmap->AddGeantVolumeLevel(cname + "Quadrant",             4);

    fgmap->SetSingleSensorContainerVolume(cname + "Tower");
#else
    for(unsigned iqq=0; iqq<fptr->GetLayerNum(); iqq++) {
      FiberTowerLayer *layer = fptr->GetLayer(iqq); 
      EicGeoMap *fgmap = emcal->CreateNewMap();

      fgmap->AddGeantVolumeLevel(layer->mFiberCoreName,     0);
      fgmap->AddGeantVolumeLevel(layer->mFiberCladdingName, 0);
      fgmap->AddGeantVolumeLevel(layer->mLayerName,         0);
      fgmap->AddGeantVolumeLevel(cname + "Tower",               0);
      fgmap->AddGeantVolumeLevel(cname + "TowerShell",          rowsNumMax*rowsNumMax);   
      fgmap->AddGeantVolumeLevel(cname + "Quadrant",            4);

      fgmap->SetSingleSensorContainerVolume(cname + "Tower");
    } //for iqq
#endif
    
    emcal->AddBlackHoleVolume(cname + "TowerShell");

    // Populate quadrant volume with towers; 
    unsigned tcounter = 0; 
    for(int ix=0; ix<rowsNumMax; ix++) {
      FemcRow *brx = rows + ix;
      // Yes, coordinates are remapped here; Z-coordinate will be just a focal distance;
      double xx = brx->yc;
    
      for(int iy=0; iy<rowsNumMax; iy++) {
	FemcRow *bry = rows + iy;
	double yy = bry->yc;
	
	// Calculate radial distance from (0,0); at the inner edge assume that can 
	// take center coordinates and give subunit diagonal dimension with some safety 
	// margin as a reference;
	double rr = sqrt(xx*xx + yy*yy);
	if (rr - emcal->mCellFaceSizeX/sqrt(2.) - emcal->mSafetyVolume < emcal->mEndcapMinR) continue;
	if (rr + emcal->mCellFaceSizeX/sqrt(2.) + emcal->mSafetyVolume >        endcapMaxR) continue;
	
	// Loop through 2x2 quadrants and add the 4 tower entries into the mapping table;
	for(int iqx=0; iqx<2; iqx++)
	  for(int iqy=0; iqy<2; iqy++) {
	    int XX = iqx ? rowsNumMax + ix : rowsNumMax - ix - 1;
	    int YY = iqy ? rowsNumMax + iy : rowsNumMax - iy - 1;
	    
	    // Determine 1D index in the mapping table; if fibers are not wanted, just 
	    // use the same array with an offset of 1;
	    UInt_t geant[6] = {0, 0, 0, 0, tcounter, 2*iqx + iqy}, group = 0, logical[2] = {XX, YY};
#ifdef _NO_STRUCTURE_GEOMETRY_
	    //if (fgmap->SetMappingTableEntry(id + 3, UGeo_t((XX << 16) | YY))) {
	    if (emcal->SetMappingTableEntry(fgmap, geant + 3, group, logical)) {
	      cout << "Failed to set mapping table entry!" << endl;
	      exit(0);
	    } //if
#else
	    for(unsigned iqq=0; iqq<fptr->GetLayerNum(); iqq++) {
	      EicGeoMap *fgmap = emcal->GetMapPtrViaMapID(iqq);
	      
	      //if (fgmap->SetMappingTableEntry(id, UGeo_t((XX << 16) | YY))) {
	      if (emcal->SetMappingTableEntry(fgmap, geant, group, logical)) {
		cout << "Failed to set mapping table entry!" << endl;
		exit(0);
	      } //if
	    } //for iqq
#endif
	  } //for iqx..iqy 
      
	{
	  TGeoRotation *rt = new TGeoRotation();
	  // Want to mimic 2x1 tower assembly layout of fibers; so "swap" all towers with 
	  // odd X-direction index; actually 180 degree Z-rotation of "new" tower layout 
	  // should not make any effect;
#ifdef _ASSUME_TWO_TOWER_ASSEMBLIES_
	  if (ix%2) rt->RotateZ(180); 
#endif

	  vquadrant->AddNode(vtower, tcounter++, 
			     new TGeoCombiTrans(0.1 * xx, 0.1 * yy, 0.0, rt));
	}
      } //for iy
    } //for ix 

    printf("%5d towers per quadrant\n", tcounter);

    // Locate 4 quadrant copies in the "FEMC" volume;
    for(int ixy=0; ixy<4; ixy++) {
      Double_t local[4] = {emcal->mInterQuadrantGap/2, emcal->mInterQuadrantGap/2, 0, 0}, master[4];
    
      _qquad[ixy]->LocalToMaster(local, master);
      
      emcal->GetTopVolume()->AddNode(vquadrant, ixy, 
				    new TGeoCombiTrans(0.1 * master[0], 0.1 * master[1], 0.0, _qquad[ixy]));
    } //for ixy
  } 
  
  // And put this stuff as a whole into the top volume; account Z-shift;
  {
    TGeoRotation *rw = new TGeoRotation();

    if (beamLineOffset < 0.0) rw->RotateY(180); 

    //emcal->FinalizeOutput(new TGeoCombiTrans(0., 0., 0.1 * emcal->mBeamLineOffset, rw));
    emcal->SetTopVolumeTransformation(new TGeoCombiTrans(0.0, 0.0, 0.1 * beamLineOffset, rw));
  }

  //emcal->GetColorTable()->AddPatternMatch(cname + "Tower", kBlue);
  emcal->GetColorTable()->AddPatternMatch(cname + "Quadrant", kBlue);

  emcal->FinalizeOutput();
  } //for fb

  // Yes, always exit;
  exit(0);
}

