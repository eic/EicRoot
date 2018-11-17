
#include <TObject.h>

#ifndef _EIC_CAD_FILE_CONFIG_
#define _EIC_CAD_FILE_CONFIG_

#include <EicUnits.h>
#include <EicDetector.h>
#include <EicMediaHub.h>

class EicCadFileConfig: public TObject {
 public:
  enum XYZ {_X_, _Y_, _Z_, _UNDEFINED_};

  // 2018/04/08: change default units to [cm], since this simplifies STL re-export;
 EicCadFileConfig(): mScale(1.0), mUnits(eic::cm), mStlVertexMergingTolerance(0.0), mRotationAxis(_UNDEFINED_),
    mRotationAngle(0.0), mMediaHub(NULL), 
    mStlQualityCoefficient(0.001) {};
  ~EicCadFileConfig() {};

  // No sanity checks?;
  void SetUnits(double units)                         { mUnits = units; };
  // This call is deprecated, unless one really wants to re-scale the model by some 
  // arbitrary factor, with no relationship to the actual units;
  void SetScale(double scale)                         { mScale = scale; };
  // FIXME: make this tolerance parameter passing more smart, please;
  void SetStlVertexMergingTolerance(double tolerance) { mStlVertexMergingTolerance = tolerance; }
  void CreateMediaHub(char *media_name = 0)           { mMediaHub = new EicMediaHub(media_name); };
  void SetStlQualityCoefficient(double cff)           { mStlQualityCoefficient = cff; };
 
  double GetScale()                             const { return mScale; };
  double GetUnits()                             const { return mUnits; };
  EicMediaHub *GetMediaHub( void )              const { return mMediaHub; };
  // Sorry, I like short C-style names too;
  double scale()                                const { return mScale; };
  double units()                                const { return mUnits; };
  EicMediaHub *mhub( void )                     const { return mMediaHub; };

  double GetStlVertexMergingTolerance( void )   const { return mStlVertexMergingTolerance; };
  double GetStlQualityCoefficient( void )       const { return mStlQualityCoefficient; };

  //void SetRotation(unsigned axis, double angle)       { mRotationAxis = axis; mRotationAngle = angle; };
  void SetRotationY(double angle)                     { mRotationAxis = _Y_; mRotationAngle = angle; };
  unsigned GetRotationAxis( void )              const { return mRotationAxis; };
  double GetRotationAngle( void )               const { return mRotationAngle; };

 private:
  // Length units should be handled with care; per default assume [mm]; STL vertex merge 
  // tolerance is given in absolute units as well;
  double mScale, mUnits, mStlVertexMergingTolerance;

  XYZ mRotationAxis;
  double mRotationAngle;

  // If unset, the default OCC value of 0.001 (see StlAPI_Writer::StlAPI_Writer()) will 
  // be used both in EicCadFile::DumpAsStlSolid() and EicCompositeShape::LocalFillBuffer3D();
  // NB: prefer to assign this value of 0.001 right in the constructor in order not to 
  // depend on possible changes in OCC codes in future releases;
  double mStlQualityCoefficient;

  EicMediaHub *mMediaHub;

  ClassDef(EicCadFileConfig,3)  
};

#endif
