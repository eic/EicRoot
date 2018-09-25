//
// AYK (ayk@bnl.gov), 2014/08/07
//
//  MAPS geometry description file;
//

#include <iostream>
using namespace std;

#include <assert.h>

#include <MapsGeoParData.h>

#include <TMath.h>
#include <TGeoTrd1.h>
#include <TGeoArb8.h>
#include <TGeoTube.h>
#include <TGeoTorus.h>
#include <TGeoCompositeShape.h>

#include <MapsGeoParData.h>
#include <MapsMimosaAssembly.h>

// ---------------------------------------------------------------------------------------

TGeoVolume *MapsGeoParData::ConstructMimosaCell(MapsMimosaAssembly *mcell, unsigned id)
{
  const char *detName = mDetName->Name().Data();

  // Do not mind to create different names for each layer, even if some layers 
  // have the same building blocks;
  char flexName[128], aluStripName[128], flatRoofName[128], roofSideBeamName[128];
  char waterPipeName[128], waterName[128], apexEnforcementBeamName[128];
  char coldPlateName[128], sideWallName[128], baseEnforcementBeamName[128];
      
  snprintf(flexName,                128-1, "%sCellFlexLayer%02d",       detName, id);
  snprintf(aluStripName,            128-1, "%sAluStrips%02d",           detName, id);
  snprintf(coldPlateName,           128-1, "%sColdPlate%02d",           detName, id);
  snprintf(mMimosaShellName,        128-1, "%sMimosaShell%02d",         detName, id);
  snprintf(mMimosaCoreName,         128-1, "%sMimosaCore%02d",          detName, id);
  snprintf(waterPipeName,           128-1, "%sWaterPipe%02d",           detName, id);
  snprintf(waterName,               128-1, "%sWater%02d",               detName, id);
  snprintf(apexEnforcementBeamName, 128-1, "%sEnforcementBeamApex%02d", detName, id);
  snprintf(baseEnforcementBeamName, 128-1, "%sEnforcementBeamBase%02d", detName, id);
  snprintf(sideWallName,            128-1, "%sSideWall%02d",            detName, id);
  snprintf(flatRoofName,            128-1, "%sFlatRoof%02d",            detName, id);
  snprintf(roofSideBeamName,        128-1, "%sRoofSideBeam%02d",        detName, id);
  snprintf(mCellAssemblyName,       128-1, "%sChipAssembly%02d",        detName, id);
  
  // Used throughout the code;
  //mAssemblyHeight = (mcell->mAssemblyBaseWidth/2)*
  //tan(mcell->mAssemblySideSlope*TMath::Pi()/180.);
  mAssemblyHeight = mcell->GetAssemblyHeight();
  mAssemblyLength = mcell->GetAssemblyLength();

  // Compose elementary cell volume with the Mimosa 34 chip inside;
  TGeoVolume *vcell;
  if (UseTriangularAssemblies()) {
    // This is fine for the VST;
    TGeoTrd1 *trd1 = new TGeoTrd1(mCellAssemblyName,
				  0.1 * mcell->mAssemblyBaseWidth/2,
				  0.0,
				  0.1 * mAssemblyLength/2,
				  0.1 * mAssemblyHeight/2);

    vcell = new TGeoVolume(mCellAssemblyName, trd1, GetMedium(_AIR_));
  }
  else {
    char tmp1Name[128], tmp2Name[128];

    snprintf(tmp1Name,              128-1, "%sTmp1Volume%02d",          detName, id);
    snprintf(tmp2Name,              128-1, "%sTmp2Volume%02d",          detName, id);

    assert(0);
    // For FST/BST have to cut triangular cell edges, otherwise can not come close enough
    // to the beam pipe; TGeoCompositeShape has problems with drawing of course, but 
    // I set it invisible in eventDisplay.C anyway; NB: one could arrange Arb8 shape with 
    // only one edge cut, but then I'd have to re-orient all the subvolumes because Z axis
    // would be oriented differently -> forget it;
    TGeoTrd1 *tmp1 = new TGeoTrd1(tmp1Name,
				  0.1 * mcell->mAssemblyBaseWidth/2,
				  0.0,
				  0.1 * mAssemblyLength/2,
				  0.1 * mAssemblyHeight/2);
    TGeoBBox *tmp2 = new TGeoBBox(tmp2Name,
				  0.1 * mcell->mAssemblyBaseWidth/2,
				  //0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				  0.1 * mAssemblyLength/2,
				  0.1 * mAssemblyHeight/2);
    TGeoCompositeShape *cs = new TGeoCompositeShape(mCellAssemblyName, TString(tmp1Name) + "*" + TString(tmp2Name));

    vcell = new TGeoVolume(mCellAssemblyName, cs, GetMedium(_AIR_));
  } //if

  double aluThickness = 0.0, kaptonThickness = 0.0, carbonThickness = 0.0, waterThickness = 0.0;

  // A running variable (accumulated offset);
  double zOffset = -mAssemblyHeight/2;

  // Create internal chip assembly cell structure;
  {
    //
    // FIXME: make a routine to add a single Z-layer;
    //

    // Flex Printed Circuit;
    {

      if (GetGeometryType() == MapsGeoParData::NoStructure)
	kaptonThickness += mcell->mFlexCableKaptonThickness;
      else {
	TGeoBBox *flex = new TGeoBBox(flexName,
				      0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				      0.1 * mAssemblyLength/2,
				      0.1 * mcell->mFlexCableKaptonThickness/2);
	TGeoVolume *vflex = new TGeoVolume(flexName, flex, GetMedium(mKaptonMaterial));
	
	vcell->AddNode(vflex, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * (zOffset + mcell->mFlexCableKaptonThickness/2), 0));
      } //if    
 
      // Yes, want to keep precise track on the silicon chip final offset, so it is the same in 
      // all three "fs", "ss", "ns" geometries;
      zOffset += mcell->mFlexCableKaptonThickness;
    }
    
    // Alu strips;
    {
      if (GetGeometryType() == MapsGeoParData::NoStructure)
	aluThickness += mcell->mFlexCableAluThickness;
      else {
	TGeoBBox *alu = new TGeoBBox(aluStripName,
				     0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				     0.1 * mAssemblyLength/2,
				     0.1 * mcell->mFlexCableAluThickness/2);
	TGeoVolume *valu = new TGeoVolume(aluStripName, alu, GetMedium(_ALUMINUM_));
	
	vcell->AddNode(valu, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * (zOffset + mcell->mFlexCableAluThickness/2), 0));
      } //if    
 
      zOffset += mcell->mFlexCableAluThickness;
    }
    
    // Mimosa chips; keep this structure in all theree geometry versions;
    {
      TGeoBBox *mimosa = new TGeoBBox(mMimosaShellName,
				      0.1 * mcell->mChipWidth/2,
				      0.1 * mcell->mChipLength/2,
				      0.1 * mcell->mChipThickness/2);
      TGeoVolume *vmimosa = new TGeoVolume(mMimosaShellName, mimosa, GetMedium(_SILICON_));

      mMimosaOffset = zOffset;
      vcell->AddNode(vmimosa, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * (zOffset + mcell->mChipThickness/2), 0));
      
      zOffset += mcell->mChipThickness;
      
      // Mimosa chips sensitive layer;
      {
	TGeoBBox *micore = new TGeoBBox(mMimosaCoreName,
					0.1 * (mcell->mChipWidth - mcell->mChipDeadAreaWidth)/2,
					0.1 * mcell->mChipLength/2,
					0.1 * mcell->mChipActiveZoneThickness/2);
	TGeoVolume *vmicore = new TGeoVolume(mMimosaCoreName, micore, GetMedium(_SILICON_));
	
	// NB: shift active area to the side in local X direction; NB: don't change '-' here, 
	// otherwise will need to modify rotations in FST/BST in order to bring chips more close 
	// to the beam line;
	vmimosa->AddNode(vmicore, 0, new TGeoCombiTrans(0.1 * (-mcell->mChipDeadAreaWidth/2), 0.0, 0.0, 0));
      }
    }
    
    // Cold plate;
    {
      if (GetGeometryType() == MapsGeoParData::NoStructure)
	carbonThickness += mcell->mColdPlateThickness;
      else {
	TGeoBBox *cold = new TGeoBBox(coldPlateName,
				      0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				      0.1 * mAssemblyLength/2,
				      0.1 * mcell->mColdPlateThickness/2);
	TGeoVolume *vcold = new TGeoVolume(coldPlateName, cold, GetMedium(mCarbonFiberMaterial));
	
	vcell->AddNode(vcold, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * (zOffset + mcell->mColdPlateThickness/2), 0));
      } //if     
 
      zOffset += mcell->mColdPlateThickness;
    }
    
    // Water pipes;
    {
      double waterPipeOuterDiameter = mcell->mWaterPipeInnerDiameter + 2*mcell->mWaterPipeWallThickness;
      
      if (GetGeometryType() == MapsGeoParData::NoStructure)
      {
	// FIXME: make a function call;
	double effectiveWaterThickness = 
	  (TMath::Pi()*(mcell->mWaterPipeInnerDiameter * mcell->mWaterPipeInnerDiameter)/4.)/
	  mcell->mAssemblyDeadMaterialWidth;

	// NB: 2 pipes ...;
	waterThickness += 2*effectiveWaterThickness;

	double effectiveKaptonThickness = 
	  (TMath::Pi()*(waterPipeOuterDiameter * waterPipeOuterDiameter)/4.)/
	  mcell->mAssemblyDeadMaterialWidth;
	// NB: 2 pipes ...;
	kaptonThickness += 2*(effectiveKaptonThickness - effectiveWaterThickness);
      }
      else {
	TGeoTube *wpipe = new TGeoTube(waterPipeName,
				       0.1 * mcell->mWaterPipeInnerDiameter/2,
				       0.1 * waterPipeOuterDiameter/2,
				       0.1 * mAssemblyLength/2);
	TGeoVolume *vwpipe = new TGeoVolume(waterPipeName, wpipe, GetMedium(mKaptonMaterial));
	
	// Water itself;
	TGeoTube *water = new TGeoTube(waterName,
				       0.0,
				       0.1 * mcell->mWaterPipeInnerDiameter/2,
				       0.1 * mAssemblyLength/2);
	TGeoVolume *vwater = new TGeoVolume(waterName, water, GetMedium(_WATER_));
	
	TGeoRotation *rw = new TGeoRotation();
	rw->RotateX(90);
	
	// Yes, 15mm split in 5+5+5mm distances according to ALICE TRD;
	mWaterPipeXoffset = mcell->mChipWidth/(2*3);
	mWaterPipeZoffset = zOffset + waterPipeOuterDiameter/2;
	
	for(unsigned lr=0; lr<2; lr++) {
	  double xOffset = (lr ? -1.0 : 1.0)*mWaterPipeXoffset;
	  
	  vcell->AddNode(vwpipe, lr, new TGeoCombiTrans(0.1 * xOffset, 0.0, 0.1 * mWaterPipeZoffset, rw));
	  vcell->AddNode(vwater, lr, new TGeoCombiTrans(0.1 * xOffset, 0.0, 0.1 * mWaterPipeZoffset, rw));
	} //for lr
      } //if
    }
    
    // Base and apex enforcement beams;
    if (GetGeometryType() == MapsGeoParData::NoStructure)
    {
      double slope = mcell->mAssemblySideSlope*TMath::Pi()/180.;

      carbonThickness += 
	// Apex beam (as two halves);
	(TMath::Pi()*(mcell->mApexEnforcementBeamDiameter * mcell->mApexEnforcementBeamDiameter)/4. +
	 // 2x base triangular beams;
	 2*(mcell->mBaseEnforcementBeamWidth*mcell->mBaseEnforcementBeamWidth*tan(slope)/2))/
	mcell->mAssemblyDeadMaterialWidth;
    }
    else
    {
      double slope = mcell->mAssemblySideSlope*TMath::Pi()/180.;

      // Apex beam;
      TGeoTube *abeam = new TGeoTube(apexEnforcementBeamName,
				     0.0,
				     0.1 * mcell->mApexEnforcementBeamDiameter/2,
				     0.1 * mAssemblyLength/2);
      TGeoVolume *vabeam = new TGeoVolume(apexEnforcementBeamName, abeam, GetMedium(mCarbonFiberMaterial));
      
      TGeoRotation *rw = new TGeoRotation();
      rw->RotateX(90);

      {
	double hOffset = zOffset + 
	  (mcell->mAssemblyDeadMaterialWidth/2)*tan(slope) - (mcell->mApexEnforcementBeamDiameter/2)/cos(slope);
	vcell->AddNode(vabeam, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * hOffset, rw));
      }

      // Base beams; have to resort to Arb8 shape, sorry;
      double width  = 0.1 * mcell->mBaseEnforcementBeamWidth;
      double length = 0.1 * mAssemblyLength;

      double vert[8][2] = {
	{-width,   -length/2},
	{-width,    length/2},
	{     0,    length/2},
	{     0,   -length/2},
	{     0,   -length/2},
	{     0,    length/2},
	{     0,    length/2},
	{     0,   -length/2}
      };

      double baseBeamHeight = mcell->mBaseEnforcementBeamWidth*tan(slope);
      TGeoArb8 *bbeam = new TGeoArb8(baseEnforcementBeamName, 0.1 * baseBeamHeight/2, (double*)vert); 
      TGeoVolume *vbbeam = new TGeoVolume(baseEnforcementBeamName, bbeam, GetMedium(mCarbonFiberMaterial));
      {
	double hOffset = zOffset + baseBeamHeight/2;

	for(unsigned lr=0; lr<2; lr++) {
	  double xOffset = (lr ? 1.0 : -1.0)*(mcell->mAssemblyDeadMaterialWidth/2 - mcell->mBaseEnforcementBeamWidth);

	  TGeoRotation *rw = new TGeoRotation();
	  if (lr) rw->RotateZ(180);

	  vcell->AddNode(vbbeam, lr, new TGeoCombiTrans(0.1 * xOffset, 0.0, 0.1 * hOffset, rw));
	} //for lr
      }
    } //if

    // Side walls; it looks like those are present in ALICE design;
    {
      double radangle = mcell->mAssemblySideSlope*TMath::Pi()/180.;
      
      double sideWallWidth = (mcell->mAssemblyDeadMaterialWidth/2)/cos(mcell->mAssemblySideSlope*TMath::Pi()/180.);

      // Do not be pedantic, finally; place rectangular beams; assume, that 1) available width 
      // in any of those 4 slots is "1/4 of the total cell length - beam width" and beam is 
      // aligned along the diagonal of this space;
      double slotWidth  = mAssemblyLength/4 - mcell->mEnforcementStripWidth;
      double slotHeight = sideWallWidth;
      
      double radSlope = atan(slotWidth/slotHeight);
      double degSlope = radSlope*180./TMath::Pi();
      
      double beamLength = sqrt(slotWidth*slotWidth + slotHeight*slotHeight) - 
	2*(mcell->mEnforcementStripWidth/2)/tan(radSlope);

      double fullThickness;
      if (GetGeometryType() == MapsGeoParData::FullStructure)
	fullThickness = mcell->mSideWallThickness + mcell->mEnforcementStripThickness;
      else
	fullThickness = mcell->mSideWallThickness + 
	  mcell->mEnforcementStripThickness*(4*beamLength*mcell->mEnforcementStripWidth)/(mAssemblyLength*sideWallWidth);
	
      if (GetGeometryType() == MapsGeoParData::NoStructure)
	// NB: 2 sides ...;
	carbonThickness += fullThickness*(2*sideWallWidth/mcell->mAssemblyDeadMaterialWidth);
      else {
	// Side wall volume; air container in case of full geometry; a single thin carbon layer
	// in case of beam-line-uniform geometry;
	TGeoBBox *side = new TGeoBBox(sideWallName,
				      0.1 * sideWallWidth/2,
				      0.1 * mAssemblyLength/2,
				      0.1 * fullThickness/2);
	TGeoVolume *vside;
	if (GetGeometryType() == MapsGeoParData::SimpleStructure)
	  vside = new TGeoVolume(sideWallName, side, GetMedium(mCarbonFiberMaterial));
	else {
	  vside = new TGeoVolume(sideWallName, side, GetMedium(_AIR_));
	
	  // Side wall solid thin layer (roof); 
	  {
	    TGeoBBox *flat = new TGeoBBox(flatRoofName,
					  0.1 * sideWallWidth/2,
					  0.1 * mAssemblyLength/2,
					  0.1 * mcell->mSideWallThickness/2);
	    TGeoVolume *vflat = new TGeoVolume(flatRoofName, flat, GetMedium(mCarbonFiberMaterial));
	    
	    vside->AddNode(vflat, 2, new TGeoCombiTrans(0.0, 0.0,  0.1 * mcell->mEnforcementStripThickness/2, 0));
	  }
	  
	  // 4x roof enforcement beams;
	  {
	    
	    TGeoBBox *beam = new TGeoBBox(roofSideBeamName,
					  0.1 * beamLength/2,
					  0.1 * mcell->mEnforcementStripWidth/2,
					  0.1 * mcell->mEnforcementStripThickness/2);
	    TGeoVolume *vbeam = new TGeoVolume(roofSideBeamName, beam, GetMedium(mCarbonFiberMaterial));
	    
	    for(unsigned iq=0; iq<4; iq++) {
	      double qOffset = (mAssemblyLength/4)*(iq - 1.5);
	      
	      TGeoRotation *rw = new TGeoRotation();
	      rw->RotateZ((iq%2 ? -1. : 1.)*degSlope);
	      
	      vside->AddNode(vbeam, iq, new TGeoCombiTrans(0.0, 0.1 * qOffset, -0.1 * mcell->mSideWallThickness/2, rw));
	    } //for iq
	  }
	} //if   
	
	// Place both side walls into the assembly;
	{
	  double hOffset = zOffset + (fullThickness/2)*cos(radangle) + (sideWallWidth/2)*sin(radangle);
	  
	  for(unsigned lr=0; lr<2; lr++) {
	    double xOffset = (lr ? -1.0 : 1.0)*(mcell->mAssemblyDeadMaterialWidth/2 + 
						(fullThickness/2)*sin(radangle) - 
						(sideWallWidth/2)*cos(radangle));
	    
	    TGeoRotation *rw = new TGeoRotation();
	    if (lr)
	      rw->SetAngles(90.0 + mcell->mAssemblySideSlope, 0.0 + 180.0, 90.0, 90.0 + 180.0,  0.0 - mcell->mAssemblySideSlope, 0.0);
	    else
	      rw->SetAngles(90.0 + mcell->mAssemblySideSlope, 0.0,         90.0, 90.0,          0.0 + mcell->mAssemblySideSlope, 0.0);
	    
	    vcell->AddNode(vside, lr, new TGeoCombiTrans(0.1 *xOffset, 0.0, 0.1 * hOffset, rw));
	  } //for lr
	}
      } //if
    }
  }

  //cout << GetMedium(mCarbonFiberMaterial)->GetMaterial()->GetRadLen() << endl; exit(0);

  // Place a single equivalent thickness layer parallel to the assembly base;
  if (GetGeometryType() == MapsGeoParData::NoStructure)
  {
    // FIXME: may want to re-write this stuff in a better way once get rid of CINT (so that 
    // at least std::vector is available);
    double aluRadLen, kaptonRadLen, carbonRadLen, waterRadLen;

    //printf("%f %f %f %f\n", aluThickness, kaptonThickness, carbonThickness, waterThickness);

    aluRadLen    = 0.1 * aluThickness    / GetMedium(_ALUMINUM_)->GetMaterial()->GetRadLen();
    kaptonRadLen = 0.1 * kaptonThickness / GetMedium(mKaptonMaterial)->GetMaterial()->GetRadLen();
    carbonRadLen = 0.1 * carbonThickness / GetMedium(mCarbonFiberMaterial)->GetMaterial()->GetRadLen();
    waterRadLen  = 0.1 * waterThickness  / GetMedium(_WATER_)->GetMaterial()->GetRadLen();
    //printf("%f %f %f %f\n", 100*aluRadLen, 100*kaptonRadLen, 100*carbonRadLen, 100*waterRadLen);

    // Effective carbon fiber layer thickness;
    double equivalentCarbonThickness = 10. * (aluRadLen + kaptonRadLen + carbonRadLen + waterRadLen)*
      GetMedium(mCarbonFiberMaterial)->GetMaterial()->GetRadLen();
    //printf("%f\n", equivalentCarbonThickness);

    // NB: full thickness may not fit into the triangular profile above the silicon chip layer -> 
    // split into two; first layer goes below silicon chip, second one (if needed) - above it;
    {
      double offsetFromBottomSide = mMimosaOffset + mAssemblyHeight/2;
      // Let first layer mask itself as a flex cable;
      double equivalentFlexThickness = 
	equivalentCarbonThickness < offsetFromBottomSide ? equivalentCarbonThickness : offsetFromBottomSide;

      TGeoBBox *flex = new TGeoBBox(flexName,
				    0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				    0.1 * mAssemblyLength/2,
				    0.1 * equivalentFlexThickness/2);
      TGeoVolume *vflex = new TGeoVolume(flexName, flex, GetMedium(mCarbonFiberMaterial));
      
      vcell->AddNode(vflex, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * (equivalentFlexThickness - mAssemblyHeight)/2, 0));

      double equivalentColdThickness = equivalentCarbonThickness - equivalentFlexThickness;

      if (equivalentColdThickness > 0.0) {
	// Let it be called cold plate;
	TGeoBBox *cold = new TGeoBBox(coldPlateName,
				      0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				      0.1 * mAssemblyLength/2,
				      0.1 * equivalentColdThickness/2);
	TGeoVolume *vcold = new TGeoVolume(coldPlateName, cold, GetMedium(mCarbonFiberMaterial));
	
	// Yes, zOffset is still a valid current variable here; there will be a gap because of 
	// missing pipes; ignore;
	vcell->AddNode(vcold, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * (zOffset + equivalentColdThickness/2), 0));
      } //if
    }
  } //if

  return vcell;
} // MapsGeoParData::ConstructMimosaCell()

