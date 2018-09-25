
// Meaningless number for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

// Just rectangular PbSciMix volumes with no structure; useful for drawing purposes
// as well as for fast simulation (FIXME: should be adapted); NB: this definition
// should be *before* hcal-lib.C include below;
#define _NO_STRUCTURE_GEOMETRY_

// Yes, keep it simple; assume this library file is in the same directory;
#include <./hcal-lib.C>

#if 0
struct {
  char *name;
  double offset;
} dets[] = {
  {"fhac",  3400.},
  {"bhac", -3400.+1130.}
};
#endif
static double offsets[2] = {-3400.+1130., 3400.};

hcal()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Loop over forward/backward parts;
  for(unsigned fb=0; fb<2; fb++) {
    HcalGeoParData *hcal = 
      //(HcalGeoParData *)define_basic_parameters(fb ? "BHAC" : "FHAC", _VERSION_, _SUBVERSION_);
      (HcalGeoParData *)define_basic_parameters(fb ? "FHAC" : "BHAC", _VERSION_, _SUBVERSION_);
    const TString& cname = hcal->GetDetName()->Name();

  // Forward calorimeter only, for now;
#ifdef _NO_STRUCTURE_GEOMETRY_
    hcal->SetGeometryType(EicGeoParData::NoStructure);
    //TString filename = lname + "-ns.root";
#else
    //TString filename = lname + "-fs.root";
    hcal->SetGeometryType(EicGeoParData::FullStructure);
#endif

    //
    // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
    // definition routines only;
    //

    // Endcap is shifted along H+ direction as a whole;
    double beamLineOffset           = offsets[fb];//dets[fb].offset;
    
    // Pick up whatever parameters for now;
    hcal->mEndcapMinR               =   20.0;
    hcal->mEndcapMaxTheta           =   40.;
    // Do not allow subunits to protrude towards too small and too large radii;
    hcal->mSafetyVolume             =   10.0;
    
    // Well, assume no-gap packing?;
    hcal->mInterQuadrantGap         =    0.0;

    // This part of the script can be shared between various calorimeter types;
    const Double_t endcapMaxR   = 
      fabs(beamLineOffset) * tan(hcal->mEndcapMaxTheta * TMath::Pi() / 180.);
    
    const Double_t envelopeWidth = hcal->mCellFaceSizeX + hcal->mInterCellGap;
    
    // Estimate an absolutely max number of rows;
    const Int_t rowsNumMax = (int)floor(endcapMaxR / envelopeWidth);
    //cout << rowsNumMax << endl;
    // Do not mind to use just this number in order to calculate XY-dimensions;
    // if the actual matrix will be a bit smaller, this does not really matter;
    //hcal->SetDimX(2*rowsNumMax); hcal->SetDimY(2*rowsNumMax);
    hcal->AddLogicalVolumeGroup(2*rowsNumMax, 2*rowsNumMax);
    struct FemcRow {
      Double_t yc;
    } *rows = calloc(rowsNumMax, sizeof(FemcRow));
    
    for(int iq=0; iq<rowsNumMax; iq++) {
      FemcRow *row = rows + iq;
	
      row->yc = envelopeWidth * (iq + 0.5);
    } //for iq
    
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
					      0.1 * hcal->mEndcapMinR,
					      0.1 * endcapMaxR,
					      0.1 * hcal->mCellLength/2.,
					      0.0, 90.0);
      TGeoVolume *vquadrant = new TGeoVolume(cname + "Quadrant", quadrant, hcal->GetMedium("air"));

      // _DUMMY_ definition can be interpreted correctly there;
      TGeoVolume *vtower = make_single_tower(hcal);
      
      // Configure tower map;
      EicGeoMap *gmap = hcal->CreateNewMap();
#ifndef _NO_STRUCTURE_GEOMETRY_
      gmap->AddGeantVolumeLevel(cname + "ScintillatorPlate", 0);
#endif
      gmap->AddGeantVolumeLevel(cname + "Tower",             rowsNumMax*rowsNumMax);
      gmap->AddGeantVolumeLevel(cname + "Quadrant",          4);

      gmap->SetSingleSensorContainerVolume(cname + "Tower");

      hcal->AddBlackHoleVolume(cname + "Quadrant");
      hcal->AddBlackHoleVolume(cname + "FrontPlate");

      // Populate quadrant volume with towers; FIXME: later there should be some construction 
      // material as well; for now do NOT put something like front steel plate into 
      // either quadrant separately or a single tower;
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
	  if (rr - hcal->mCellFaceSizeX/sqrt(2.) - hcal->mSafetyVolume < hcal->mEndcapMinR) continue;
	  if (rr + hcal->mCellFaceSizeX/sqrt(2.) + hcal->mSafetyVolume >        endcapMaxR) continue;
	  
	  // Loop through 2x2 quadrants and add the 4 tower entries into the mapping table;
	  for(int iqx=0; iqx<2; iqx++)
	    for(int iqy=0; iqy<2; iqy++) {
	      int XX = iqx ? rowsNumMax + ix : rowsNumMax - ix - 1;
	      int YY = iqy ? rowsNumMax + iy : rowsNumMax - iy - 1;
	      
	      // Determine 1D index in the mapping table; 
#ifdef _NO_STRUCTURE_GEOMETRY_
	      UInt_t geant[2] = {   tcounter, 2*iqx + iqy};
#else
	      UInt_t geant[3] = {0, tcounter, 2*iqx + iqy};
#endif
	      unsigned group = 0, logical[2] = {XX, YY};
	      //if (gmap->SetMappingTableEntry(id, UGeo_t((XX << 16) | YY))) {
	      if (hcal->SetMappingTableEntry(gmap, geant, group, logical)) {
		cout << "Failed to set mapping table entry!" << endl;
		exit(0);
	      } //if
	    } //for iqx..iqy 
	  
	  {
	    TGeoRotation *rt = new TGeoRotation();

	    vquadrant->AddNode(vtower, tcounter++, 
			       new TGeoCombiTrans(0.1 * xx, 0.1 * yy, 0.0, rt));
	  }
	} //for iy
      } //for ix
      
      printf("%5d towers per quadrant\n", tcounter);

      // Locate 4 quadrant copies in the "main" volume;
      for(int ixy=0; ixy<4; ixy++) {
	Double_t local[4] = {hcal->mInterQuadrantGap/2, hcal->mInterQuadrantGap/2, 0, 0}, master[4];

	_qquad[ixy]->LocalToMaster(local, master);
	
	hcal->GetTopVolume()->AddNode(vquadrant, ixy, 
				      new TGeoCombiTrans(0.1 * master[0], 0.1 * master[1], 0.0, _qquad[ixy]));
      }
    } 
    
    // And put this stuff as a whole into the top volume; account Z-shift;
    {
      TGeoRotation *rw = 0;

      if (beamLineOffset < 0.0) {
	rw = new TGeoRotation();

	rw->RotateY(180); 
      } //if

      hcal->SetTopVolumeTransformation(new TGeoCombiTrans(0.0, 0.0, 0.1 * beamLineOffset, rw));
    }

    // Let them be green;
    hcal->GetColorTable()->AddPatternMatch(cname + "Tower", kGreen+1);
    //hcal->GetColorTable()->AddPatternMatch(cname + "Quadrant", kGreen+1);

    hcal->FinalizeOutput();
  } //for fb

  // Yes, always exit;
  exit(0);
}

