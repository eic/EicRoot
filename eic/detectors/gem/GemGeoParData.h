//
// AYK (ayk@bnl.gov), 2014/08/06
//
//  GEM geometry description file;
//

#include <TMath.h>

#include <EicGeoParData.h>

#ifndef _GEM_GEO_PAR_DATA_
#define _GEM_GEO_PAR_DATA_

// FIXME: unify with the other codes later;
#define _AIR_      ("air")
#define _ALUMINUM_ ("aluminum")
#define _COPPER_   ("copper")

class GemModule: public TObject {
 public:
  GemModule() { ResetVars(); };
  GemModule(const GemModule *sample) { *this = *sample; };
  ~GemModule() {};

  //
  // POD public access;
  //

  Double_t mActiveWindowBottomWidth;   // bottom side width of the active window
  Double_t mActiveWindowTopWidth;      // top side width of the active window
  Double_t mActiveWindowHeight;        // height of the active window

  Double_t mFrameThickness;            // frame thickness along the beam line --> G10
  Double_t mFrameBottomEdgeWidth;      // frame bottom edge width
  Double_t mFrameTopEdgeWidth;         // frame top edge width
  Double_t mFrameSideEdgeWidth;        // frame side edge width

  // FIXME: assume mylar, hmm; may want to add aluminum (copper) layer later; 
  TString  mEntranceWindowMaterial;    // entrance window material
  Double_t mEntranceWindowThickness;   // entrance window thickness            --> parameter

  Double_t mDriftFoilKaptonThickness;  // drift foil main material thickness   --> kapton
  Double_t mDriftFoilCopperThickness;  // drift foil metallization thickness   --> copper

  Double_t mGemFoilAreaFraction;       // well, there are holes in this foil
  Double_t mGemFoilKaptonThickness;    // GEM foil main material thickness     --> kapton
  Double_t mGemFoilCopperThickness;    // GEM foil metallization thickness     --> copper

  Double_t mReadoutG10Thickness;       // overall thickness of readout G10     --> G10 
  Double_t mReadoutCopperThickness;    // thickness of readout copper          --> copper 
  Double_t mReadoutKaptonThickness;    // thickness of readout kapton          --> kapton 
  TString  mReadoutSupportMaterial;    // readout support material
  Double_t mReadoutSupportThickness;   // thickness of readout support         --> parameter

  TString  mGasMixture;                // gas mixture

  // NB: the difference between mFrameThickness and these region lengths will 
  // be filled by air; NB: these numbers DO NOT include respective foil thickness;
  Double_t mEntranceRegionLength;      // length between entrance window and drift foil
  Double_t mDriftRegionLength;         // length of drift region
  // Assume triple GEM layout;
  Double_t mFirstTransferRegionLength; // length of the first transfer region
  Double_t mSecondTransferRegionLength;// length of the second transfer region
  Double_t mInductionRegionLength;     // length of induction region

 private:
  void ResetVars() {
    mActiveWindowBottomWidth = mActiveWindowTopWidth = mActiveWindowHeight = 0.0;

    mFrameThickness = mFrameBottomEdgeWidth = mFrameTopEdgeWidth = mFrameSideEdgeWidth = 0.0;

    mEntranceWindowThickness = mEntranceRegionLength = 0.0;

    mDriftFoilKaptonThickness = mDriftFoilCopperThickness = 0.0; 

    mGemFoilAreaFraction = mGemFoilKaptonThickness = mGemFoilCopperThickness = 0.0; 

    mReadoutCopperThickness = mReadoutKaptonThickness = mReadoutSupportThickness = 0.0;

    mDriftRegionLength = mFirstTransferRegionLength = mSecondTransferRegionLength = 0.0;
    mInductionRegionLength = 0.0;
  };

  ClassDef(GemModule,3);
};

class GemWheel: public TObject {
 public:
  GemWheel() { ResetVars(); };
 GemWheel(GemModule *module, unsigned moduleNum, double radius, 
	  TGeoMatrix *transformation):
  mModule(module), mModuleNum(moduleNum), mRadius(radius), 
    mTransformation(transformation) {};
 GemWheel(GemModule *module, unsigned moduleNum, double radius, double beamLineOffset, 
	  double beamLineRotation): 
  mModule(module), mModuleNum(moduleNum), mRadius(radius) {
    // FIXME: unify with MAPS codes;
    TGeoRotation *rw = beamLineRotation ? new TGeoRotation() : 0;   
    if (beamLineRotation) rw->RotateZ(beamLineRotation/**TMath::Pi()/180*/);

    mTransformation = new TGeoCombiTrans(0.0, 0.0, 0.1 * beamLineOffset, rw);
  };
  ~GemWheel() {};

