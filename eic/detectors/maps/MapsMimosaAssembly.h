//
// AYK (ayk@bnl.gov), 2014/08/11
//
//  MAPS Mimosa 34 container description;
//

#include <TMath.h>
#include <TObject.h>

#include <EicGeoParData.h>

#ifndef _MAPS_MIMOSA_ASSEMBLY_
#define _MAPS_MIMOSA_ASSEMBLY_

class MapsMimosaAssembly: public TObject {
 public:
  MapsMimosaAssembly() { ResetVars(); };
  MapsMimosaAssembly(MapsMimosaAssembly *source) { *this = *source; };
  ~MapsMimosaAssembly() {};
  
  void ResetVars() {
    mAssemblyBaseWidth = mAssemblySideSlope = mChipToChipGap = mAssemblyDeadMaterialWidth = 0.0;
    mApexEnforcementBeamDiameter = mBaseEnforcementBeamWidth = 0.0;

    mChipLength = mChipWidth = mChipThickness = mChipActiveZoneThickness = 0.0;

    mFlexCableKaptonThickness = mFlexCableAluThickness = mColdPlateThickness = 0.0;

    mWaterPipeInnerDiameter = mWaterPipeWallThickness = 0.0;

    mEnforcementStripWidth = mEnforcementStripThickness = 0.0;
  };

  //
  //  Data container -> no private section;
  //

  // Air container volume parameters sufficient to pack all the stuff;
  Double_t mAssemblyBaseWidth;          // air container TRD1 (with 0 width apex) volume base width
  Double_t mAssemblySideSlope;          // angle at the base of triangular container profile; [degree]
  Double_t mChipToChipGap;              // gap between neighboring chips on a stave

  // Space structure;
  Double_t mApexEnforcementBeamDiameter;// diameter of the thin support tube at the assembly apex
  Double_t mBaseEnforcementBeamWidth;   // width of support triangles at the assemble edges
  Double_t mEnforcementStripWidth;      // width of side wall enforcement strip
  Double_t mEnforcementStripThickness;  // thickness of side wall enforcement strip
  Double_t mSideWallThickness;          // thickness of container side walls

  // Basic Mimosa 34 chip parameters; pixel size does not matter here (will play 
  // a role during digitization only);
  Double_t mChipLength;                 // Mimosa 34 chip length along the stave (30mm)
  Double_t mChipWidth;                  // Mimosa 34 chip width (15mm)
  Double_t mChipThickness;              // Mimosa 34 silicon thickness (50um)
  Double_t mChipActiveZoneThickness;    // Mimosa 34 active layer thickness (assume 20um in the middle)
  Double_t mChipDeadAreaWidth;          // Mimosa 34 dead area along the long side (2mm or so)

  Double_t mAssemblyDeadMaterialWidth;  // cold head, etc can be wider than the Mimosa 34 chip

  // Layers at the base of the assembly;
  Double_t mFlexCableKaptonThickness;   // flex cable substrate thickness
  Double_t mFlexCableAluThickness;      // flex cable effective aluminum bus strips thickness
  Double_t mColdPlateThickness;         // cold plate thickness

  // Water pipes; assume 2 parallel pipes;
  Double_t mWaterPipeInnerDiameter;     // water pipe inner diameter
  Double_t mWaterPipeWallThickness;     // water pipe wall thickness

  double GetAssemblyLength() const { return mChipLength + mChipToChipGap; };

  double GetAssemblyHeight() const { return (mAssemblyBaseWidth/2)*
      tan(mAssemblySideSlope*TMath::Pi()/180.);
  };

  ClassDef(MapsMimosaAssembly,13);
};

#endif
