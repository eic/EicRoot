
// Meaningless number for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

// Do not want to always overwrite "official" files; place "test" tag into the file name;
//#define _TEST_VERSION_

// All construction elements are smeared (so chip assembly is uniform in both 
// beam line and asimuthal direction); 
#define _NO_STRUCTURE_GEOMETRY_

mumegas()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  MuMegasGeoParData *mmt = new MuMegasGeoParData("MMT", _VERSION_, _SUBVERSION_);

#ifdef _TEST_VERSION_
  mmt->SetTestGeometryFlag();
#endif

  MuMegasLayer *layer = new MuMegasLayer();

  // FIXME: need FR4 here; 320um thick (e-mail from Maxence 2015/01/23);
  layer->mReadoutPcbMaterial        = "MuMegasG10";
  layer->mReadoutPcbThickness       =  0.320;

  // FIXME: this is effective thickness I guess?; 
  layer->mCopperStripThickness      =  0.010;

  // Let's say, Ar(70)/CO2(30), see Maxence' muMegas.C; 130um amplification region;
  layer->mGasMixture                = "arco27030mmg";
  layer->mAmplificationRegionLength =  0.130;

  // FIXME: will need 'steel' in media.geo;
  layer->mSteelMeshThickness        =  0.300 * (19./50.) * (19./50.);
  
  // 3mm conversion gap for now;
  layer->mConversionRegionLength    =  3.000;

  // Basically a placeholder for now; assume 25um kapton as entrance window;
  layer->mExitWindowMaterial        = "MuMegasKapton";
  layer->mExitWindowThickness       =  0.100;

  // FIXME: these parameters need to be checked and made real; the problem is 
  // that they will be sitting in the clear acceptance, so handle with care;
  // for instance PandaRoot tracker clearly gets confused (provides a bit 
  // biased momentum estimates when these frames are thick;
  layer->mInnerFrameWidth           =  4.00;
  layer->mInnerFrameThickness       =  4.00;
  layer->mOuterFrameWidth           =  4.00;
  layer->mOuterFrameThickness       =  4.00;

  // Parameters are: 
  //   - layer construction;
  //   - length;
  //   - segmentation in Z;
  //   - radius;
  //   - segmentation in phi;
  //   - Z offset from 0.0 (default);
  //   - asimuthat offset from 0.0 (default);
  mmt->AddBarrel(layer, 2000., 2, 825., 6, 0.0, 0.0);
  mmt->AddBarrel(layer, 2000., 2, 835., 6, 0.0, 0.0);

  mmt->AddBarrel(layer,  450., 1, 175., 3, 0.0, 0.0);
  //-mmt->AddBarrel(layer,  450., 1, 175., 1, 0.0, 0.0);
  mmt->AddBarrel(layer,  450., 1, 185., 3, 0.0, 0.0);
  //-mmt->AddBarrel(layer,  450., 1, 185., 1, 0.0, 0.0);

  mmt->AttachSourceFile("./mumegas.C");

  mmt->GetColorTable()->AddPatternMatch("Frame",      kGray);
  mmt->GetColorTable()->AddPatternMatch("ReadoutPcb", kOrange+2);
  mmt->GetColorTable()->AddPatternMatch("ExitWindow", kOrange+2);

  //
  // Fine, at this point structure is completely defined -> code it in ROOT;
  //

  mmt->ConstructGeometry();

  // Yes, always exit;
  exit(0);
}

