//
// AYK (ayk@bnl.gov), 2014/08/06
//
//  GEM geometry description file;
//

#include <iostream>
using namespace std;

#include <TGeoTube.h>
#include <TGeoVolume.h>
#include <TGeoMatrix.h>
#include <TGeoTrd1.h>
#include <TGeoArb8.h>
#include <TGeoPara.h>

#include <GemGeoParData.h>

// ---------------------------------------------------------------------------------------

int GemGeoParData::ConstructGeometry(bool root, bool gdml, bool check)
{
  const char *detName = mDetName->Name().Data();

  // Loop through all wheels (or perhaps single modules) independently;
  for(unsigned wl=0; wl<mWheels.size(); wl++) {
    GemWheel *wheel   = mWheels[wl];
    GemModule *module = wheel->mModule;

    // Assume top side is wider;
    double sideSlope = atan((module->mActiveWindowTopWidth - module->mActiveWindowBottomWidth)/
			    (2*module->mActiveWindowHeight));

    double moduleContainerHeight;
    TGeoVolume *vwcontainer, *vmcontainer;

    // Figure out parameters of the wheel container (air) volume first; 
    {
      double thickness = (wheel->mModuleNum == 1 ? 1 : 2)*module->mFrameThickness + 
	mMountingRingBeamLineThickness;
    
      double minRadius = wheel->mRadius - module->mActiveWindowHeight/2 - 
	module->mFrameBottomEdgeWidth;
      // Can perfectly be for a configuration with a single module FLYSUB "wheel";
      if (minRadius < 0.0) minRadius = 0.0;

      // Assume frame side width is given in a section parallel to the base; 
      double xx = module->mActiveWindowTopWidth/2 + module->mFrameTopEdgeWidth*tan(sideSlope) + 
	module->mFrameSideEdgeWidth;
      double yy = wheel->mRadius + module->mActiveWindowHeight/2 + module->mFrameTopEdgeWidth;
      double maxRadius = sqrt(xx*xx + yy*yy);

      char wheelContainerVolumeName[128];
      snprintf(wheelContainerVolumeName, 128-1, "%sWheelContainerVolume%02d", detName, wl);
      
      TGeoTube *wcontainer = new TGeoTube(wheelContainerVolumeName,
					 0.1 * minRadius,
					 0.1 * maxRadius,
					 0.1 * thickness/2);
      vwcontainer = new TGeoVolume(wheelContainerVolumeName, wcontainer, GetMedium(_AIR_));
		  
      GetTopVolume()->AddNode(vwcontainer, 0, wheel->mTransformation);
    }

    // Module container (air) volume; here can indeed use TRD1 volume;
    {
      char moduleContainerVolumeName[128];
      snprintf(moduleContainerVolumeName, 128-1, "%sModuleContainerVolume%02d", detName, wl);

      moduleContainerHeight = module->mFrameTopEdgeWidth + module->mFrameBottomEdgeWidth +
	module->mActiveWindowHeight;

      TGeoTrd1 *mcontainer = new TGeoTrd1(moduleContainerVolumeName,
					  0.1 * (module->mActiveWindowBottomWidth/2 + module->mFrameSideEdgeWidth - 
						 module->mFrameBottomEdgeWidth*tan(sideSlope)),
					  0.1 * (module->mActiveWindowTopWidth/2 + module->mFrameSideEdgeWidth + 
						 module->mFrameTopEdgeWidth*tan(sideSlope)),
					  0.1 *  module->mFrameThickness/2,
					  0.1 * moduleContainerHeight/2);
      vmcontainer = new TGeoVolume(moduleContainerVolumeName, mcontainer, GetMedium(_AIR_));

      // Place all the modules into the wheel container volume;
      for(unsigned md=0; md<wheel->mModuleNum; md++) {
	double effRadius = wheel->mRadius + (module->mFrameTopEdgeWidth - module->mFrameBottomEdgeWidth)/2;

	TGeoRotation *rw = new TGeoRotation();
	double degAngle = md*360.0/wheel->mModuleNum;
	double radAngle = degAngle*TMath::Pi()/180.0;
	rw->SetAngles(90.0, 0.0 - degAngle, 180.0,  0.0, 90.0, 90.0 - degAngle);
	
	double xOffset = effRadius*sin(radAngle);
	double yOffset = effRadius*cos(radAngle);
	double zOffset = wheel->mModuleNum == 1 ? 0.0 : 
	  (md%2 ? -1.0 : 1.0)*(module->mFrameThickness + mMountingRingBeamLineThickness)/2;
	
	vwcontainer->AddNode(vmcontainer, md, new TGeoCombiTrans(0.1 *xOffset, 0.1 * yOffset, 0.1 * zOffset, rw));
      } //for md
    }

    // And now put the other stuff piece by piece; unfortunately have to cook frame 
    // out of 4 pieces rather than a single TRD1, since otherwise drawing will 
    // become a nightmare (ROOT seems to be not able to draw inner walls at the
    // border of volume and its subvolume in a reasonable way);
#if _LATER_ 
    if (module->mActiveWindowTopWidth == module->mActiveWindowBottomWidth) {
      // An easy (square shape) frame and other volumes;
      assert(0);

    } 
    else
#endif 
    {
      //
      // A trapezoid shape; indeed could use TRD1 inner volumes and rotate them
      // accordingly; the trouble is that then I'd have to screw up local
      // coordinate system of the sensitive volume (so Z will be pointing in 
      // radial direction rather than along the beam line; consider to use
      // TGeoArb8 shape for this reason; CHECK: is there a performance penalty?;
      //

      char bottomFrameEdgeName[128], topFrameEdgeName[128], sideFrameEdgeName[128];

      // Want them to start with the same name pattern;
      snprintf(bottomFrameEdgeName, 128-1, "%sFrameEdgeBottom%02d", detName, wl);
      snprintf(topFrameEdgeName,    128-1, "%sFrameEdgeTop%02d",    detName, wl);
      snprintf(sideFrameEdgeName,   128-1, "%sFrameEdgeSide%02d",   detName, wl);
      
      // Bottom edge; here I can indeed use TRD1 volume;
      {
	TGeoTrd1 *bottom = new TGeoTrd1(bottomFrameEdgeName,
					0.1 * (module->mActiveWindowBottomWidth/2 - 
					       module->mFrameBottomEdgeWidth*tan(sideSlope)),
					0.1 * module->mActiveWindowBottomWidth/2,
					0.1 * module->mFrameThickness/2,
					0.1 * module->mFrameBottomEdgeWidth/2);
	TGeoVolume *vbottom = new TGeoVolume(bottomFrameEdgeName, bottom, GetMedium(mG10Material));
	
	double zOffset = -(moduleContainerHeight - module->mFrameBottomEdgeWidth)/2;
	vmcontainer->AddNode(vbottom, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, 0));
      }

      // Top edge; the same;
      {
	TGeoTrd1 *top = new TGeoTrd1(topFrameEdgeName,
				     0.1 *  module->mActiveWindowTopWidth/2,
				     0.1 * (module->mActiveWindowTopWidth/2 +
					    module->mFrameTopEdgeWidth*tan(sideSlope)),
				     0.1 * module->mFrameThickness/2,
				     0.1 * module->mFrameTopEdgeWidth/2);
	TGeoVolume *vtop = new TGeoVolume(topFrameEdgeName, top, GetMedium(mG10Material));
	
	double zOffset = (moduleContainerHeight - module->mFrameTopEdgeWidth)/2;
	vmcontainer->AddNode(vtop, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, 0));
      }

      // A pair of side edges; TGeoPara will do;
      TGeoPara *side = new TGeoPara(sideFrameEdgeName,
				    0.1 * module->mFrameSideEdgeWidth/2,
				    0.1 * module->mFrameThickness/2,
				    0.1 * moduleContainerHeight/2,
				    0.0, sideSlope*180/TMath::Pi(), 0.0);
      TGeoVolume *vside = new TGeoVolume(sideFrameEdgeName, side, GetMedium(mG10Material));
      for(unsigned lr=0; lr<2; lr++) {
	double xOffset = (lr ? -1.0 : 1.)*
	  (module->mActiveWindowBottomWidth/2 - module->mFrameBottomEdgeWidth*tan(sideSlope) + 
	   module->mFrameSideEdgeWidth/2 + (moduleContainerHeight/2)*tan(sideSlope));

	TGeoRotation *rw = 0;
	if (lr) {
	  rw = new TGeoRotation();
	  rw->RotateZ(180.0);
	} //if

	vmcontainer->AddNode(vside, lr, new TGeoCombiTrans(0.1 * xOffset, 0.0, 0.0, rw));
      } //for lr

      // Eventually populate single layers, one by one;
      {
	double yOffset = -module->mFrameThickness/2;

	// XY-projection shape does not change; it's only zOffset and thickness;
	double wdt = 0.1 * module->mActiveWindowTopWidth;
	double wdb = 0.1 * module->mActiveWindowBottomWidth;
	double ht  = 0.1 * module->mActiveWindowHeight;

	double vert[8][2] = {
	  {-wdb/2,   -ht/2},
	  {-wdt/2,    ht/2},
	  { wdt/2,    ht/2},
	  { wdb/2,   -ht/2},
	  {-wdb/2,   -ht/2},
	  {-wdt/2,    ht/2},
	  { wdt/2,    ht/2},
	  { wdb/2,   -ht/2}
	};

	// 
	// Proceed in direction opposite to the incident particles; whatever
	// is left in thickness will remain air in front of the entrance foil;
	//

	// This readout support stuff is optionally present;
	if (module->mReadoutSupportThickness)
	  PlaceMaterialLayer(detName, "ReadoutSupport", wl, vmcontainer, 
			     module->mReadoutSupportMaterial.Data(), (double*)vert,
			     module->mReadoutSupportThickness, &yOffset); 
	if (module->mReadoutG10Thickness)
	  PlaceMaterialLayer(detName, "ReadoutG10", wl, vmcontainer, 
			     mG10Material.Data(), (double*)vert,
			     module->mReadoutG10Thickness, &yOffset); 

	// Readout foil is always there;
	PlaceMaterialLayer(detName, "ReadoutKapton", wl, vmcontainer, 
			   mKaptonMaterial.Data(), (double*)vert,
			   module->mReadoutKaptonThickness, &yOffset); 

	// Cooper layers are extremely thin -> put one effective layer;
	{
	  double thickness = module->mReadoutCopperThickness +  
	    // 3x kapton layer, double-sided metallization;
	    3*2*module->mGemFoilAreaFraction*module->mGemFoilCopperThickness +
	    module->mDriftFoilCopperThickness;

	  PlaceMaterialLayer(detName, "EffectiveCopper", wl, vmcontainer, 
			     _COPPER_, (double*)vert,
			     thickness, &yOffset); 
	}

	// Induction region;
	{
	  PlaceMaterialLayer(detName, "InductionRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), (double*)vert,
			     module->mInductionRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "InductionRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), (double*)vert,
			     module->mGemFoilAreaFraction*module->mGemFoilKaptonThickness, 
			     &yOffset); 
	}

	// 2-d transfer region;
	{
	  PlaceMaterialLayer(detName, "SecondTransferRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), (double*)vert,
			     module->mSecondTransferRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "SecondTransferRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), (double*)vert,
			     module->mGemFoilAreaFraction*module->mGemFoilKaptonThickness, 
			     &yOffset); 
	}

	// 1-st transfer region;
	{
	  PlaceMaterialLayer(detName, "FirstTransferRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), (double*)vert,
			     module->mFirstTransferRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "FirstTransferRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), (double*)vert,
			     module->mGemFoilAreaFraction*module->mGemFoilKaptonThickness, 
			     &yOffset); 
	}

	// drift region;
	{
	  // NB: this is the sensitive volume!;
	  PlaceMaterialLayer(detName, "DriftRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), (double*)vert,
			     module->mDriftRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "DriftRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), (double*)vert,
			     module->mDriftFoilKaptonThickness, &yOffset); 
	}

	// entrance region;
	{
	  PlaceMaterialLayer(detName, "EntranceRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), (double*)vert,
			     module->mEntranceRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "EntranceWindow", wl, vmcontainer, 
			     module->mEntranceWindowMaterial.Data(), (double*)vert,
			     module->mEntranceWindowThickness, &yOffset); 
	}

	printf("-> %f\n", yOffset);
      }
    } //if

    {
      AddLogicalVolumeGroup(wheel->mModuleNum);
      // Yes, carelessly create one map per layer;
      EicGeoMap *fgmap = CreateNewMap();

      // FIXME: do it better later;
      char volumeName[128];
      snprintf(volumeName, 128-1, "%s%s%02d", detName, "DriftRegionGas", wl);
      fgmap->AddGeantVolumeLevel(volumeName,   0);

      fgmap->SetSingleSensorContainerVolume(volumeName);

      snprintf(volumeName, 128-1, "%sModuleContainerVolume%02d", detName, wl);
      fgmap->AddGeantVolumeLevel(volumeName,   wheel->mModuleNum);

      for(unsigned md=0; md<wheel->mModuleNum; md++) {
	  UInt_t geant[2] = {0, md}, logical[1] = {md};

	  if (SetMappingTableEntry(fgmap, geant, wl, logical)) {
	    cout << "Failed to set mapping table entry!" << endl;
	    exit(0);
	  } //if
	} //for md
	    }
  } //for wl

  // Place this stuff as a whole into the top volume and write out;
  FinalizeOutput(root, gdml, check);

  return 0;
} // GemGeoParData::ConstructGeometry()

