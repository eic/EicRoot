//
// AYK (ayk@bnl.gov), 2014/08/07
//
//  MAPS geometry description file;
//

#include <EicGeoParData.h>
#include <MapsMimosaAssembly.h>

#ifndef _MAPS_GEO_PAR_DATA_
#define _MAPS_GEO_PAR_DATA_

// These materials are not really MAPS-specific; just take them from media.geo 
// shared section and use as-is;
#define _AIR_      ("air")
#define _WATER_    ("water")
#define _ALUMINUM_ ("aluminum")
#define _SILICON_  ("silicon")

class MapsStave: public TNamed
{
  friend class MapsGeoParData;

 public:
  MapsStave() { ResetVars(); };
 MapsStave(const char *name): TNamed(name, 0) { ResetVars(); };
  ~MapsStave() {};
  
  TGeoVolume *GetVolume() const { return mVolume; };
  const double GetLength()      const { return mLength; };

 private:
  TGeoVolume *mVolume;   //! ROOT TGeoVolume
  double mLength;        //! calculated stave length

  void ResetVars() {
    mVolume = 0;
    mLength = 0.0;
  };

  ClassDef(MapsStave,1);
};

class MapsGeoParData: public EicGeoParData
{
 private:

 public:
 MapsGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  EicGeoParData(detName, version, subVersion), mMountingRingBeamLineThickness(0.0), 
    mMountingRingRadialThickness(0.0), mWaterPipeExtensionLength(0.0),
    mEnforcementBracketThickness(0.0), mUseTriangularAssemblies(false),
    mWithMountingRings(false), mWithEnforcementBrackets(false), mWithExternalPipes(false),
    mCarbonFiberMaterial("MapsCarbonFiber"), mKaptonMaterial("MapsKapton") {
    mGeometryType = FullStructure;

    // Transient variables; 
    mMimosaCoreName[0] = mMimosaShellName[0] = mCellAssemblyName[0] = 0; 
    mAssemblyHeight = mAssemblyLength = mWaterPipeXoffset = mWaterPipeZoffset = mMimosaOffset = 0.0;
  };
  ~MapsGeoParData() {};

  // Add sort of mounting rings on both sides of each barrel layer; assume they are 
  // the same construction for all layers;
  Double_t mMountingRingBeamLineThickness; // mounting ring thickness in beam direction 
  Double_t mMountingRingRadialThickness;   // mounting ring thickness in radial direction
  
  Double_t mWaterPipeExtensionLength;      // length of water pipe external pieces
  Double_t mEnforcementBracketThickness;   // thickness of triangular bracket at both stave ends

  void UseTriangularAssemblies(bool yesNo) { mUseTriangularAssemblies = yesNo; };
  bool UseTriangularAssemblies()  const    { return mUseTriangularAssemblies; };

  void WithMountingRings(bool yesNo)       { mWithMountingRings       = yesNo; };
  bool WithMountingRings()        const    { return mWithMountingRings; };

  void WithEnforcementBrackets(bool yesNo) { mWithEnforcementBrackets = yesNo; };
  bool WithEnforcementBrackets()  const    { return mWithEnforcementBrackets; };

  void WithExternalPipes(bool yesNo)       { mWithExternalPipes       = yesNo; };
  bool WithExternalPipes()        const    { return mWithExternalPipes; };

  void SetCarbonFiberMaterial(const char *material) { mCarbonFiberMaterial = TString(material); };
  void SetKaptonMaterial     (const char *material) { mKaptonMaterial      = TString(material); };

 private:
  Bool_t mUseTriangularAssemblies; // either use simple triangular cell profile or composite shape

  Bool_t mWithMountingRings;       // either create or not mounting rings
  Bool_t mWithEnforcementBrackets; // either create or not enforcement brackets
  Bool_t mWithExternalPipes;       // either create or not external pieces op water pipes

 protected:
  TString mCarbonFiberMaterial;    // may want to specify a different carbon fiber in media.geo
  TString mKaptonMaterial;         // may want to specify a different kapton in media.geo

  // Transient (global) working variables; FIXME: do it better later; NB: have to be on different
  // lines, otherwise '//!' has effect only on the last item;
  char mMimosaCoreName  [128]; //!
  char mMimosaShellName [128]; //!
  char mCellAssemblyName[128]; //!
  double mAssemblyHeight;      //!
  double mAssemblyLength;      //!
  double mWaterPipeXoffset;    //!
  double mWaterPipeZoffset;    //! 
  double mMimosaOffset;        //!

  TGeoVolume *ConstructMimosaCell(MapsMimosaAssembly *mcell, unsigned id);

  double GetExpectedStaveLength(unsigned chipNum, MapsMimosaAssembly *mcell) {
    return chipNum * mcell->GetAssemblyLength() + 2*mEnforcementBracketThickness + 
      2*mWaterPipeExtensionLength;
  };

  double GetAssemblyContainerWidth(const MapsMimosaAssembly *mcell) const {
    return (UseTriangularAssemblies() ? mcell->mAssemblyBaseWidth : 
	    mcell->mAssemblyDeadMaterialWidth);
  };

  MapsStave *ConstructStave(unsigned chipNum, unsigned id, MapsMimosaAssembly *mcell);
  MapsStave *ConstructStaveWithMapping(unsigned chipNum, unsigned id, MapsMimosaAssembly *mcell);

  ClassDef(MapsGeoParData,10);
};

#endif
