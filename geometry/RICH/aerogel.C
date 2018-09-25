
// Meaningless number for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

// Do not want to always overwrite "official" files; place "test" tag into the file name;
//#define _TEST_VERSION_

// All construction elements are smeared (so chip assembly is uniform in both 
// beam line and asimuthal direction); 
//#define _NO_STRUCTURE_GEOMETRY_

aerogel()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Forward & backward parts; 
  for(unsigned fb=0; fb<2; fb++) {
    EicGeoParData *fbagr = new EicGeoParData(fb ? "BLR" : "FLR", _VERSION_, _SUBVERSION_);

    if (fb) {      
      TGeoRotation *rw = new TGeoRotation();
      rw->RotateY(180); 

      fbagr->SetTopVolumeTransformation(new TGeoCombiTrans(0.0, 0.0, 0.0, rw));
    } //if

#ifdef _TEST_VERSION_
    fbagr->SetTestGeometryFlag();
#endif

    TGeoTube *outv = new TGeoTube("OuterVolumeFBAG", 
				  0.1 *  100.0,
				  0.1 * 1100.0,
				  0.1 *  250.0/2);
    TGeoVolume *voutv = new TGeoVolume("OuterVolumeFBAG", outv, fbagr->GetMedium("CF4"));

    //double zOffset = (fb ? -1.0 : 1.0)*1350.0;
  
    // Yes, prefer to rotate right here -> local coordinates will point towards 
    // the registering pads for both upstream and downstream halves;
    {
      TGeoRotation *rw = new TGeoRotation();
      
      if (fb) rw->RotateY(180); 
      
      fbagr->GetTopVolume()->AddNode(voutv, fb, new TGeoCombiTrans(0.0, 0.0, 0.1 * 1350.0, rw));
    }

    fbagr->AttachSourceFile("./aerogel.C");
    
    // Some colorification, please;
    fbagr->GetColorTable()->AddPrefixMatch       ("OuterVolume",  kPink+1); 
    fbagr->GetTransparencyTable()->AddPrefixMatch("OuterVolume",  30);

    // And put this stuff as a whole into the top volume;
    fbagr->FinalizeOutput();

    fbagr->ConstructGeometry();
  } //for fb

  // Barrel part;
  {
    EicGeoParData *ctagr = new EicGeoParData("CLR", _VERSION_, _SUBVERSION_);

#ifdef _TEST_VERSION_
    ctagr->SetTestGeometryFlag();
#endif
    TGeoTube *outv = new TGeoTube("OuterVolumeCTAG", 
				  0.1 *  850.0,
				  0.1 * 1100.0,
				  0.1 * 2300.0/2);
    TGeoVolume *voutv = new TGeoVolume("OuterVolumeCTAG", outv, ctagr->GetMedium("CF4"));

    ctagr->GetTopVolume()->AddNode(voutv, 0, new TGeoCombiTrans(0.0, 0.0, 0.0, 0));

    ctagr->AttachSourceFile("./aerogel.C");
    
    // Some colorification, please;
    ctagr->GetColorTable()->AddPrefixMatch       ("OuterVolume",  kPink+2); 
    ctagr->GetTransparencyTable()->AddPrefixMatch("OuterVolume",  30);

    // And put this stuff as a whole into the top volume;
    ctagr->FinalizeOutput();

    ctagr->ConstructGeometry();
  }

  // Yes, always exit;
  exit(0);
}