// ---------------------------------------------------------------------------------------

MapsStave *MapsGeoParData::ConstructStave(unsigned chipNum, unsigned id, 
					  MapsMimosaAssembly *mcell)
{
  const char *detName = mDetName->Name().Data();

  TGeoVolume *vcell = ConstructMimosaCell(mcell, id);

  char staveName[128], waterPipeName[128], waterName[128];
  char bracketName[128], waterTorusPipeName[128], waterTorusName[128];

  snprintf(staveName,            128-1, "%sStave%02d",               detName, id);
  snprintf(bracketName,          128-1, "%sStaveBracket%02d",        detName, id);
  snprintf(waterTorusPipeName,   128-1, "%sStaveWaterPipeTorus%02d", detName, id);
  snprintf(waterTorusName,       128-1, "%sStaveWaterTorus%02d",     detName, id);
  snprintf(waterPipeName,        128-1, "%sStaveWaterPipe%02d",      detName, id);
  snprintf(waterName,            128-1, "%sStaveWater%02d",          detName, id);

  MapsStave *stave = new MapsStave(staveName);
  //stave->SetName(staveName);

  //
  // Compose stave volume; yes, for VST they are simple TRD1 volumes; for now just 
  // take it of exactly N time chip assembly size + a bracket and pipes;
  //

  // NB: for now do not mind to do stave setup front/back symmetric 
  // in terms of elementary cell location;
  //stave->mLength = chipNum * mAssemblyLength + 2*mEnforcementBracketThickness + 
  //2*mWaterPipeExtensionLength;
  stave->mLength = GetExpectedStaveLength(chipNum, mcell);

  if (UseTriangularAssemblies()) {
    TGeoTrd1 *qstave = new TGeoTrd1(staveName,
				    0.1 * mcell->mAssemblyBaseWidth/2,
				    0.0,
				    0.1 * stave->mLength/2,
				    0.1 * mAssemblyHeight/2);
    stave->mVolume = new TGeoVolume(staveName, qstave, GetMedium(_AIR_));
  }
  else {
    char tmp1Name[128], tmp2Name[128];

    snprintf(tmp1Name,              128-1, "%sTmp1Volume%02d",          detName, id);
    snprintf(tmp2Name,              128-1, "%sTmp2Volume%02d",          detName, id);

    assert(0);
    // Same stuff as in MapsGeoParData::ConstructMimosaCell();
    TGeoTrd1 *tmp1 = new TGeoTrd1(tmp1Name,
				  0.1 * mcell->mAssemblyBaseWidth/2,
				  0.0,
				  0.1 * stave->mLength/2,
				  0.1 * mAssemblyHeight/2);
    TGeoBBox *tmp2 = new TGeoBBox(tmp2Name,
				  0.1 * mcell->mAssemblyBaseWidth/2,
				  //0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				  0.1 * stave->mLength/2,
				  0.1 * mAssemblyHeight/2);
    TGeoCompositeShape *cs = new TGeoCompositeShape(staveName, TString(tmp1Name) + "*" + TString(tmp2Name));

    stave->mVolume = new TGeoVolume(staveName, cs, GetMedium(_AIR_));
  } //if
  
  // Place chip assemblies into the stave volume;
  for(unsigned nn=0; nn<chipNum; nn++) {
    double yOffset = (nn - 0.5*(chipNum-1))*mAssemblyLength;
    
    stave->mVolume->AddNode(vcell, nn, new TGeoCombiTrans(0.0, 0.1 * yOffset, 0.0, 0));
  } //for nn

  // Place two brackets at the end of assembly; let them be always present, for all geometry types;
  if (WithEnforcementBrackets()) {
    TGeoTrd1 *bracket = new TGeoTrd1(bracketName,
				     0.1 * mcell->mAssemblyDeadMaterialWidth/2,
				     0.0,
				     0.1 * mEnforcementBracketThickness/2,
				     0.1 * mAssemblyHeight/2);
    TGeoVolume *vbracket = new TGeoVolume(bracketName, bracket, GetMedium(mCarbonFiberMaterial));
    
    for(unsigned ud=0; ud<2; ud++) {
      double yOffset = (ud ? -1.0 : 1.0)*(chipNum * mAssemblyLength + mEnforcementBracketThickness)/2;
      
      stave->mVolume->AddNode(vbracket, ud, new TGeoCombiTrans(0.0, 0.1 * yOffset, 0.0, 0));
    } //for ud
  } //if

  if (WithExternalPipes() &&
      GetGeometryType() == MapsGeoParData::FullStructure) {
    double waterPipeOuterDiameter = mcell->mWaterPipeInnerDiameter + 2*mcell->mWaterPipeWallThickness;
    
    // Place straight water pipe pieces at the upstream end; FIXME: unify with maps-lib.C later;
    {
      TGeoTube *wpipe = new TGeoTube(waterPipeName,
				     0.1 * mcell->mWaterPipeInnerDiameter/2,
				     0.1 * waterPipeOuterDiameter/2,
				     0.1 * mWaterPipeExtensionLength/2);
      TGeoVolume *vwpipe = new TGeoVolume(waterPipeName, wpipe, GetMedium(mKaptonMaterial));
      
      // Water itself;
      TGeoTube *water = new TGeoTube(waterName,
				     0.0,
				     0.1 * mcell->mWaterPipeInnerDiameter/2,
				     0.1 * mWaterPipeExtensionLength/2);
      TGeoVolume *vwater = new TGeoVolume(waterName, water, GetMedium(_WATER_));
      
      TGeoRotation *rw = new TGeoRotation();
      rw->RotateX(90);
      
      for(unsigned lr=0; lr<2; lr++) {
	double xOffset = (lr ? -1.0 : 1.0)*mWaterPipeXoffset;
	double yOffset = (stave->mLength - mWaterPipeExtensionLength)/2;
	
	stave->mVolume->AddNode(vwpipe, lr, new TGeoCombiTrans(0.1 * xOffset, 0.1 * yOffset, 0.1 * mWaterPipeZoffset, rw));
	stave->mVolume->AddNode(vwater, lr, new TGeoCombiTrans(0.1 * xOffset, 0.1 * yOffset, 0.1 * mWaterPipeZoffset, rw));
      } //for lr
    }
    
    // Place round U-pieces at the downstream end;
    {
      TGeoTorus *wtpipe = new TGeoTorus(waterTorusPipeName,
					0.1 * mWaterPipeXoffset,
					0.1 * mcell->mWaterPipeInnerDiameter/2,
					0.1 * waterPipeOuterDiameter/2,
					180.0, 180.);
      TGeoVolume *vwtpipe = new TGeoVolume(waterTorusPipeName, wtpipe, GetMedium(mKaptonMaterial));
      
      
      // Water itself;
      TGeoTorus *twater = new TGeoTorus(waterTorusName,
					0.1 * mWaterPipeXoffset,
					0.0, 
					0.1 * mcell->mWaterPipeInnerDiameter/2,
					180.0, 180.);
      TGeoVolume *vtwater = new TGeoVolume(waterTorusName, twater, GetMedium(_WATER_));
      
      double qyOffset = chipNum * mAssemblyLength/2 + mEnforcementBracketThickness;
      stave->mVolume->AddNode(vwtpipe,  0, new TGeoCombiTrans(0.0, -0.1 * qyOffset, 0.1 * mWaterPipeZoffset, 0));
      stave->mVolume->AddNode(vtwater,  0, new TGeoCombiTrans(0.0, -0.1 * qyOffset, 0.1 * mWaterPipeZoffset, 0));
    } 
  } //if

  return stave;
} // MapsGeoParData::ConstructStave()

