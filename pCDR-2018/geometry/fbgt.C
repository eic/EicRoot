
// Meaningless number for now; fine;
#define _VERSION_     2
#define _SUBVERSION_  0

// Do not want to always overwrite "official" files; place "test" tag into the file name;
//#define _TEST_VERSION_

// All construction elements are smeared (so chip assembly is uniform in both 
// beam line and asimuthal direction); 
//#define _NO_STRUCTURE_GEOMETRY_

// Comment out if no mounting rings not wanted;
//#define _WITH_MOUNTING_RINGS_

fbgt()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Configure SBS GEMs; they will be used as a sample for other modules;
  GemModule *sbs = new GemModule();

  sbs->mActiveWindowBottomWidth    =  110.0 - 80.0;
  sbs->mActiveWindowTopWidth       =  270.0 + 80.0 + 80.0;
  sbs->mActiveWindowHeight         =  300.0 + 2*150.0 + 100.0;

  // Frame parameters accoring to Kondo's sbsCrossSection.pdf file;
  sbs->mFrameThickness             =   18.0;
  sbs->mFrameBottomEdgeWidth       =   30.0;
  sbs->mFrameTopEdgeWidth          =   30.0;
  sbs->mFrameSideEdgeWidth         =    8.0;

  // FIXME: put aluminum layer later as well;
  sbs->mEntranceWindowMaterial     = "GemKapton";
  sbs->mEntranceWindowThickness    =    0.025;
  
  // Use evaristo.pdf p.10 for the foil parameters:
  //  - drift foil    : 50um kapton + 3um copper;
  //  - GEM foil      : 30um kapton + 3um copper (80% area fraction); 
  //  - readout foils : 30um kapton + 3um copper total;
  sbs->mDriftFoilKaptonThickness   =   0.050;
  sbs->mDriftFoilCopperThickness   =   0.003;

  sbs->mGemFoilAreaFraction        =   0.80;
  sbs->mGemFoilKaptonThickness     =   0.030;
  sbs->mGemFoilCopperThickness     =   0.003;

  sbs->mReadoutG10Thickness        =   0.0;
  sbs->mReadoutKaptonThickness     =   0.030; 
  sbs->mReadoutCopperThickness     =   0.003;

  // 3mm thick Nomex honeycomb for SBS GEMs;
  sbs->mReadoutSupportMaterial     = "GemNomex";
  sbs->mReadoutSupportThickness    =   3.000;

  // FIXME: check on that!;
  sbs->mGasMixture                 = "arco27030";

  sbs->mEntranceRegionLength       =  3.000;
  sbs->mDriftRegionLength          =  3.000;
  // Assume triple GEM layout;
  sbs->mFirstTransferRegionLength  =  2.000;
  sbs->mSecondTransferRegionLength =  2.000;
  sbs->mInductionRegionLength      =  2.000;

  for(unsigned fb=0; fb<2; fb++) {
    GemGeoParData *fbgt = new GemGeoParData(fb ? "BGT" : "FGT", _VERSION_, _SUBVERSION_);

    if (fb) {      
      TGeoRotation *rw = new TGeoRotation();
      rw->RotateY(180); 

      fbgt->SetTopVolumeTransformation(new TGeoCombiTrans(0.0, 0.0, 0.0, rw));
    } //if

#ifdef _TEST_VERSION_
    fbgt->SetTestGeometryFlag();
#endif

#ifdef _WITH_MOUNTING_RINGS_
    fbgt->WithMountingRings();

    fbgt->mMountingRingBeamLineThickness =    5.00;
    fbgt->mMountingRingRadialThickness   =    5.00;
    //fbgt->mMountingRingRadialOffset      =    3.00;
#endif

    // FIXME: for now just make them fit the conical vacuum pipe;
    fbgt->AddWheel(sbs, 12, 370. + 65, 1030.0);
    fbgt->AddWheel(sbs, 12, 370. + 65, 1080.0);
    fbgt->AddWheel(sbs, 12, 370. + 65, 1130.0);

    fbgt->AttachSourceFile("./fbgt.C");
    
    //
    // Fine, at this point structure is completely defined -> code it in ROOT;
    //
    
    // Some colorification, please;
    fbgt->GetColorTable()->AddPatternMatch("FrameEdge",      kGray);
    fbgt->GetColorTable()->AddPatternMatch("EntranceWindow", kOrange);
    fbgt->GetColorTable()->AddPatternMatch("ReadoutSupport", kOrange);

    fbgt->ConstructGeometry();
  } //for fb  

  // Yes, always exit;
  exit(0);
}

