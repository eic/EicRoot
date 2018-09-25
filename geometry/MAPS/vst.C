
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
//#define _BEAM_LINE_UNIFORM_GEOMETRY_

// In case of VST no need to fall back to tricky TGeoCompositeShape cell assemblies;
#define _USE_TRIANGULAR_ASSEMBLIES_

// Comment out if no mounting rings wanted;
//#define _WITH_MOUNTING_RINGS_
// Comment out if no stave enforcement brackets wanted;
//#define _WITH_ENFORCEMENT_BRACKETS_
// Comment out if no external water pipe pieces wanted;
//#define _WITH_EXTERNAL_PIPES_

#include <./maps-lib.C>

vst()
{
  // Load basic libraries;
  gROOT->Macro("$VMCWORKDIR/gconfig/rootlogon.C");

  //
  // Prefer to think in [mm] and convert to [cm] when calling ROOT shape 
  // definition routines only;
  //

  VstGeoParData *vst = new VstGeoParData(_VERSION_, _SUBVERSION_);

  // Parse #define statements and make certain configuration calls accordingly;
  // NB: cast to the base MAPS geo class since void* interface used; FIXME!; 
  DefinitionParser((MapsGeoParData*)vst);

  //
  //  For now assume ALICE Inner Barrel design only (but with more than 9 chips);
  // no problem to introduce similar design with a bit different parameter set later; 
  // construction a-la ALICE Outer Barrel looks like an overkill to the moment;
  //

  // No offset per default;
  //vst->SetTopVolumeTransformation(new TGeoTranslation(0.0, 0.0, 0.0));

  //
  // NB: all these numbers are sort of arbitrary; allow to get better visual 
  //     representation, but do not reflect any reality of the final design;
  //

  // Mounting ring construction; arbitrary numbers, same for all layers;
  if (vst->WithMountingRings()) {
    vst->mMountingRingBeamLineThickness =    5.00;
    vst->mMountingRingRadialThickness   =    5.00;
    vst->mMountingRingRadialOffset      =    3.00;
  } //if
  // Simplify the design -> just a triangular piece with a reasonable volume; 
  if (vst->WithEnforcementBrackets())
    vst->mEnforcementBracketThickness   =    1.00;
  // This is something for display purposes mostly;
  if (vst->WithExternalPipes())
    vst->mWaterPipeExtensionLength      =    4.00;

  MapsMimosaAssembly *ibcell = (MapsMimosaAssembly*)ConfigureAliceCell();

  // If (much) longer staves needed, create an enforced configuration;
  //MapsMimosaAssembly *obcell = new MapsMimosaAssembly(ibcell);
  //obcell->mEnforcementBeamDiameter     =    1.00; ... and so on ...

  //
  // Now when basic building blocks are created, compose barrel layers;
  //
  // a dirty part, but perhaps the easiest (and most readable) to do; parameters are:
  //  - cell assembly type;
  //  - number of stoves in this layer;
  //  - number of chips in a stove;
  //  - chip center installation radius;
  //  - additional stove slope around beam line direction; [degree];
  //  - layer rotation around beam axis "as a whole"; [degree];
  //
  vst->AddBarrelLayer(ibcell,   12,  9,   23.4, 12.0, 0.0);
  vst->AddBarrelLayer(ibcell, 2*12,  9, 2*23.4, 12.0, 0.0);

  vst->AddBarrelLayer(ibcell, 6*12, 14, 6*23.4, 14.0, 0.0);
  vst->AddBarrelLayer(ibcell, 4*20, 14, 4*39.3, 14.0, 0.0);

  vst->AttachSourceFile("vst.C");
  vst->AttachSourceFile("maps-lib.C");
  vst->AttachSourceFile("../../eic/detectors/maps/VstGeoParData.cxx");

  // Specify color preferences; NB: void* interface, sorry;
  SetMapsColors((EicGeoParData*)vst);

  //
  // Fine, at this point structure is completely defined -> code it in ROOT;
  //

  vst->ConstructGeometry();

  // Yes, always exit;
  exit(0);
}

