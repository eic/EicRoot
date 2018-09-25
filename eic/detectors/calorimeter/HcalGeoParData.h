//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  HCAL (Hadronic calorimeter) specific data definitions
//

#include <EndcapGeoParData.h>
#include <FiberParData.h>

#ifndef _HCAL_GEO_PAR_DATA_
#define _HCAL_GEO_PAR_DATA_

class HcalGeoParData: public EndcapGeoParData
{
 public:
 HcalGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  EndcapGeoParData(detName, version, subVersion) { ResetVars(); };
  ~HcalGeoParData() {};

  void ResetVars() {
    mSubCellNum = 0; mSubCellLength = 0.0;
    mLeadPlateThickness = mLeadPlateWidth = mLeadPlateHeight = 0.0;
    mScintillatorPlateThickness = mScintillatorPlateWidth = mScintillatorPlateHeight = 0.0;
    mWlsPlateThickness = mWlsPlateLength = mWlsPlateHeight = 0.0;
    mSteelFrontPlateThickness = mSteelFrontPlateSlope = 0.0;
    mSteelSpacerThickness = mMylarThickness = 0.0;
    mRotationY = 0.0;
    mPinLength = mPinDiameter = mPinToPinDistance = 0.0;
  };

  Int_t mSubCellNum;                    // number of tile+lead sub-cells
  Double_t mSubCellLength;              // longitudinal length of tsuch a sub-cell

  Double_t mLeadPlateThickness;         // sub-cell lead absorber (as well as rare steel plate) thickness
  Double_t mLeadPlateWidth;             // lead plate width
  Double_t mLeadPlateHeight;            // lead plate height

  Double_t mScintillatorPlateThickness; // scintillator plate thickness
  Double_t mScintillatorPlateWidth;     // scintillator plate width
  Double_t mScintillatorPlateHeight;    // scintillator plate height

  Double_t mWlsPlateThickness;          // WLS plate thickness
  Double_t mWlsPlateLength;             // full WLS plate length along cell axis
  Double_t mWlsPlateHeight;             // WLS plate height

  Double_t mMylarThickness;             // mylar layer thickness along WLS plate (on both sides)

  Double_t mSteelSpacerThickness;       // top steel plate thickness

  Double_t mPinLength;                  // full steel pin length (as shared between two rows)
  Double_t mPinDiameter;                // pin diameter
  Double_t mPinToPinDistance;           // pin-to-pin distance in X-direction

  Double_t mSteelFrontPlateThickness;   // front steel plate thickness at the middle
  Double_t mSteelFrontPlateSlope;       // front steel plate slope in [degree]

  Double_t mRotationY;                  // overall Y-rotation (2 degree, relevant for T1018 test run only)

  ClassDef(HcalGeoParData,10);
};

#endif