// ---------------------------------------------------------------------------------------

void GemGeoParData::PlaceMaterialLayer(const char *detName, const char *volumeNamePrefix, 
				       unsigned wheelID, 
				       TGeoVolume *moduleContainer, const char *material, 
				       double *vert, double thickness, double *yOffset)
{
  char volumeName[128];
  GemWheel *wheel   = mWheels[wheelID];
  GemModule *module = wheel->mModule;

  snprintf(volumeName, 128-1, "%s%s%02d", detName, volumeNamePrefix, wheelID);

  TGeoArb8 *shape = new TGeoArb8(volumeName, 0.1 * thickness/2, vert); 

  TGeoVolume *vshape = new TGeoVolume(volumeName, shape, GetMedium(material));

  double zOffset = -(module->mFrameTopEdgeWidth - module->mFrameBottomEdgeWidth)/2;
  TGeoRotation *rw = new TGeoRotation();
  rw->RotateX(90.0);
  moduleContainer->AddNode(vshape, 0, new TGeoCombiTrans(0.0, 0.1 * (*yOffset + thickness/2), 
							 0.1 * zOffset, rw));

  *yOffset += thickness;
} // GemGeoParData::PlaceMaterialLayer()

// ---------------------------------------------------------------------------------------

ClassImp(GemModule)
ClassImp(GemWheel)
ClassImp(GemGeoParData)