  void ResetVars() {
    mModule = 0;
    mModuleNum = 0;
    mRadius = 0.0;

    mTransformation = 0;
  };

  UInt_t mModuleNum;           // number of moduless
  GemModule *mModule;          // internals of single module design

  // This is basically the distance between active window center and the 
  // geometrical center of the wheel (so beam line axis in EIC full configuration);
  Double_t mRadius;            // active window *center* installation radius
  TGeoMatrix *mTransformation; // 3D transformation

  ClassDef(GemWheel,9);
};

class GemGeoParData: public EicGeoParData
{
 public:
 GemGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  EicGeoParData(detName, version, subVersion),
  mMountingRingBeamLineThickness(0.0), mMountingRingRadialThickness(0.0),
    mWithMountingRings(false), mKaptonMaterial("GemKapton"), 
    //mMylarMaterial("GemMylar"), 
    //mRohacellMaterial("GemRohacell"), mNomexMaterial("GemNomex"),
    mG10Material("GemG10")/*, mGasMaterial("GemArCO2")*/ {};
 ~GemGeoParData() {};

 void AddWheel(GemModule *module, unsigned moduleNum, double radius, 
	       double beamLineOffset, double beamLineRotation = 0.0) {
   mWheels.push_back(new GemWheel(module, moduleNum, radius,
				  beamLineOffset, beamLineRotation));
 };
 void AddWheel(GemModule *module, unsigned moduleNum, double radius, 
	       TGeoMatrix *transformation) {
   mWheels.push_back(new GemWheel(module, moduleNum, radius, transformation));
 };

 // This should be sufficient for FLYSUB;
 void AddSingleModule(GemModule *module, double beamLineOffset, 
		      double beamLineRotation = 0.0) {
   AddWheel(module, 1, 0.0, beamLineOffset, beamLineRotation);
 };
 void AddSingleModule(GemModule *module, TGeoMatrix *transformation) {
   AddWheel(module, 1, 0.0, transformation);
 };

  void WithMountingRings(bool yesNo)       { mWithMountingRings       = yesNo; };
  bool WithMountingRings()        const    { return mWithMountingRings; };

  void SetKaptonMaterial  (const char *material) { mKaptonMaterial   = TString(material); };
  //void SetRohacellMaterial(const char *material) { mRohacellMaterial = TString(material); };
  //void SetNomexMaterial   (const char *material) { mNomexMaterial    = TString(material); };
  //void SetMylarMaterial   (const char *material) { mMylarMaterial    = TString(material); };
  void SetG10Material     (const char *material) { mG10Material      = TString(material); };
  //void SetGasMixture      (const char *material) { mGasMixture       = TString(material); };

  //const GemWheel *GetWheel(unsigned wheelID) const { 
  //return wheelID <= mWheels.size() ? mWheels[wheelID] : 0; 
  //};

  int ConstructGeometry(bool root = true, bool gdml = false, bool check = false);

  // Add sort of mounting rings on both sides of each barrel layer; assume they are 
  // the same construction for all discs;
  Double_t mMountingRingBeamLineThickness; // mounting ring thickness in beam direction 
  Double_t mMountingRingRadialThickness;   // mounting ring thickness in radial direction

  Bool_t mWithMountingRings;               // either create or not mounting rings

 private:
  // Default values are hardcoded in comnstructor; everything should be tuned once in terms
  // of radiation length thickness and not touched any longer;
  TString mKaptonMaterial;                 // may want to specify a different kapton in media.geo
  //TString mRohacellMaterial;               // ... as well as Rohacell ...
  //TString mNomexMaterial;                  // ... as well as honeycomb Nomex ...
  TString mG10Material;                    // ... as well as G10 ...
  //TString mMylarMaterial;                  // ... as well as mylar ...
  //TString mGasMaterial;                    // ... as well as Ar/CO2 gas mixture

  std::vector <GemWheel*> mWheels;         // GEM wheel assemblies
  
  void PlaceMaterialLayer(const char *detName, const char *namePrefix, unsigned wheelID, 
			  TGeoVolume *moduleContainer, const char *material, 
			  double *vert, double thickness, double *yOffset);

  ClassDef(GemGeoParData,6);
};

#endif
