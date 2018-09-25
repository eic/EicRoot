
// Meaningless number for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

// Do not want to always overwrite "official" files; place "test" tag into the file name;
#define _TEST_VERSION_

// No real structure for now, clear;
#define _NO_STRUCTURE_GEOMETRY_

// Give up something like 1" extra on both inner and outer radial size;
// so effectively the radial thickness of the sensitive volume will be 
// 2x25.0mm less than Rmax-Rmin value; for now assume I can just declare 
// a smaller tube volume and let "world air" fill the radial gap; 
// FIXME: eventually put this into the parameter list;
#define _GAS_VOLUME_RADIAL_GAP_ 25.0
// Just for a better visibility?;
//#define _GAS_VOLUME_LINEAR_GAP_  5.0

tpc()
{
  //TString filename = "tpc.root";

  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  //
  // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
  // definition routines only;
  //

  TpcGeoParData *tpc = new TpcGeoParData(_VERSION_, _SUBVERSION_);

  // No offset per default;
  //tpc->SetTopVolumeTransformation(new TGeoTranslation(0.0, 0.0, 0.0));

#ifdef _NO_STRUCTURE_GEOMETRY_
  tpc->SetGeometryType(EicGeoParData::NoStructure);
#endif
#ifdef _TEST_VERSION_
  tpc->SetTestGeometryFlag();
#endif

  //
  // Declare basic parameters;
  //

  // NB: these are the GAS VOLUME dimensions; 
  tpc->mInnerGasVolumeRadius     =  200.0;
  tpc->mOuterGasVolumeRadius     =  800.0;
  tpc->mTotalGasVolumeLength     = 1960.0;

  // Shielding will be simulated as 50um alu on 150um effective single capton 
  // layers; the rest will be taken as lower-than-diamond density 
  // (1.76g/cm^3, see media.geo) carbon fiber layer in order to match 
  // overall rad.length values of ~1.2% for IFC and ~4.0% for OFC (just tune 
  // via macro/rad_length.C media scan by hand);
  tpc->mKaptonBarrelThickness    =    0.150;
  tpc->mAluBarrelThickness       =    0.050;
  tpc->mIfcCarbonFiberThickness  =    2.620;
  tpc->mOfcCarbonFiberThickness  =    9.280;

  // Central membrane; let it be 100um thick kapton;
  tpc->mCentralMembraneThickness =    0.100; 

  // Will be simulated as aluminum disks matching 15% rad.length;
  tpc->mAluEndcapThickness       =   13.4;

  tpc->AddLogicalVolumeGroup(0, 0, 2);

  // Field cage layers will have this length;
  double barrelLayerLength = tpc->mTotalGasVolumeLength + tpc->mCentralMembraneThickness;

  // Mapping table: just two independent gas volumes;
  EicGeoMap *fgmap = tpc->CreateNewMap();
  fgmap->AddGeantVolumeLevel("TpcGas", 2);
  fgmap->SetSingleSensorContainerVolume("TpcGas");

  tpc->AddStepEnforcedVolume("TpcGas");

  //
  // Well, consider to put ALL volumes independently (so they will appear on the same 
  // level in the geometry tree;
  //

  // A pair of gas volumes (upstream and downstream);
  {
    // Yes, ~2m is a full length -> divide by 2 here to get one-half full length;
    double singleHalfLength = tpc->mTotalGasVolumeLength/2;

    TGeoTube *gas = new TGeoTube("TpcGas", 
				 0.1 * (tpc->mInnerGasVolumeRadius + _GAS_VOLUME_RADIAL_GAP_),
				 0.1 * (tpc->mOuterGasVolumeRadius - _GAS_VOLUME_RADIAL_GAP_), 
				 0.1 *  singleHalfLength/2);
    //0.1 * (singleHalfLength/2 - _GAS_VOLUME_LINEAR_GAP_));
    TGeoVolume *vgas = new TGeoVolume("TpcGas", gas, tpc->GetMedium("ArCF4iC4H10"));

    for(unsigned ud=0; ud<2; ud++) {
      // Insert mapping table entry; a trivial one, indeed;
      UInt_t geant[1] = {ud}, logical[3] = {0, 0, ud};
      if (tpc->SetMappingTableEntry(fgmap, geant, 0, logical)) {
	cout << "Failed to set mapping table entry!" << endl;
	exit(0);
      } //if

      double zOffset = (ud ? -1.0 : 1.0)*(tpc->mCentralMembraneThickness + singleHalfLength)/2;
  
      // Yes, prefer to rotate right here -> local coordinates will point towards 
      // the registering pads for both upstream and downstream halves;
      {
	TGeoRotation *rw = new TGeoRotation();

	if (ud) rw->RotateY(180); 

	tpc->GetTopVolume()->AddNode(vgas, ud, new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, rw));
      }
    } //for up
  }

  // Inner field cage layers;
  {
    double radialOffset = tpc->mInnerGasVolumeRadius;

    //#if _TODAY_
    // Carbon fiber barrel;
    {
      TGeoTube *ifcC = new TGeoTube("TpcIfcCarbon", 
				    0.1 * (radialOffset - tpc->mIfcCarbonFiberThickness),
				    0.1 *  radialOffset,
				    0.1 * barrelLayerLength/2);
      TGeoVolume *vifcC = new TGeoVolume("TpcIfcCarbon", ifcC, tpc->GetMedium("CarbonFiber"));
  
      tpc->GetTopVolume()->AddNode(vifcC, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

      radialOffset -= tpc->mIfcCarbonFiberThickness;
    }

    //#if _TODAY_
    // Kapton layer;
    {
      TGeoTube *ifcK = new TGeoTube("TpcIfcKapton", 
				    0.1 * (radialOffset - tpc->mKaptonBarrelThickness),
				    0.1 *  radialOffset,
				    0.1 * barrelLayerLength/2);
      TGeoVolume *vifcK = new TGeoVolume("TpcIfcKapton", ifcK, tpc->GetMedium("kapton"));
  
      tpc->GetTopVolume()->AddNode(vifcK, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

      radialOffset -= tpc->mKaptonBarrelThickness;
    }
    //#endif

    // Alu layer;
    {
      TGeoTube *ifcA = new TGeoTube("TpcIfcAlu", 
				    0.1 * (radialOffset - tpc->mAluBarrelThickness),
				    0.1 *  radialOffset,
				    0.1 * barrelLayerLength/2);
      TGeoVolume *vifcA = new TGeoVolume("TpcIfcAlu", ifcA, tpc->GetMedium("aluminum"));
  
      tpc->GetTopVolume()->AddNode(vifcA, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));
    }
  }

#if 1
  // Outer field cage layers;
  {
    double radialOffset = tpc->mOuterGasVolumeRadius;

    // Carbon fiber barrel;
    {
      TGeoTube *ofcC = new TGeoTube("TpcOfcCarbon", 
				    0.1 *  radialOffset,
				    0.1 * (radialOffset + tpc->mOfcCarbonFiberThickness),
				    0.1 * barrelLayerLength/2);
      TGeoVolume *vofcC = new TGeoVolume("TpcOfcCarbon", ofcC, tpc->GetMedium("CarbonFiber"));
  
      tpc->GetTopVolume()->AddNode(vofcC, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

      radialOffset += tpc->mOfcCarbonFiberThickness;
    }

    // Kapton layer;
    {
      TGeoTube *ofcK = new TGeoTube("TpcOfcKapton", 
				    0.1 *  radialOffset,
				    0.1 * (radialOffset + tpc->mKaptonBarrelThickness),
				    0.1 * barrelLayerLength/2);
      TGeoVolume *vofcK = new TGeoVolume("TpcOfcKapton", ofcK, tpc->GetMedium("kapton"));
  
      tpc->GetTopVolume()->AddNode(vofcK, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

      radialOffset += tpc->mKaptonBarrelThickness;
    }

    // Alu layer;
    {
      TGeoTube *ofcA = new TGeoTube("TpcOfcAlu", 
				    0.1 *  radialOffset,
				    0.1 * (radialOffset + tpc->mAluBarrelThickness),
				    0.1 * barrelLayerLength/2);
      TGeoVolume *vofcA = new TGeoVolume("TpcOfcAlu", ofcA, tpc->GetMedium("aluminum"));
  
      tpc->GetTopVolume()->AddNode(vofcA, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));
    }
  }

  // Central membrane; do not account _GAS_VOLUME_RADIAL_GAP_ here?;
  {
    TGeoTube *CM = new TGeoTube("TpcCentralMembrane", 
				0.1 * tpc->mInnerGasVolumeRadius,
				0.1 * tpc->mOuterGasVolumeRadius,
				0.1 * tpc->mCentralMembraneThickness/2);
    TGeoVolume *vCM = new TGeoVolume("TpcCentralMembrane", CM, tpc->GetMedium("kapton"));
    
    tpc->GetTopVolume()->AddNode(vCM, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));
  }

  // Alu endcaps;
  {
    TGeoTube *ecap = new TGeoTube("TpcEndcap", 
				  0.1 * (tpc->mInnerGasVolumeRadius - 
					 (tpc->mKaptonBarrelThickness +
					  tpc->mAluBarrelThickness +
					  tpc->mIfcCarbonFiberThickness)),
				  0.1 * (tpc->mOuterGasVolumeRadius +
					 (tpc->mKaptonBarrelThickness +
					  tpc->mAluBarrelThickness +
					  tpc->mOfcCarbonFiberThickness)),
				  0.1 * tpc->mAluEndcapThickness/2);
    TGeoVolume *vecap = new TGeoVolume("TpcEndcap", ecap, tpc->GetMedium("aluminum"));
    
    for(unsigned ud=0; ud<2; ud++) {
      double zOffset = (ud ? -1.0 : 1.0)*(barrelLayerLength + tpc->mAluEndcapThickness)/2;

      tpc->GetTopVolume()->AddNode(vecap, ud, new TGeoCombiTrans(0.0, 0.0, 0.1 * zOffset, 0));
    } //for ud
  }
#endif

  tpc->AttachSourceFile("./tpc.C");

  // Want to see gas volume only; let it also be a bit transparent?;
  tpc->GetColorTable()->AddPrefixMatch("TpcGas", kCyan);
  tpc->GetTransparencyTable()->AddPrefixMatch("TpcGas", 30);
  //-tpc->GetTransparencyTable()->AddPrefixMatch("TpcGas", 70);
  //-tpc->GetColorTable()->AddPrefixMatch("TpcIfc",    kGray+2);
  //-tpc->GetColorTable()->AddPrefixMatch("TpcOfc",    kGray+2);
  //-tpc->GetColorTable()->AddPrefixMatch("TpcEndcap", kGray+2);

  // And put this stuff as a whole into the top volume;
  tpc->FinalizeOutput();

  // Yes, always exit;
  exit(0);
}

