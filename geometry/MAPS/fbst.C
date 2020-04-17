
// Meaningless number for now; fine;
#define _VERSION_     1
#define _SUBVERSION_  0

// Do not want to always overwrite "official" files; place "test" tag into the file name;
//#define _TEST_VERSION_

// All construction elements are smeared (so chip assembly is uniform in both
// beam line and asimuthal direction); 
//#define _NO_STRUCTURE_GEOMETRY_

// No tricky elements like inclined roof beams; still water pipes, etc are 
// created (so chip assembly is uniform in beam line direction only);
#define _BEAM_LINE_UNIFORM_GEOMETRY_

// FIXME: does not work; in case of FST/BST will have to fall back to tricky 
// TGeoCompositeShape cell assemblies; also half of th ecells should be Z-rotated 
// to bring chips close to the beam pipe;
#define _USE_TRIANGULAR_ASSEMBLIES_

// Comment out if no mounting rings wanted;
//#define _WITH_MOUNTING_RINGS_
// Comment out if no stave enforcement brackets wanted;
//#define _WITH_ENFORCEMENT_BRACKETS_
// Comment out if no external water pipe pieces wanted;
//#define _WITH_EXTERNAL_PIPES_

#include <./maps-lib.C>

//#define _DISK_NUM_  (7)
//-#define _DISK_NUM_  (8)
#define _DISK_NUM_  (6)
//#define _DISK_NUM_  (3)

fbst()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  // Assume 7 disks in both forward and backward directions; change symmetry later if needed;
  //const Double_t Z[_DISK_NUM_] = { 350., 500., 700., 900., 1020., 1120., 1220.};
  //@@@const Double_t Z[_DISK_NUM_] = { 270., 500., 700., 900., 1020., 1120., 1220.};
  const Double_t Z[_DISK_NUM_] = { 250., 400., 600., 800., 1000., 1210.};
  const Double_t R[_DISK_NUM_] = {   0.,  45.,   0.,  45.,    0.,   45.};
  //-const Double_t Z[_DISK_NUM_] = { 250., 400., 600., 800., 1000., 1235., 1500., 1700.};
  //-const Double_t R[_DISK_NUM_] = {   0.,  45.,   0.,  45.,    0.,   45., 0., 45.};
  //const Double_t Z[_DISK_NUM_] = { 270., 400., 500., 600., 700., 900., 1220.};
  //const Double_t Z[_DISK_NUM_] = {300., 500., 800.};

  for(unsigned fb=0; fb<2; fb++) {
    FstGeoParData *fbst = new FstGeoParData(fb ? "BST" : "FST", _VERSION_, _SUBVERSION_);

    // Parse #define statements and make certain configuration calls accordingly;
    // NB: cast to the base MAPS geo class since void* interface used; FIXME!; 
    DefinitionParser((MapsGeoParData*)fbst);

    //
    // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
    // definition routines only;
    //

    // Mounting ring construction; arbitrary numbers, same for all layers;
    if (fbst->WithMountingRings()) {
      fbst->mMountingRingBeamLineThickness =    3.00;
      fbst->mMountingRingRadialThickness   =    5.00;
    } //if
    // Simplify the design -> just a triangular piece with a reasonable volume; 
    if (fbst->WithEnforcementBrackets())
      fbst->mEnforcementBracketThickness   =    1.00;
    // This is something for display purposes mostly;
    if (fbst->WithExternalPipes())
      fbst->mWaterPipeExtensionLength      =    4.00;

    //
    //  For now assume ALICE Inner Barrel design, composed in staves with 
    // varying number of Mimosa chips;
    //

    MapsMimosaAssembly *ibcell = (MapsMimosaAssembly*)ConfigureAliceCell();

    //   For now consider a single disc design (all the same); add staves by hand;
    // Parameters:
    //     - cell assembly pointer;
    //     - min available radius;
    //     - max available radius;
    //     - default neigboring stave overlap in "X" direction;
    //+FstDisc *disc = new FstDisc(ibcell, 18.0, 197.0, 12.8);
    FstDisc *disc = new FstDisc(ibcell, 18.0, 185.0, 12.8);

    // Declare discs; just put them by hand at hardcoded locations along the beam line;
    for(unsigned dc=0; dc<_DISK_NUM_; dc++) 
      fbst->AddDisc(disc, (fb ? -1.0 : 1.0)*Z[dc], R[dc]);
#if 0
    if (!fb) {
      FstDisc *hdisc = new FstDisc(ibcell, 18.0, 400.0, 12.8);

      fbst->AddDisc(hdisc, 1500.);
    } //if
#endif

    fbst->AttachSourceFile("fbst.C");
    fbst->AttachSourceFile("maps-lib.C");
    fbst->AttachSourceFile("../../eic/detectors/maps/FstGeoParData.cxx");

    // Specify color preferences; NB: void* interface, sorry;
    SetMapsColors((EicGeoParData*)fbst);
    //fbst->GetTransparencyTable()->AddPatternMatch("MountingRing",   50);

    // Geometry is defined -> build it in ROOT;
    fbst->ConstructGeometry();
  } //for fb
  
  // Yes, always exit;
  exit(0);
}

