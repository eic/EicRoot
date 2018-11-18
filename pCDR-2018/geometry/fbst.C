
#define _VERSION_     2
#define _SUBVERSION_  0

#define _DISK_NUM_   (6)

fbst()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Well, consider to tune by hand for now; and for FST only;
  const Double_t         Z[_DISK_NUM_] = { 250., 400., 600., 800., 1000., 1180.};
  const Double_t      rMin[_DISK_NUM_] = {  35.,  35.,  35.,  40.,   46.,   51.};
  const Double_t   xOffset[_DISK_NUM_] = {   0.,   0.,   0.,   2.,    5.,    7.};

  // rMax is defined by the TPC inner field cage diameter; 
  double rMax                         = 185.0;
  //Double_t nominal_proton_direction   =   0.022;
  //Double_t proton_cone_opening_angle  =   0.020;
  // Cylindrical section around IP;
  //double cylindrical_beam_pipe_radius =  31.00;
  // Stay clear from the beam pipe;
  //double safety_margin                =   3.00;
  Double_t waferThickness             =   0.2;

  for(unsigned fb=0; fb<2; fb++) {
    EicGeoParData *fbst = new EicGeoParData(fb ? "BST" : "FST", _VERSION_, _SUBVERSION_);

    char wname[128];
    sprintf(wname, "%sSiliconWafer", fb ? "Bst" : "Fst");
    TGeoTube *wafer = new TGeoTube(wname,
				   0.0,
				   0.1 * rMax,
				   0.1 * waferThickness/2);

    for(unsigned wf=0; wf<_DISK_NUM_; wf++) {
      double z0 = Z[wf], r0 = rMin[wf], dx = fb ? 0.0 : xOffset[wf];

      char hname[128];
      sprintf(hname, "%sSiliconHole%02d", fb ? "Bst" : "Fst", wf);
      TGeoTube *hole = new TGeoTube(hname, 
				    0.0,
				    0.1 * r0,
				    0.1 * waferThickness/2 + 0.1);

      char tname[128];
      sprintf(tname, "%sCombi%02d", fb ? "Bst" : "Fst", wf);
      TGeoCombiTrans *combi = new TGeoCombiTrans(tname, 0.1 * dx, 0, 0, 0);
      combi->RegisterYourself();

      char vname[128], cname[128];
      sprintf(vname, "%sSiliconPlate%02d", fb ? "Bst" : "Fst", wf);
      sprintf(cname, "%sSiliconWafer-%s:%s", fb ? "Bst" : "Fst", hname, tname);
      TGeoCompositeShape *comp = new TGeoCompositeShape(vname, cname);

      TGeoVolume *vwafer = new TGeoVolume(vname, comp, fbst->GetMedium("silicon"));	

      fbst->GetTopVolume()->AddNode(vwafer, 0, new TGeoCombiTrans(0.0, 0.0, 0.1 * z0, 0));
    } //for iq
  } //for fb
  
  fbst->GetColorTable()->AddPatternMatch       ("Silicon", kYellow);
  fbst->GetTransparencyTable()->AddPatternMatch("Silicon", 50);

  fbst->FinalizeOutput();

  // Yes, always exit;
  exit(0);
}

