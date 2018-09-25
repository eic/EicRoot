// AYK (ayk@bnl.gov), 2014/09/03
//
//  EIC IR Beam line element magnetic field map handler;
//

#include <EicMagneticFieldMap.h>

#ifndef _EIC_BEAM_LINE_ELEMENT_MAP_
#define _EIC_BEAM_LINE_ELEMENT_MAP_

#define _CSV_EXTENSION_ (".csv")

class Mgrid;

class EicBeamLineElementMap: public EicMagneticFieldMap
{
 public:
  // NB: box shape parameters and 3D transformation will be known only after the
  // file is read in -> just give the file name to EicMagneticFieldMap constructor; 
 EicBeamLineElementMap(const char *fileName = 0): EicMagneticFieldMap(fileName), mGrid(0),
    mLength(0.0), mAngle(0.0), mBore(0.0), mYoke(0), mScale(1.) {
    // Just for debuggning purposes;
    //Initialize(); exit(0);
 };
  ~EicBeamLineElementMap() {};

  // Ok, provide Initialize() ASCII file parser; Contains() call is the
  // "standard" one since both box shape and 3D transformation are provided;
  int Initialize();

  int GetFieldValue(const double xx[], double B[]) const;
  void SetFieldScale(const double fieldScaler);

  // Assume, that Stephen's files contain all the required information;
  bool CapableToBuildYoke()                        const { return true; };
  TString GetDetectorName()                        const { 
    return TString(BasenameWrapper(GetFileName().Data())).ReplaceAll(_CSV_EXTENSION_, "");
  };
  int ConstructGeometry();

  TGeoVolume *GetYokeVolume() const { return mYoke; };

 private:
  // Fo rnow assume this map can be transient (build it on-the-fly);
  Mgrid *mGrid;      //! structure describing the magnetic field map in HTC-style 

  // Parameters encoded in th elast few lines of Stephens' .csv files;
  Double_t mLength;  // the length of the bore
  Double_t mAngle;   // zero to the moment
  Double_t mBore;    // bore radius
  Double_t mScale;   // scaling for magnetic fields for different energy running

  TGeoVolume *mYoke; //! yoke TGeo volume

  ClassDef(EicBeamLineElementMap,4);
};

#endif
