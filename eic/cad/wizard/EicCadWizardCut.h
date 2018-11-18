
#include <vector>

#ifndef _EIC_CAD_WIZARD_CUT_
#define _EIC_CAD_WIZARD_CUT_

class TGeoCombiTrans;
class TopoDS_Shape;
class gp_Ax3;

// FIXME: make them configurable later;
#define _ANGULAR_TOLERANCE_ (1E-10)
#define _SPATIAL_TOLERANCE_ (1E-10)

#define SQR(x) ((x)*(x))

TGeoCombiTrans *Ax3ToCombiTrans(const char *name, const gp_Ax3 &ax3, double offset);

class EicCadWizardCut {
 public:
 EicCadWizardCut(): /*mSubtractionFlag(true),*/ mSolid(0), mDimension(0.0), mMultiplicity(1) {};
  ~EicCadWizardCut() {};

  virtual bool IsEqual(const EicCadWizardCut *cut) const = 0;
  virtual TGeoCombiTrans *BuildRootVolume(const char *vname, const char *tname) = 0;

  const TopoDS_Shape &GetSolid( void ) const { return *mSolid; };
  void IncrementMultiplicity( void )         { mMultiplicity++; };
  unsigned GetMultiplicity( void )     const { return mMultiplicity; }

  void AddCommonSolid(TopoDS_Shape *solid)   { mCommonSolids.push_back(solid); };
  void AddCutSolid(TopoDS_Shape *solid)      { mCutSolids.push_back(solid); };

  unsigned GetCommonSolidsCount( void ) const { return mCommonSolids.size(); };
  unsigned GetCutSolidsCount(void )     const { return mCutSolids.size(); };

  //bool mSubtractionFlag;

 protected:
  double mDimension;

  TopoDS_Shape *mSolid;

  unsigned mMultiplicity;

 public:
  std::vector<TopoDS_Shape*> mCutSolids, mCommonSolids;
};


#endif
