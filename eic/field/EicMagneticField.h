//
// AYK (ayk@bnl.gov), 2014/08/29
//
//  EIC magnetic field map handler;
//

#include <set>

// Make it easy (do not expect the code to be used on iphones, right?);
//#ifdef __APPLE__
//#include <libgen.h>
//#endif

#include <FairField.h>

#include <PndSolenoidMap.h>

//#include <EicDetector.h>
#include <EicMagneticFieldMap.h>
#include <EicBeamLineElementMap.h>
#include <EicBeamLineElementGrad.h>
#include <EicConstantField.h>

#ifndef _EIC_MAGNETIC_FIELD_
// It looks I can not alwyas use EicMagneticField::ClassName(); NB: never change this, 
// since magnetic field object will be encoded with this name in simulation.root file;
#define _EIC_MAGNETIC_FIELD_ ("EicMagneticField")

// Need this crappy interface for the transition period; remove once debugging 
// of EicMagneticFieldMap classes is over; NB: ignore Position() calls, etc; so
// basically assume map has no 3D transformation from local to global system;
class EicPndFieldMap: public EicMagneticFieldMap
{
 public:
 EicPndFieldMap(PndFieldMap *map = 0): mMap(map), //mBufferString(map ? map->GetFileName() : ""),
#if 1
    // basename() is clearly a hack; assume these maps are all in 'input' directory
    // without subdirectory structure; eventually will get rid of this stuff anyway;
    // NB: I need to help EicMagneticFieldMap::GetMD5Signature() somehow;
    EicMagneticFieldMap(map ? BasenameWrapper(map->GetFileName()) : 0) {};
#endif
  //EicMagneticFieldMap(map ? map->GetFileName() : 0) {};
  //EicPndFieldMap(const char *mapName, const char *mapType) {
  //mMap = new PndFieldMap(mapName, mapType);
  //};
  ~EicPndFieldMap() {};

  int Initialize() {
    if (!mMap) return -1;

    // No return codes, whatsoever, fine;
    mMap->Init();

    return EicMagneticFieldMap::Initialize();
  };

  bool Contains(const double xx[]) const {
    int ix, iy, iz;
    double  dx, dy, dz;
    
    return mMap->IsInside(xx[0], xx[1], xx[2], ix, iy, iz, dx, dy, dz);
  };

  int GetFieldValue(const double xx[], double B[]) const {
    if (!mMap) return -1;

    // Again, no return codes; great;
    mMap->GetBxyz(xx, B);
    //printf("%f %f %f\n", B[0], B[1], B[2]);

    return 0;
  };

 private:
  PndFieldMap *mMap; // PndFieldMap structure 

  ClassDef(EicPndFieldMap,1);
};

class EicMagneticField: public FairField 
{
 public:
  EicMagneticField(const char *fileName = 0);
  ~EicMagneticField() {};

  void AddFieldMap(EicMagneticFieldMap *map) { 
    mMaps.push_back(map); 
  };
  int AddBeamLineElementMaps(const char *directory, float fieldScaler = 1., int color = _DEFAULT_YOKE_COLOR_);
  int AddBeamLineElementGrads(const char *directory, float fieldScaler = 1., int color = _DEFAULT_YOKE_COLOR_);

  enum XYZ {eX, eY, eZ};

  //
  // FairRoot methods follow;
  //

  void Init()                                             { InitializeFieldMaps(); };

  // FIXME: perhaps arrange a look-up table?;
  Double_t GetBx(Double_t x, Double_t y, Double_t z)      { return GetBxyzCore(x, y, z, eX); };
  Double_t GetBy(Double_t x, Double_t y, Double_t z)      { return GetBxyzCore(x, y, z, eY); };
  Double_t GetBz(Double_t x, Double_t y, Double_t z)      { return GetBxyzCore(x, y, z, eZ); };
  
  void GetBxyz(const Double_t point[3], Double_t* bField) {
    GetFieldSumValue(point, bField);
  };

  void SuppressYokeCreation(const char *name) { mSuppressedYokes.insert(name); };

  // NB: this call should be *after* all the AddFieldMap(new EicBeamLineElementMap()) calls;
  int CreateYokeVolumes(Bool_t Active  = kFALSE);

  int Export(const char *fileName) const;

 private:
  Bool_t mInitialized;                     //! indicates whether Initialize() call was made or not

  std::set<TString> mSuppressedYokes;
  //Bool_t mCreateYokeVolumes;               //! indicates whether yoke volumes should be created for beam line maps

  std::vector<EicMagneticFieldMap*> mMaps; // magnetic field maps 

  int InitializeFieldMaps();

  // A single point of access for field quering routines;
  int GetFieldSumValue(const double xx[], double B[]);

  Double_t GetBxyzCore(Double_t x, Double_t y, Double_t z, XYZ coord) { 
    double xx[3] = {x, y, z}, B[3];

    int ret = GetFieldSumValue(xx, B);

    // In fact gets reset to 0.0 in GetFieldSumValue() anyway; ok, shoot twice;
    return (ret ? 0.0 : B[coord]); 
  };

  ClassDef(EicMagneticField,8)
};

#endif
