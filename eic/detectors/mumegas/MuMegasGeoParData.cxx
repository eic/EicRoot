//
// AYK (ayk@bnl.gov), 2015/01/28
//
//  MuMegas geometry description file;
//

#include <iostream>
using namespace std;

#include <TGeoTube.h>
#include <TGeoVolume.h>

#include <MuMegasGeoParData.h>

// ---------------------------------------------------------------------------------------

int MuMegasGeoParData::ConstructGeometry(bool root, bool gdml, bool check)
{
  const char *detName = mDetName->Name().Data();

  // Loop through all wheels (or perhaps single modules) independently;
  for(unsigned bl=0; bl<mBarrels.size(); bl++) {
    MuMegasBarrel *barrel   = mBarrels[bl];
    MuMegasLayer *layer = barrel->mLayer;

    // Figure out thickness of the sector gas container volume;
    double gasSectorThickness = layer->mReadoutPcbThickness + layer->mCopperStripThickness + 
      layer->mAmplificationRegionLength + layer->mSteelMeshThickness + layer->mConversionRegionLength + 
      layer->mExitWindowThickness;

    // Figure out thickness of the overall air container volume;
    double airContainerThickness = gasSectorThickness < layer->mInnerFrameThickness ? 
      layer->mInnerFrameThickness : gasSectorThickness;
    if (airContainerThickness < layer->mOuterFrameThickness) 
      airContainerThickness = layer->mOuterFrameThickness;

    // Define air container volume and place it into the top volume;
    char barrelContainerVolumeName[128];
    snprintf(barrelContainerVolumeName, 128-1, "%sBarrelContainerVolume%02d", detName, bl);
      
    TGeoTube *bcontainer = new TGeoTube(barrelContainerVolumeName,
					0.1 *  barrel->mRadius,
					0.1 * (barrel->mRadius + airContainerThickness),
					0.1 *  barrel->mLength/2);
    TGeoVolume *vbcontainer = new TGeoVolume(barrelContainerVolumeName, bcontainer, GetMedium(_AIR_));
		  
    GetTopVolume()->AddNode(vbcontainer, 0, barrel->mTransformation);
    
    // Define and place a pair of outer frame rings;
    char outerFrameVolumeName[128];
    snprintf(outerFrameVolumeName, 128-1, "%sOuterFrameVolume%02d", detName, bl);
      
    TGeoTube *oframe = new TGeoTube(outerFrameVolumeName,
				    0.1 * barrel->mRadius,
				    0.1 * (barrel->mRadius + layer->mOuterFrameThickness),
				    0.1 * layer->mOuterFrameWidth/2);
    TGeoVolume *voframe = new TGeoVolume(outerFrameVolumeName, oframe, GetMedium("MuMegasCarbonFiber"));
    double zOffset = (barrel->mLength - layer->mOuterFrameWidth)/2;
    for(unsigned fb=0; fb<2; fb++)
      vbcontainer->AddNode(voframe, fb, new TGeoCombiTrans(0.0, 0.0, 0.1 * (fb ? -1.0 : 1.0)*zOffset, 0));

    // Figure out sector length;
    double singleSectorLength = 
      (barrel->mLength - 2.0 * layer->mOuterFrameWidth - 
       (barrel->mBeamLineSectionNum-1)*layer->mInnerFrameWidth)/barrel->mBeamLineSectionNum;
    double singleSectorZspacing = singleSectorLength + layer->mInnerFrameWidth;

    // Then inner frame rings (in case more than 1 section); 
    {
      char innerFrameVolumeName[128];
      snprintf(innerFrameVolumeName, 128-1, "%sInnerFrameVolume%02d", detName, bl);
      
      TGeoTube *iframe = new TGeoTube(innerFrameVolumeName,
				      0.1 * barrel->mRadius,
				      0.1 * (barrel->mRadius + layer->mInnerFrameThickness),
				      0.1 * layer->mInnerFrameWidth/2);
      TGeoVolume *viframe = new TGeoVolume(innerFrameVolumeName, iframe, GetMedium("MuMegasCarbonFiber"));

      for(unsigned iz=0; iz<barrel->mBeamLineSectionNum-1; iz++) {
	double zOffset = singleSectorZspacing*(iz - (barrel->mBeamLineSectionNum-2)/2.);

	vbcontainer->AddNode(viframe, iz, new TGeoCombiTrans(0.0, 0.0, 0.1*zOffset, 0));
      } //for iz
    } 

    // Figure out asimuthal "clear acceptance" angular range occupied by each sector; 
    double circumference = 2.0 * TMath::Pi() * barrel->mRadius; 
    double clearAcceptanceFraction = 
      (circumference - barrel->mAsimuthalSectorNum*layer->mInnerFrameWidth)/circumference;
    double singleSectorAngle = 360.0 * clearAcceptanceFraction / barrel->mAsimuthalSectorNum;
    
    // Define single sector gas container volume;
    char sectorContainerVolumeName[128];
    snprintf(sectorContainerVolumeName, 128-1, "%sSectorContainerVolume%02d", detName, bl);
    {
      // Funny enough, it looks like TGeoTubs has a bug in the navigation methods; but 
      // TGeoCtub, which is more generic, works; see also MuMegasGeoParData::PlaceMaterialLayer();
      TGeoCtub *sector = new TGeoCtub(sectorContainerVolumeName,
				      0.1 * barrel->mRadius,
				      0.1 * (barrel->mRadius + gasSectorThickness),
				      0.1 * singleSectorLength/2, 
				      0.0, singleSectorAngle,
				      0, 0, -1, 0, 0, 1);
      TGeoVolume *vsector = new TGeoVolume(sectorContainerVolumeName, sector, GetMedium(layer->mGasMixture));
		  
      // Place them all into the air container volume;
      for(unsigned ir=0; ir<barrel->mAsimuthalSectorNum; ir++) {
	TGeoRotation *rw = 0;
	if (ir) {
	  rw = new TGeoRotation();
	  rw->RotateZ(ir*360./barrel->mAsimuthalSectorNum);
	} //if
	
	for(unsigned iz=0; iz<barrel->mBeamLineSectionNum; iz++) {
	  double zOffset = singleSectorZspacing*(iz - (barrel->mBeamLineSectionNum-1)/2.);
	  
	  vbcontainer->AddNode(vsector, ir*barrel->mBeamLineSectionNum+iz, 
			       new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, rw));
	} //for iz
      } //for ir

      // Populate gas sector with essential material layers;
      {
	double rOffset = barrel->mRadius;

	PlaceMaterialLayer(detName, "ReadoutPcb", bl, vsector, 
			   layer->mReadoutPcbMaterial.Data(), 
			   singleSectorLength, singleSectorAngle,
			   layer->mReadoutPcbThickness,
			   &rOffset);

	PlaceMaterialLayer(detName, "ReadoutStrips", bl, vsector, 
			   "copper",
			   singleSectorLength, singleSectorAngle,
			   layer->mCopperStripThickness,
			   &rOffset);

	PlaceMaterialLayer(detName, "AmplificationRegion", bl, vsector, 
			   layer->mGasMixture.Data(),
			   singleSectorLength, singleSectorAngle,
			   layer->mAmplificationRegionLength,
			   &rOffset);

	PlaceMaterialLayer(detName, "SteelMesh", bl, vsector, 
			   "iron",
			   singleSectorLength, singleSectorAngle,
			   layer->mSteelMeshThickness,
			   &rOffset);

	PlaceMaterialLayer(detName, "ConversionRegion", bl, vsector, 
			   layer->mGasMixture.Data(),
			   singleSectorLength, singleSectorAngle - 1.0,
			   layer->mConversionRegionLength,
			   &rOffset);

	PlaceMaterialLayer(detName, "ExitWindow", bl, vsector, 
			   layer->mExitWindowMaterial.Data(),
			   singleSectorLength, singleSectorAngle,
			   layer->mExitWindowThickness,
			   &rOffset);
      }
    }

    // Define and place beam-aligned pieces of support frames;
    {
      char sectorFrameVolumeName[128];
      snprintf(sectorFrameVolumeName, 128-1, "%sSectorFrameVolume%02d", detName, bl);
      double singleSectorFrameAngle = 360./barrel->mAsimuthalSectorNum - singleSectorAngle;
      TGeoCtub *sframe = new TGeoCtub(sectorFrameVolumeName,
				      0.1 * barrel->mRadius,
				      0.1 * (barrel->mRadius + layer->mInnerFrameThickness),
				      0.1 * singleSectorLength/2, 
				      0.0, 
				      singleSectorFrameAngle,
				      0, 0, -1, 0, 0, 1);
      TGeoVolume *vsframe = new TGeoVolume(sectorFrameVolumeName, sframe, GetMedium("MuMegasCarbonFiber"));
      
      for(unsigned ir=0; ir<barrel->mAsimuthalSectorNum; ir++) {
	TGeoRotation *rw = new TGeoRotation();
	rw->RotateZ(ir*360./barrel->mAsimuthalSectorNum + singleSectorAngle);
	
	for(unsigned iz=0; iz<barrel->mBeamLineSectionNum; iz++) {
	  double zOffset = singleSectorZspacing*(iz - (barrel->mBeamLineSectionNum-1)/2.);
	  
	  vbcontainer->AddNode(vsframe, ir*barrel->mBeamLineSectionNum+iz, 
			       new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, rw));
	} //for iz
      } //for ir
    }

    // Yes, one map per layer; FIXME: conversion volume name should be calculated once;
    // also may want to unify with the other [ir..iz] loop;
    {
      AddLogicalVolumeGroup(barrel->mAsimuthalSectorNum, 0, barrel->mBeamLineSectionNum);

      EicGeoMap *fgmap = CreateNewMap();

      char conversionVolumeName[128];
      snprintf(conversionVolumeName, 128-1, "%s%s%02d", detName, "ConversionRegion", bl);
      
      fgmap->AddGeantVolumeLevel(conversionVolumeName,                                                               0);
      fgmap->AddGeantVolumeLevel(sectorContainerVolumeName,  barrel->mAsimuthalSectorNum * barrel->mBeamLineSectionNum);

      fgmap->SetSingleSensorContainerVolume(conversionVolumeName);

      for(unsigned ir=0; ir<barrel->mAsimuthalSectorNum; ir++) 
	for(unsigned iz=0; iz<barrel->mBeamLineSectionNum; iz++) {
	  UInt_t geant[2] = {0, ir*barrel->mBeamLineSectionNum+iz}, logical[3] = {ir, 0, iz};

	  if (SetMappingTableEntry(fgmap, geant, bl, logical)) {
	    cout << "Failed to set mapping table entry!" << endl;
	    exit(0);
	  } //if
	} //for ir..iz
    }
  } //for bl

  // Place this stuff as a whole into the top volume and write out;
  FinalizeOutput(root, gdml, check);

  return 0;
} // MuMegasGeoParData::ConstructGeometry()

// ---------------------------------------------------------------------------------------

void MuMegasGeoParData::PlaceMaterialLayer(const char *detName, const char *volumeNamePrefix, 
					   unsigned barrelID, 
					   TGeoVolume *sectorContainer, const char *material, 
					   double length, double angle,
					   double thickness, double *rOffset)
{
  char volumeName[128];

  snprintf(volumeName, 128-1, "%s%s%02d", detName, volumeNamePrefix, barrelID);
  TGeoCtub *shape = new TGeoCtub(volumeName,
				 0.1 * (*rOffset),
				 0.1 * (*rOffset + thickness),
				 0.1 * length/2, 
				 0.0, 
				 angle, 
				 0, 0, -1, 0, 0, 1);
  TGeoVolume *vshape = new TGeoVolume(volumeName, shape, GetMedium(material));

  sectorContainer->AddNode(vshape, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

  *rOffset += thickness;
} // MuMegasGeoParData::PlaceMaterialLayer()

// ---------------------------------------------------------------------------------------

ClassImp(MuMegasLayer)
ClassImp(MuMegasBarrel)
ClassImp(MuMegasGeoParData)
