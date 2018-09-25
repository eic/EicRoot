//
// AYK (ayk@bnl.gov), 2014/09/03
//
//  EIC magnetic field map;
//

#include <TString.h>
#include <TGeoShape.h>
#include <TGeoTube.h>
#include <TGeoMatrix.h>

#ifndef _EIC_MAGNETIC_FIELD_MAP_
#define _EIC_MAGNETIC_FIELD_MAP_

// Go green :-)
#define _DEFAULT_YOKE_COLOR_ (kGreen)

class EicMagneticFieldMap: public TObject 
{
 public:
  EicMagneticFieldMap(const char *fileName = 0, TGeoMatrix *transformation = 0, 
		      TGeoShape *shape = 0, int color = _DEFAULT_YOKE_COLOR_);
  ~EicMagneticFieldMap() {};

  // A complementary call: ideally should define transformation right in the constructor;
  //void SetTransformation(TGeoMatrix *transformation) { mTransformation = transformation; };

  // Import field map and set up internal structures if needed; can in principle 
  // be empty call for simple map types;
  virtual int Initialize();
  bool Initialized()                                       const { return mInitialized; };

  // The actual routine indeed is different for different field map implementations;
  // do not see much sense in providing (even empty) default call here;
  virtual int GetFieldValue(const double xx[], double B[]) const = 0;

  TGeoShape *GetShape()                                    const { return mShape; };
  
  // The default implementation assumes that mShape is available;
  virtual bool Contains(const double xx[])                 const;

  const TString &GetFileName()                             const { return mFileName; };

  // Incapable per default;
  virtual bool CapableToBuildYoke()                        const { return false; };
  // This is needed only if field map pretends to be able to cook a solid object like 
  // a yoke imitator; in particular EicBeamLineElementMap just strips .csv extension 
  // from the file name;
  virtual TString GetDetectorName()                        const { return GetFileName(); };
  virtual int ConstructGeometry()                                { return 0; };

  void SetYokeColor(int color)                                   { mColor = color; };
  int  GetYokeColor()                                      const { return mColor; };

  virtual TGeoVolume *GetYokeVolume()                      const { return 0; };

  // Another clear hack in order to avoid basename() call modifying 'const char *' under Mac OS;
 protected: 
  const char *BasenameWrapper(const char *fname) const;

 private:
  // NB: format is arbitrary, but Initialize() should be able to interpret it;
  TString mFileName;           // input file name

 protected:
  UInt_t mMD5BufferSize;       // basically it is MD5_DIGEST_LENGTH constant from openssl/md5.h
  UChar_t *mMD5Signature;      //[mMD5BufferSize] input file MD5 signature
  Bool_t mInitialized;         //! indicates whether Initialize() call was made or not

  TGeoMatrix *mTransformation; // transformation to the world coordinate system

  // This is needed either to precisely determine simple constant field maps (box/tube, etc)
  // or to facilitate voxelization in case several field maps need to be imported at once
  // like for beam line elements;
  TGeoShape *mShape;           // optional bounding shape of this field map

  Int_t mColor;                // yoke color in event display

  int GetMD5Signature(unsigned char output[]);

  ClassDef(EicMagneticFieldMap,14)
};

#endif
