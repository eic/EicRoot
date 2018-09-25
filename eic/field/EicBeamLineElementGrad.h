// RMP (rpetti@bnl.gov), 2016/03/02
//
//  EIC IR Beam line element magnetic field gradient handler;
//

//#include <EicMagneticFieldGrad.h>
#include <EicMagneticFieldMap.h>

#ifndef _EIC_BEAM_LINE_ELEMENT_GRAD_
#define _EIC_BEAM_LINE_ELEMENT_GRAD_

#define _CSV_EXTENSION_ (".csv")

class Mgrid;

class EicBeamLineElementGrad: public EicMagneticFieldMap
{
 public:
  // NB: box shape parameters and 3D transformation will be known only after the
  // file is read in -> just give the file name to EicMagneticFieldGrad constructor; 
 EicBeamLineElementGrad(const char *name = 0, double x = 0.0, double y = 0.0, double z = 0.0, double boreZin = 0.0, 
			double boreZout = 0.0, double dOut = 0.0, double l = 0.0, double angle = 0.0, double b = 0.0, double gradient = 0.0): 
  EicMagneticFieldMap/*Grad*/(name), mName(name),
    mCenterX(x), mCenterY(y), mCenterZ(z), mBoreZin(boreZin), mBoreZout(boreZout), mDiaOut(dOut), mLength(l), 
    mAngle(angle), mB(b), mGradient(gradient), mScale(1.), mYoke(0), mTransformation(0) {
    // Just for debuggning purposes;
    //Initialize(); exit(0);
 };
  ~EicBeamLineElementGrad() {};

  // Ok, provide Initialize() ASCII file parser; Contains() call is the
  // "standard" one since both box shape and 3D transformation are provided;
  int Initialize();

  int GetFieldValue(const double xx[], double B[]) const;
  void SetFieldScale(const double fieldScaler);

  // Assume, that Stephen's files contain all the required information;
  bool CapableToBuildYoke()                        const { return true; };
  TString GetDetectorName()                        const { 
    return TString(BasenameWrapper(GetFileName().Data())).ReplaceAll(".txt", "");
  };
  const char *BasenameWrapper(const char *fname) const;
  int ConstructGeometry();

  TGeoVolume *GetYokeVolume() const { return mYoke; };

 private:
  TString mName;
  Double_t mCenterX;
  Double_t mCenterY;
  Double_t mCenterZ;
  Double_t mBoreZin, mBoreZout, mDiaOut;
  Double_t mLength;
  Double_t mAngle;
  Double_t mB;
  Double_t mGradient;
  Double_t mScale;   // scaling for magnetic fields for different energy running

  TGeoVolume *mYoke; //! yoke TGeo volume

  TGeoCombiTrans *mTransformation;

  ClassDef(EicBeamLineElementGrad,1);
};


#endif
