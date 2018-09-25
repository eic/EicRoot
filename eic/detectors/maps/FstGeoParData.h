//
// AYK (ayk@bnl.gov), 2014/08/07
//
//  FST MAPS geometry description file;
//

#include <TMath.h>

#include <MapsGeoParData.h>
#include <MapsMimosaAssembly.h>

#ifndef _FST_GEO_PAR_DATA_
#define _FST_GEO_PAR_DATA_

#if _OLD_
class FstStave: public TObject {
 public:
  FstStave() { ResetVars(); };
 FstStave(unsigned mimosaChipNum, double vOffset, bool yRot, 
	  bool zRot): mMimosaChipNum(mimosaChipNum), 
    mVoffset(vOffset), mYrot(yRot), mZrot(zRot)  {};
  ~FstStave() {};
  
  void ResetVars() {
    mMimosaChipNum = 0;
    mVoffset = 0.0;
    mYrot = mZrot = false;
  };

  // It is assumed, that a stave is composed of short identical sections
  // which contain Mimosa chip and all th ecable, support, cooling, etc stuff;
  UInt_t mMimosaChipNum;             // number of Mimosa chips 
  Double_t mVoffset;                 // vertical offset wrt the disc center
  Bool_t mYrot;                      // "true" if rotated around disc Y axis
  Bool_t mZrot;                      // "true" if rotated around disc Z axis

  ClassDef(FstStave,3);
};
#endif

class FstDisc: public TObject {
 public:
  FstDisc() { ResetVars(); };
  FstDisc(MapsMimosaAssembly *chipAssembly, double minRadius, double maxRadius, 
	  double staveSpacing):
  mChipAssembly(chipAssembly), mMinRadius(minRadius), mMaxRadius(maxRadius), 
    mStaveSpacing(staveSpacing) { mTransformation = 0; };
  // This not exactly intelligent, but for now I want to duplicate same disc
  // entries with their own Z-offset and phi rotation rather than creating 
  // an intermediate class {disc*, z, phi};
  FstDisc(const FstDisc *sample, double beamLineOffset, double asimuthalOffset) { 
    *this = *sample;  
    
    TGeoRotation *rw = asimuthalOffset ? new TGeoRotation() : 0;   
    if (asimuthalOffset) rw->RotateZ(asimuthalOffset/**TMath::Pi()/180*/);

    mTransformation = new TGeoCombiTrans(0.0, 0.0, 0.1 * beamLineOffset, rw);
  };
  FstDisc(const FstDisc *sample, TGeoMatrix *transformation) { 
    *this = *sample;  
    mTransformation = transformation;
  };
  ~FstDisc() {};

  void ResetVars() {
    mChipAssembly = 0;

    mTransformation = 0;
    mStaveSpacing = mMinRadius = mMaxRadius = 0.0;
  };

  MapsMimosaAssembly *mChipAssembly; // all the details of the chip assembly 
  Double_t mMinRadius;               // min radius disc can afford to occupy
  Double_t mMaxRadius;               // max radius disc can afford to occupy
  Double_t mStaveSpacing;            // stave-to-stave distance (except for central ones)

  TGeoMatrix *mTransformation;       // 3D transformation

  ClassDef(FstDisc,6);
};

class FstGeoParData: public MapsGeoParData
{
 private:

 public:
 FstGeoParData(const char *detName = 0, int version = -1, int subVersion = 0): 
  MapsGeoParData(detName, version, subVersion) {};
  ~FstGeoParData() {};

  // Missing asimuthal rotation is the only reasonable default value here I guess;
  void AddDisc(const FstDisc *disc, double beamLineOffset, double asimuthalOffset = 0.0) {
    mDiscs.push_back(new FstDisc(disc, beamLineOffset, asimuthalOffset));
  };
  void AddDisc(const FstDisc *disc, TGeoMatrix *transformation) {
    mDiscs.push_back(new FstDisc(disc, transformation));
  };

  int ConstructGeometry();

 private:
  std::vector <FstDisc*> mDiscs; // FST (or BST) discs

  ClassDef(FstGeoParData,5);
};

#endif
