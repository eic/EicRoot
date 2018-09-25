//
// AYK (ayk@bnl.gov), 2014/08/07
//
//  VST MAPS geometry description file; should have probably kept this stuff
// in geometry/MAPS/vst.C; but 1) I'm really tired of CINT bugs and restrictions, 
// 2) this way I perfectly decouple input data and construction algorithms;
//
//  FIXME: materials are hardcoded indeed; but I know, they are well defined ...;
//

#include <iostream>
using namespace std;

#include <TMath.h>
#include <TGeoTrd1.h>
#include <TGeoTube.h>

#include <VstGeoParData.h>

// ---------------------------------------------------------------------------------------

int VstGeoParData::ConstructGeometry()
{
  // FIXME: may want to put these into MapsGeoParData?!;
  unsigned staveGlobalCounter = 0, chipGlobalCounter = 0;

  for(unsigned bl=0; bl<GetNumberOfLayers(); bl++) {
    const VstBarrelLayer *blayer = GetBarrelLayer(bl);
    MapsMimosaAssembly *mcell = blayer->mChipAssembly;

    staveGlobalCounter += blayer->mStaveNum;
    chipGlobalCounter  += blayer->mStaveNum*blayer->mMimosaChipNum;

    AddLogicalVolumeGroup(blayer->mStaveNum, 0, blayer->mMimosaChipNum);

    char mountingRingName[128];

    snprintf(mountingRingName,     128-1, "VstMountingRing%02d",        bl);
 
    MapsStave *stave = ConstructStave(blayer->mMimosaChipNum, bl, mcell);

    // Yes, carelessly create one map per layer;
    EicGeoMap *fgmap = CreateNewMap();
    fgmap->AddGeantVolumeLevel(mMimosaCoreName,   0);
    fgmap->AddGeantVolumeLevel(mMimosaShellName,  0);
    fgmap->AddGeantVolumeLevel(mCellAssemblyName, blayer->mMimosaChipNum);
    fgmap->AddGeantVolumeLevel(stave->GetName(),  blayer->mStaveNum);

    fgmap->SetSingleSensorContainerVolume(mMimosaCoreName);

    // Construct mapping table; no tricks like linear cell numbering and circular 
    // quadrant arrangement in say FEMC calorimeter construction -> may just use a separate loop;
    for(unsigned st=0; st<blayer->mStaveNum; st++) 
      for(unsigned nn=0; nn<blayer->mMimosaChipNum; nn++) {
	UInt_t geant[4] = {0, 0, nn, st}, logical[3] = {st, 0, nn};

	if (SetMappingTableEntry(fgmap, geant, bl, logical)) {
	  cout << "Failed to set mapping table entry!" << endl;
	  exit(0);
	} //if
      } //for st..nn

    {
      double staveCenterRadius;

      // Place staves into master volume; FIXME: no extra hierarchy here?; well, otherwise 
      // would have to precisely calculate barrel TUBE volume inner/outer radius; 
      for(unsigned st=0; st<blayer->mStaveNum; st++) {
	TGeoRotation *rw = new TGeoRotation();
	  
	double degAngle = st*360.0/blayer->mStaveNum + blayer->mAsimuthalOffset;
	double radAngle = degAngle*TMath::Pi()/180.0;
	
	{
	  double fullAngle = degAngle + blayer->mStaveSlope;
	  rw->SetAngles(90.0, 0.0 - fullAngle, 180.0,  0.0, 90.0, 90.0 - fullAngle);
	}
	  
	// ALICE TDR gives silicon chip center intallation radius (p.8); want to reproduce the 
	// geometry (and slope in particular) in order to cross-check material budget;
	{
	  // Use sine theorem;
	  double alfa = blayer->mStaveSlope*TMath::Pi()/180.0;
	  double beta = asin(fabs(mMimosaOffset)*sin(alfa)/blayer->mRadius);
	  double gamma = TMath::Pi() - alfa - beta;
	  staveCenterRadius = blayer->mRadius*sin(gamma)/sin(alfa);
	  GetTopVolume()->AddNode(stave->GetVolume(), st, 
				  new TGeoCombiTrans(0.1 * staveCenterRadius*sin(radAngle), 
						     0.1 * staveCenterRadius*cos(radAngle), 0.0, rw));
	}
      } //for st
      
      // Construct a mounting ring; let it be there for all geometry types;
      if (WithMountingRings())
      {
	TGeoTube *mring = new TGeoTube(mountingRingName,
				       0.1 * (staveCenterRadius + mMountingRingRadialOffset - mMountingRingRadialThickness/2),
				       0.1 * (staveCenterRadius + mMountingRingRadialOffset + mMountingRingRadialThickness/2),
				       0.1 * mMountingRingBeamLineThickness/2);
	TGeoVolume *vmring = new TGeoVolume(mountingRingName, mring, GetMedium(mCarbonFiberMaterial));
	
	// Place two rings;
	for(unsigned fb=0; fb<2; fb++) {
	  double zOffset = (fb ? -1. : 1.)*(stave->GetLength()/2 + mMountingRingBeamLineThickness/2);
	  
	  GetTopVolume()->AddNode(vmring, fb, new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, 0));
	} //for fb
      } //if
    }
  } //for bl
   
  printf("%5d chip(s) and %5d stave(s) total\n", chipGlobalCounter, staveGlobalCounter);

  // Place this stuff as a whole into the top volume and write out;
  FinalizeOutput();

  return 0;
} // VstGeoParData::ConstructGeometry()

// ---------------------------------------------------------------------------------------

ClassImp(VstGeoParData)
ClassImp(VstBarrelLayer)
