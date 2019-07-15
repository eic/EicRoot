
// Units are [mm]; will be rescaled to [cm] internally by fbgt->AddWheel() 
// when calling respective ROOT primitive creation routines;

// Meaningless numbers for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

fbgt()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  double centerChamDist[2] = { 420.0,  840.0};
  double distFirstWheel[2] = {1030.0, 2300.0};
  double distSecWheel  [2] = {1100.0, 2370.0};
  double distThiWheel  [2] = {1170.0, 2440.0};

  for(unsigned fb=0; fb<2; fb++) {
    GemGeoParData *fbgt = new GemGeoParData(fb ? "FFG" : "FGT", _VERSION_, _SUBVERSION_);

    // A single module; different for front and rear models; basically the rear sectors
    // (behind the RICH volume) are twice as large;
    GemModule *fit = new GemModule();

    fit->mActiveWindowBottomWidth  = fb ?   60.0 :  30.0;
    fit->mActiveWindowTopWidth     = fb ?  860.0 : 430.0;
    fit->mActiveWindowHeight       = fb ? 1400.0 : 700.0;
    
    //Frame parameters; thickness of 16.0 mm frame to frame with 2.0 mm thickness total of both frames, 
    //width of 8.0 mm on side, width of 30.0 mm top and bottom;
    fit->mFrameThickness  = 18.0;
    fit->mFrameBottomEdgeWidth  = 30.0;
    fit->mFrameTopEdgeWidth  = 30.0;
    fit->mFrameSideEdgeWidth  = 8.0;
    
    //Entrance window; FIXME: need Al layer;
    fit->mEntranceWindowMaterial  = "GemKapton";
    fit->mEntranceWindowThickness  = 0.025;
    
    //Foil parameters; Per FIT configuration, 5 um of Cu and 50 um of Kapton;
    fit->mDriftFoilKaptonThickness   =   0.050;
    fit->mDriftFoilCopperThickness   =   0.005;

    fit->mGemFoilAreaFraction        =   0.80;
    fit->mGemFoilKaptonThickness     =   0.050;
    //fit->mGemFoilCopperThickness     =   0.005;
    fit->mGemFoilCopperThickness = 0.00176; //2 Cr-GEM, 1 Std GEM
    
    fit->mReadoutG10Thickness        =   0.0;
    fit->mReadoutKaptonThickness     =   0.050;
    fit->mReadoutCopperThickness     =   0.005;
    
    //Emulation of exit window; FIXME: need Al layer;
    fit->mReadoutSupportMaterial     = "GemKapton";
    fit->mReadoutSupportThickness    =   0.025;
    
    // FIXME: check on that!;
    fit->mGasMixture                 = "arco27030";
    
    fit->mEntranceRegionLength       =  3.000;
    fit->mDriftRegionLength          =  3.000;
    // Assume triple GEM layout;
    fit->mFirstTransferRegionLength  =  2.000;
    fit->mSecondTransferRegionLength =  2.000;
    fit->mInductionRegionLength      =  2.000;
    
    // Fine, at this point structure is completely defined -> code it in ROOT;
    fbgt->AddWheel(fit, 12, centerChamDist[fb], distFirstWheel[fb]); 
    fbgt->AddWheel(fit, 12, centerChamDist[fb], distSecWheel  [fb]); 
    fbgt->AddWheel(fit, 12, centerChamDist[fb], distThiWheel  [fb]); 

    fbgt->AttachSourceFile("./fbgt.C");
        
    // Some colorification, please;
    fbgt->GetColorTable()->AddPatternMatch("FrameEdge",      kGray);
    fbgt->GetColorTable()->AddPatternMatch("EntranceWindow", kOrange);
    fbgt->GetColorTable()->AddPatternMatch("ReadoutSupport", kOrange);

    fbgt->ConstructGeometry();
  } //for fb
  
  // Yes, always exit;
  exit(0);
}