// ---------------------------------------------------------------------------------------

MapsStave *MapsGeoParData::ConstructStaveWithMapping(unsigned chipNum, unsigned id, 
						     MapsMimosaAssembly *mcell)
{
  MapsStave *stave = ConstructStave(chipNum, id, mcell);

  // Well, disks can be rotated, so does not make sense to assume, that chips are 
  // numbered in vertical (Y) direction -> just use the first available index (X);
  AddLogicalVolumeGroup(chipNum);

  // NB: this implementation is really dumb and does not give anything like XY-location 
  // of chips in a disk; so just help the simulation code to identify chips and 
  // reconstruction code to get access to 3D transformation matrices;
  EicGeoMap *fgmap = CreateNewMap();
  fgmap->AddGeantVolumeLevel(mMimosaCoreName,   0);
  fgmap->AddGeantVolumeLevel(mMimosaShellName,  0);
  fgmap->AddGeantVolumeLevel(mCellAssemblyName, chipNum);
  // Right, all the staves are unique for now;
  fgmap->AddGeantVolumeLevel(stave->GetName(),  0);

  fgmap->SetSingleSensorContainerVolume(mMimosaCoreName);

  for(unsigned nn=0; nn<chipNum; nn++) {
      UInt_t geant[4] = {0, 0, nn, 0}, logical[1] = {nn};

      if (SetMappingTableEntry(fgmap, geant, id, logical)) {
	cout << "Failed to set mapping table entry!" << endl;
	exit(0);
      } //if
    } //for nn

  return stave;
} // MapsGeoParData::ConstructStaveWithMapping()

// ---------------------------------------------------------------------------------------

ClassImp(MapsStave)
ClassImp(MapsGeoParData)
