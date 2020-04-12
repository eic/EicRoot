//
// AYK (ayk@bnl.gov), 2014/08/07
//
//  VST MAPS geometry description file;
//

#include <MapsGeoParData.h>
#include <MapsMimosaAssembly.h>

#ifndef _VST_GEO_PAR_DATA_
#define _VST_GEO_PAR_DATA_

#define _VST_DETECTOR_NAME_ ("VST")

class VstBarrelLayer: public TObject {
 public:
  VstBarrelLayer() { ResetVars(); };
 VstBarrelLayer(MapsMimosaAssembly *chipAssembly, unsigned staveNum, 
		 unsigned chipNum, double radius, double slope, double asimuthalOffset):
  mChipAssembly(chipAssembly), mStaveNum(staveNum), mMimosaChipNum(chipNum), mRadius(radius), 
    mStaveSlope(slope), mAsimuthalOffset(asimuthalOffset) {};
  ~VstBarrelLayer() {};

  void ResetVars() {
    mChipAssembly = 0;
    mStaveNum = mMimosaChipNum = 0;
    mRadius = mStaveSlope = mAsimuthalOffset = 0.0;
  };

  UInt_t mStaveNum;                  // number of staves in this barrel layer
  Double_t mRadius;                  // layer ~radius
  Double_t mAsimuthalOffset;         // optional extra rotation around beam line 
  Double_t mStaveSlope;              // small stave rotation around barrel axis to avoid overlaps
  MapsMimosaAssembly *mChipAssembly; // all the details of the chip assembly 
  UInt_t mMimosaChipNum;             // number of Mimosa chips per stave

  ClassDef(VstBarrelLayer,2);
};

class VstGeoParData: public MapsGeoParData
{
 private:

 public:
 VstGeoParData(int version = -1, int subVersion = 0): 
  MapsGeoParData(_VST_DETECTOR_NAME_, version, subVersion), mMountingRingRadialOffset(0.0) {};
  ~VstGeoParData() {};

  void AddBarrelLayer(MapsMimosaAssembly *chipAssembly, 
		      unsigned staveNum, unsigned chipNum, double radius, 
		      double slope, double asimuthalOffset = 0.0) {
    mBarrel.push_back(new VstBarrelLayer(chipAssembly, staveNum, chipNum, radius, 
					  slope, asimuthalOffset));
  };

  unsigned GetNumberOfLayers() const { return mBarrel.size(); };

  const VstBarrelLayer *GetBarrelLayer(unsigned layerID) const { 
    return layerID <= mBarrel.size() ? mBarrel[layerID] : 0; 
  };

  Double_t mMountingRingRadialOffset;    // mounting ring radial offset wrt the layer TRD1 volume center
  
  //void Print(const char *option = 0) const;
  int ConstructGeometry(bool root = true, bool gdml = false, bool check = false);

 private:
  std::vector <VstBarrelLayer*> mBarrel; // VST barrel layers

  ClassDef(VstGeoParData,8);
};

#endif
