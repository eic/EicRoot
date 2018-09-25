//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Fiber layout class (shared between FEMC & CEMC);
//

#include <TObject.h>
#include <TString.h>

#ifndef _FIBER_PAR_DATA_
#define _FIBER_PAR_DATA_

class FiberTowerLayer: public TObject 
{
 public: 
 FiberTowerLayer(): mThickness(0.0), mOffset(0.0) {};
 FiberTowerLayer(double thickness, const char *media): 
  mThickness(thickness), mOffset(0.0), mMedia(media) {};
  ~FiberTowerLayer() {};

  double mThickness;
  TString mMedia;

  double mOffset;
  
  // Well, don't want to calculate this stuff every time new;
  TString mLayerName;
  TString mFiberCladdingName;
  TString mFiberCoreName;

  void SetLayerNames(const char *layer, const char *cladding, const char *core) {
    mLayerName         = TString(layer);
    mFiberCladdingName = TString(cladding);
    mFiberCoreName     = TString(core);
  };

  ClassDef(FiberTowerLayer,3);
};

class FiberParData: public TObject
{
 public:
 FiberParData(): mFiberNumPerRow(0), mFiberRowNum(0), mFiberCoreDiameter(0.0), 
    mFiberSpacingX(0.0), mFiberSpacingY(0.0), mEdgeSafetyDistance(0.0), 
    mFiberCladdingThickness(0.0), mOuterDiameter(0.0), mFiberX0offset(0.0), 
    mFiberY0offset(0.0) {};
  ~FiberParData() {};

  //
  // -> Do not mind to have these variables public?; yes, they are pure 
  //    data containers, so set()/get() methods really make no sense;
  //    same true for FiberTowerLayer class;
  //

  // Honeycomb-like structure;
  Int_t    mFiberNumPerRow;             // number of fibers in a row
  Int_t    mFiberRowNum;                // number of fiber rows
  Double_t mFiberCoreDiameter;          // fiber sensitive core diameter
  Double_t mFiberCladdingThickness;     // fiber cladding thickness
  // May either be given (as it happens in practice when one has to round
  // all numbers to 0.0001") or taken ideal from cell size;
  Double_t mFiberSpacingX;              // X-distance between neighboring fibers
  // May either be given (as it happens in practice when one has to round
  // all numbers to 0.0001") or recalculated from H-spacing as sqrt(3)/2;
  Double_t mFiberSpacingY;              // Y-distance between fiber rows
  Double_t mEdgeSafetyDistance;         // min distance from fiber center to the mesh edge  

  // Well, sort of redundant; yet have no problem to put this stuff in here;
  Double_t mOuterDiameter;              // fiber outer diamater;
  Double_t mFiberX0offset;              // 1-st fiber X-offset with respect to the tower edge
  Double_t mFiberY0offset;              // 1-st fiber Y-offset with respect to the tower edge

  std::vector<FiberTowerLayer> mLayers; // layer description (epoxy, mesh, epoxy+W)

  // Well, this one want to pack inot a method;
  void AddLayer(double thickness, const char *media) {
    mLayers.push_back(FiberTowerLayer(thickness, media));
  };

  unsigned GetLayerNum() const { return mLayers.size(); };;
  const FiberTowerLayer *GetLayer(unsigned id) { 
    return id < mLayers.size() ? &mLayers[id] : 0; 
  };

  ClassDef(FiberParData,7);
};

class TowerParData: public TObject
{
 public:
 TowerParData(): mLightGuideUpstreamWidth(0.0), mLightGuideLength(0.0), mSiliconPadThickness(0.0),
    mTowerShellLength(0.0), mSensorWidth(0.0), mSensorThickness(0.0), mSensorToSensorDistance(0.0),
    mG10Thickness(0.0), mG10Width(0.0) {};
  ~TowerParData() {};

  //
  // These parameters make sense no matter the "main" tower material is smeared 
  // or represented by fibers, etc;
  //

  Double_t mLightGuideUpstreamWidth;// light guide pyramid width at the sensor side
  Double_t mLightGuideLength;       // light guide length
  Double_t mSiliconPadThickness;    // thickness of silicon pad between light guide and sensors
  Double_t mTowerShellLength;       // overall length of tower air shell assembly (see *.C script)

  Double_t mSensorWidth;            // SiPM square sensor side length
  Double_t mSensorThickness;        // sensor thickness; should not really matter
  Double_t mSensorToSensorDistance; // distance between sensors in a 2x2 setup

  Double_t mG10Thickness;           // sensor PCB (equivalent) thickness 
  Double_t mG10Width;               // a (square) sensor PCB side width

  ClassDef(TowerParData,4);
};

#endif
