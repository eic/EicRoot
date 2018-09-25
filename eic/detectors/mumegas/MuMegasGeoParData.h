//
// AYK (ayk@bnl.gov), 2015/01/28
//
//  MuMegas geometry description file;
//

#include <TMath.h>

#include <EicGeoParData.h>

#ifndef _MUMEGAS_GEO_PAR_DATA_
#define _MUMEGAS_GEO_PAR_DATA_

// FIXME: unify with the other codes later;
#define _AIR_      ("air")
//#define _ALUMINUM_ ("aluminum")
//#define _COPPER_   ("copper")

class MuMegasLayer: public TObject {
 public:
  MuMegasLayer() { ResetVars(); };
  MuMegasLayer(const MuMegasLayer *sample) { *this = *sample; };
  ~MuMegasLayer() {};

  //
  // POD public access;
  //

  // FIXME: need FR4 here; 200um thick;
  TString mReadoutPcbMaterial;         // readout PCB material; should be FR4? 
  Double_t mReadoutPcbThickness;       // PCB thickness                       -> eventually should be FR4?

  // FIXMR: this is effective thickness I guess?; 
  Double_t mCopperStripThickness;      // effective copper strips thickness   -> copper

  // Let's say, Ar(70)/CO2(30), see Maxence' muMegas.C; 120um amplification region;
  TString mGasMixture;                 // gas mixture
  Double_t mAmplificationRegionLength; // short amplification region length   -> gas mixture

  // FIXME: will need 'steel' in media.geo;
  Double_t mSteelMeshThickness;        // effective steel mesh thickness      -> steel
  
  // 3mm conversion gap for now;
  Double_t mConversionRegionLength;    // conversion region length            -> gas mixture

  // Basically a placeholder for now; assume 25um kapton as entrance window;
  TString mExitWindowMaterial;         // exit window material
  Double_t mExitWindowThickness;       // exit window thickness               -> kapton

  // FIXME: these parameters need to be checked and made real; 
  Double_t mInnerFrameWidth;           // inner frame width (wedge shape in fact)
  Double_t mInnerFrameThickness;       // inner frame thickness
  Double_t mOuterFrameWidth;           // outer frame width in beam direction
  Double_t mOuterFrameThickness;       // outer frame thickness

 private:
  void ResetVars() {
    mReadoutPcbThickness = mCopperStripThickness = mAmplificationRegionLength = 0.0;

    mSteelMeshThickness = mConversionRegionLength = mExitWindowThickness = 0.0;

    mInnerFrameWidth = mInnerFrameThickness = mOuterFrameWidth = mOuterFrameThickness;
  };

  ClassDef(MuMegasLayer,2);
};

class MuMegasBarrel: public TObject {
 public:
  MuMegasBarrel() { ResetVars(); };
 MuMegasBarrel(MuMegasLayer *layer, double length, unsigned beamLineSectionNum, 
	       double radius, unsigned sectorNum, TGeoMatrix *transformation):
  mLayer(layer), mLength(length), mBeamLineSectionNum(beamLineSectionNum), 
    mRadius(radius), mAsimuthalSectorNum(sectorNum), mTransformation(transformation) {};

 MuMegasBarrel(MuMegasLayer *layer, double length, unsigned beamLineSectionNum, 
	       double radius, unsigned sectorNum, double beamLineOffset, 
	       double beamLineRotation):
  mLayer(layer), mLength(length), mBeamLineSectionNum(beamLineSectionNum), 
    mRadius(radius), mAsimuthalSectorNum(sectorNum) {
    // FIXME: unify with MAPS codes;
    TGeoRotation *rw = beamLineRotation ? new TGeoRotation() : 0;   
    if (beamLineRotation) rw->RotateZ(beamLineRotation);

    mTransformation = new TGeoCombiTrans(0.0, 0.0, 0.1 * beamLineOffset, rw);
  };
  ~MuMegasBarrel() {};

  void ResetVars() {
    mLayer = 0;

    mLength = mRadius = 0.0;
    mBeamLineSectionNum = mAsimuthalSectorNum = 1;

    mTransformation = 0;
  };

  MuMegasLayer *mLayer;        // layer construction details;

  Double_t mLength;            // overall length along the beam line
  UInt_t mBeamLineSectionNum;  // segmentation along the beam line (de-facto 1 or 2)

  Double_t mRadius;            // inner radius (PCB starts here)
  UInt_t mAsimuthalSectorNum;  // segmentation in phi

  TGeoMatrix *mTransformation; // 3D transformation

  ClassDef(MuMegasBarrel, 3);
};

class MuMegasGeoParData: public EicGeoParData
{
 public:
 MuMegasGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  EicGeoParData(detName, version, subVersion) {};
 ~MuMegasGeoParData() {};

 //   - layer construction;
 //   - length;
 //   - segmentation in Z;
 //   - radius;
 //   - segmentation in phi;
 //   - Z offset from 0.0 (default);
 //   - asimuthat offset from 0.0 (default);
 void AddBarrel(MuMegasLayer *layer, double length, unsigned beamLineSectionNum, 
 		double radius, unsigned sectorNum, 
		double beamLineOffset = 0.0, double beamLineRotation = 0.0) {
   mBarrels.push_back(new MuMegasBarrel(layer, length, beamLineSectionNum, 
 					radius, sectorNum, beamLineOffset, beamLineRotation));
 };
 // The same, but a generic 3D transformation given as parameter;
 void AddBarrel(MuMegasLayer *layer, double length, unsigned beamLineSectionNum, 
 		double radius, unsigned sectorNum, TGeoMatrix *transformation) {
   mBarrels.push_back(new MuMegasBarrel(layer, length, beamLineSectionNum, 
 					radius, sectorNum, transformation));
 };

  int ConstructGeometry();

  void PlaceMaterialLayer(const char *detName, const char *namePrefix, unsigned barrelID, 
			  TGeoVolume *sectorContainer, const char *material, 
			  double length, double angle,
			  double thickness, double *yOffset);

  std::vector <MuMegasBarrel*> mBarrels;    // MuMegas barrel assemblies

  ClassDef(MuMegasGeoParData,2);
};

#endif
