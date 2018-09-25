//
// AYK (ayk@bnl.gov), 2014/08/05
//
//  TPC geometry description file;
//

#include <EicGeoParData.h>

#ifndef _TPC_GEO_PAR_DATA_
#define _TPC_GEO_PAR_DATA_

// Do not see a reason not to fix this;
#define _TPC_DETECTOR_NAME_ ("TPC")

class TpcGeoParData: public EicGeoParData
{
 private:
  void ResetVars() {
    //mBeamLineOffset = 0.0;
    mInnerGasVolumeRadius = mOuterGasVolumeRadius = mTotalGasVolumeLength = 0.0;
    mKaptonBarrelThickness = mAluBarrelThickness = 0.0;
    mIfcCarbonFiberThickness = mOfcCarbonFiberThickness = 0.0;
    mCentralMembraneThickness = mAluEndcapThickness = 0.0;
  };

 public:
 TpcGeoParData(int version = -1, int subVersion = 0): 
  EicGeoParData(_TPC_DETECTOR_NAME_, version, subVersion) { ResetVars();};

  //void CalculateInnerRadius() {
  //mInnnerRadius = mInnerGasVolumeRadius - mKaptonBarrelThickness -
  //  mAluBarrelThickness - mIfcCarbonFiberThickness;
  //};

  // NB: these are the GAS VOLUME dimensions; 
  Double_t mInnerGasVolumeRadius;     // inner gas volume radius
  Double_t mOuterGasVolumeRadius;     // outer gas volume radius
  Double_t mTotalGasVolumeLength;     // overall gas volume length

  // Will be simulated as 50um alu on 150um effective single capton layers; 
  Double_t mKaptonBarrelThickness;    // kapton inner and out barrel layer thickness
  Double_t mAluBarrelThickness;       // kapton metallization 

  // Inner and outer field cage frames; kapton+alu+carbon thickness should 
  // match expected rad.length (see tpc.C);
  Double_t mIfcCarbonFiberThickness;  // carbon fiber thickness of inner field cage barrel
  Double_t mOfcCarbonFiberThickness;  // carbon fiber thickness of outer field cage barrel

  Double_t mCentralMembraneThickness; // central membrane thickness

  // For now assume just a solid no-structure alu plate;
  Double_t mAluEndcapThickness;       // thickness of endcap readout support structure

  // NB: axis is assumed to be aligned to the lepton beam; 
  //Double_t mBeamLineOffset;     // TPC offset along the hadron beam line direction

  //static TpcGeoParData *Instance() { return mInstance; };
  //static double GetInnerRadius()   { return mInnnerRadius; };

  //static TpcGeoParData *mInstance;
  //static double mInnnerRadius;

  ClassDef(TpcGeoParData,7);
};

#endif
