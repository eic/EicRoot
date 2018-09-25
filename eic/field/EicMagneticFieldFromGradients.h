//
// RMP (rpetti@bnl.gov), 2016/02/22
//
//  EIC magnetic field map handler;
//   alternate class to import fields just as gradients
//

//#include <map>

#include <FairField.h>

#include <PndSolenoidMap.h>

//#include <EicDetector.h>
#include <EicBeamLineElementGrad.h>
#include <EicConstantField.h>
#include <EicMagneticFieldGrad.h>

#ifndef _EIC_MAGNETIC_FIELD_FROM_GRADIENTS_
// It looks I can not alwyas use EicMagneticField::ClassName(); NB: never change this, 
// since magnetic field object will be encoded with this name in simulation.root file;
#define _EIC_MAGNETIC_FIELD_FROM_GRADIENTS_ ("EicMagneticFieldFomGradients")


class EicMagneticFieldFromGradients: public FairField 
{
 public:
  EicMagneticFieldFromGradients(const char *fileName = 0);
  ~EicMagneticFieldFromGradients() {};

  void AddFieldGradient(EicMagneticFieldGrad *grad) { 
    mMaps.push_back(grad); 
  };
  int AddBeamLineElementGrads(const char *directory, float fieldScaler = 1., int color = _DEFAULT_YOKE_COLOR_);

  enum XYZ {eX, eY, eZ};

  //
  // FairRoot methods follow;
  //

  void Init()                                             { InitializeFieldGradients(); };

  // FIXME: perhaps arrange a look-up table?;
  Double_t GetBx(Double_t x, Double_t y, Double_t z)      { return GetBxyzCore(x, y, z, eX); };
  Double_t GetBy(Double_t x, Double_t y, Double_t z)      { return GetBxyzCore(x, y, z, eY); };
  Double_t GetBz(Double_t x, Double_t y, Double_t z)      { return GetBxyzCore(x, y, z, eZ); };
  
  void GetBxyz(const Double_t point[3], Double_t* bField) {
    GetFieldSumValue(point, bField);
  };

  // NB: this call should be *after* all the AddFieldMap(new EicBeamLineElementMap()) calls;
  int CreateYokeVolumes(Bool_t Active  = kFALSE);

  int Export(const char *fileName) const { return 0; };

 private:
  Bool_t mInitialized;                     //! indicates whether Initialize() call was made or not

  //Bool_t mCreateYokeVolumes;               //! indicates whether yoke volumes should be created for beam line maps

  std::vector<EicMagneticFieldGrad*> mMaps; // magnetic field gradients

  int InitializeFieldGradients();

  // A single point of access for field quering routines;
  int GetFieldSumValue(const double xx[], double B[]);

  Double_t GetBxyzCore(Double_t x, Double_t y, Double_t z, XYZ coord) { 
    double xx[3] = {x, y, z}, B[3];

    int ret = GetFieldSumValue(xx, B);

    // In fact gets reset to 0.0 in GetFieldSumValue() anyway; ok, shoot twice;
    return (ret ? 0.0 : B[coord]); 
  };

  ClassDef(EicMagneticFieldFromGradients,1)
};

#endif
