//
// AYK (ayk@bnl.gov), 2014/08/07
//
//  FST MAPS geometry description file;
//

#include <cmath>
#include <assert.h>

#include <TGeoTube.h>
#include <FstGeoParData.h>

//
// FIXME:
//  - mapping table missing;
//  - staves need to be done as TGeoCompositeShape to come close to the beam pipe;
//  - half of the staves should be Y-rotated by 180 degrees to bring chips close to beam pipe;
//  - if TGeoCompositeShape does not work, beam pipe diameter should be made ~2mm smaller
//  - pipe pieces should be oriented correctly;
// 

// ---------------------------------------------------------------------------------------

int FstGeoParData::ConstructGeometry()
{
  // Running variables used to give unique indices to all staves; and to count chips;
  // FIXME: may want to put these into MapsGeoParData?!;
  unsigned staveGlobalCounter = 0, chipGlobalCounter = 0;

#if _NOW_
  SetLogicalDimensions(GetNumberOfLayers(), maxStaveNumPerLayer, 0, maxChipNumPerStave);
#endif

  for(unsigned dc=0; dc<mDiscs.size(); dc++) {
    FstDisc *disc = mDiscs[dc];
    MapsMimosaAssembly *mcell = disc->mChipAssembly;
    char mountingRingName[128], discName[128];
    const char *detName = mDetName->Name().Data();

    // Used to identify odd and even stave indices inside one disc;
    unsigned staveLocalCounter = 0, nChipsMax = 0;
    
    double staveWidth = GetAssemblyContainerWidth(mcell);
    
    snprintf(discName,         128-1, "%sContainerVolume%02d", detName, dc);
    snprintf(mountingRingName, 128-1, "%sMountingRing%02d",    detName, dc);
 
    // Cook a TGeoTube air container volume;
    TGeoTube *mdisc = new TGeoTube(discName,
				   0.1 * disc->mMinRadius,
				   0.1 * disc->mMaxRadius,
				   0.1 * (mMountingRingBeamLineThickness + 2*mcell->GetAssemblyHeight())/2);
    TGeoVolume *vdisc = new TGeoVolume(discName, mdisc, GetMedium(_AIR_));
    GetTopVolume()->AddNode(vdisc, 0, disc->mTransformation);

    {
      // Calculate stave configuration based on min/max radius, min stave-to-stave 
      // overlap and single stave size; NB: assume, that stave assembly width and height 
      // determine the fiducial volume (not exactly intelligent);
      double hOffset = disc->mMinRadius + staveWidth/2;
    
      // Place new staves as long as do not run out of horizontal space;
      for( ; ; ) {
	if (hOffset + mcell->mAssemblyBaseWidth/2 > disc->mMaxRadius) break;
	
	// See how many chips can put on a stave at thi hOffset;
	{
	  unsigned nChips;
	  
	  for(nChips=0; ; nChips++) {
	    double staveLength = GetExpectedStaveLength(nChips+1, mcell);
	    
	    double xx = hOffset + staveWidth/2;
	    double yy = staveLength/2;
	    double rr = sqrt(xx*xx + yy*yy);
	    
	    if (rr > disc->mMaxRadius) break;
	  } //for n
	  
	  if (nChips > nChipsMax) nChipsMax = nChips;

	  //printf("%7.2f -> %3d\n", hOffset, nChips);
	  if (!nChips) break;
	  
	  // Add a stave with given number of chips at this location;
	  {    
	    //MapsStave *stave = ConstructStave(nChips, staveGlobalCounter++, mcell);
	    
	    //AddStaveMappingTable(stave);

	    bool odd = (staveLocalCounter++)%2;
	    	    
	    double zOffset = (odd ? -1.0 : 1.0)*
	      (mMountingRingBeamLineThickness + mcell->GetAssemblyHeight())/2;
	    //printf("%d\n", staveLocalCounter);
	    for(unsigned lr=0; lr<2; lr++) {
		TGeoRotation *rw;

	      if      (!lr && !odd) 
		// NB: default (!lr && !odd): no rotation;
		rw = 0;
	      else {
		rw = new TGeoRotation();

		if (     !lr &&  odd)
		  rw->SetAngles(90.0,   0.0,  90.0, 270.0, 180.0,  0.0);
		else if ( lr && !odd) 
		  rw->SetAngles(90.0, 180.0,  90.0, 270.0,   0.0,  0.0);
		else if ( lr &&  odd) 
		  rw->SetAngles(90.0, 180.0,  90.0,  90.0, 180.0,  0.0);
	      } //if

	      chipGlobalCounter += nChips;
	      // Yes, for now define *all* staves independently; later on group them by chip number;
	      MapsStave *stave = ConstructStaveWithMapping(nChips, staveGlobalCounter++, mcell);
	      vdisc->AddNode(stave->GetVolume(), /*lr*/0, new TGeoCombiTrans(0.1 * (lr ? -1.0 : 1.0)*hOffset,
									0.0,
									0.1 * zOffset, rw));
	    } //for lr
	  }
	}
	
	hOffset += disc->mStaveSpacing;
      }
    } //for inf

    // Place two central staves;
#if 1
    {
      unsigned nChips;

      for(nChips=0; ; nChips++) {
	double staveLength = GetExpectedStaveLength(nChips+1, mcell);
	
	double xx = staveWidth/2;
	double yy = disc->mMinRadius + staveLength;
	double rr = sqrt(xx*xx + yy*yy);
	
	if (rr > disc->mMaxRadius) break;
      } //for n
	  
      // Can hardly fail;
      assert(nChips);
	  
      // Add a stave with given number of chips at this location;
      {    
	//MapsStave *stave = ConstructStave(nChips, staveGlobalCounter++, mcell);
	
	//TGeoRotation *rw = new TGeoRotation();
	double zOffset = (mMountingRingBeamLineThickness + mcell->GetAssemblyHeight())/2;
	for(unsigned tb=0; tb<2; tb++) {
	  TGeoRotation *rw = 0;

	  if (tb) {
	    rw = new TGeoRotation();
	    rw->RotateZ(180);
	  } //if

	  chipGlobalCounter += nChips;
	  MapsStave *stave = ConstructStaveWithMapping(nChips, staveGlobalCounter++, mcell);
	  vdisc->AddNode(stave->GetVolume(), /*tb*/0, 
			 new TGeoCombiTrans(0.1 * (tb ? -1.0 : 1.0) * mcell->mChipDeadAreaWidth/2,
					    0.1 * (tb ? -1.0 : 1.0)*(disc->mMinRadius + stave->GetLength()/2),
					    0.1 * zOffset, rw));
	} //for tb
      }
    }
#endif

#if 1
    // Place 2x2 next-to-central staves;
    {
      unsigned nChips;
      double hOffset = disc->mStaveSpacing, dx = hOffset - staveWidth/2;
      // This trick does not work if I want to offset chip core by +/-1mm as well;
      //double y0 = sqrt(disc->mMinRadius*disc->mMinRadius - dx*dx);
      double y0 = disc->mMinRadius;

      for(nChips=0; ; nChips++) {
	double staveLength = GetExpectedStaveLength(nChips+1, mcell);
	
	double xx = staveWidth/2;
	double yy = y0 + staveLength;
	double rr = sqrt(xx*xx + yy*yy);
	
	if (rr > disc->mMaxRadius) break;
      } //for n
	  
      // Can hardly fail;
      assert(nChips);
	  
      // Add a stave with given number of chips at this location;
      {    
	//MapsStave *stave = ConstructStave(nChips, staveGlobalCounter++, mcell);
	
	double zOffset = -(mMountingRingBeamLineThickness + mcell->GetAssemblyHeight())/2;
	for(unsigned lr=0; lr<2; lr++)
	  for(unsigned tb=0; tb<2; tb++) {
	    TGeoRotation *rw = new TGeoRotation();

	    if (!tb)
	      rw->RotateY(180);
	    else
	      rw->RotateX(180);

	    chipGlobalCounter += nChips;
	    MapsStave *stave = ConstructStaveWithMapping(nChips, staveGlobalCounter++, mcell);
	    vdisc->AddNode(stave->GetVolume(), /*lr*2+tb*/0, 
			   new TGeoCombiTrans(0.1 * ((lr ? -1.0 : 1.0) * hOffset + 
						     (tb ?  1.0 :-1.0) * mcell->mChipDeadAreaWidth/2),
					      0.1 * (tb ? -1.0 : 1.0)*(y0 + stave->GetLength()/2),
					      0.1 * zOffset, rw));
	  } //for lr..tb
      }
    }
#endif
    
    // Place mounting ring;
    if (WithMountingRings())
    {
      TGeoTube *mring = new TGeoTube(mountingRingName,
				     0.1 * (disc->mMaxRadius - mMountingRingRadialThickness),
				     0.1 *  disc->mMaxRadius,
				     0.1 * mMountingRingBeamLineThickness/2);
      TGeoVolume *vmring = new TGeoVolume(mountingRingName, mring, GetMedium(mCarbonFiberMaterial));
      
      vdisc->AddNode(vmring, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));
    } //if

#if _LATER_
    // Set up a mapping table (more or less dummy for now);
    AddLogicalVolumeGroup(staveGlobalCounter, nChipsMax);

    // Yes, carelessly create one map per layer; same as for VST;
    EicGeoMap *fgmap = CreateNewMap();
    fgmap->AddGeantVolumeLevel(mMimosaCoreName,   0);
    fgmap->AddGeantVolumeLevel(mMimosaShellName,  0);
    fgmap->AddGeantVolumeLevel(mCellAssemblyName, blayer->mMimosaChipNum);
    fgmap->AddGeantVolumeLevel(stave->GetName(),  blayer->mStaveNum);

    fgmap->SetSingleSensorContainerVolume(mMimosaCoreName);
#endif
  } //for dc
	
#if 0
  // Place a fake beam pipe spot for eye guidance;
  {
    TGeoTube *bpipe = new TGeoTube("Pipe",
				   0.0,
				   0.1 * 18.0,
				   0.1 * 10.0);
    TGeoVolume *vbpipe = new TGeoVolume("Pipe", bpipe, GetMedium("MapsCarbonFiber"));
    
    GetTopVolume()->AddNode(vbpipe, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * 350., 0));
  }
#endif

  printf("%5d chip(s) and %5d stave(s) total\n", chipGlobalCounter, staveGlobalCounter);
  
  // And put this stuff as a whole into the top volume; 
  FinalizeOutput();
  
  return 0;
} // FstGeoParData::ConstructGeometry()

// ---------------------------------------------------------------------------------------

ClassImp(FstGeoParData)
ClassImp(FstDisc)
