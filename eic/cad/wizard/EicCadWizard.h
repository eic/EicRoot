
#include <TObject.h>
#include <TString.h>

#include <EicCadWizardPlane.h>
#include <EicCadWizardCylinder.h>
#include <EicCadWizardCone.h>
#include <EicCadWizardSphere.h>
#include <EicCadWizardTorus.h>

#ifndef _EIC_CAD_WIZARD_
#define _EIC_CAD_WIZARD_

class gp_Pnt;

class TopoDS_Face;
class STEPControl_Reader;
class EicOpenCascadeShape;

class EicCadWizardFileConfig: public TObject {
 public:
 EicCadWizardFileConfig(const char *material): mStlQualityCoefficient(0.01), mBooleanSolidCreationAllowed(true), 
    mForcedStlSolidCreation(false)/*, mForcedGhostSolidCreation(false)*/ {
    if (material) mMaterial = TString(material);
  };
  ~EicCadWizardFileConfig() {};

  void AllowBooleanSolidCreation(bool what = true)    { mBooleanSolidCreationAllowed = what; };
  void ForceStlSolidCreation(bool what = true)        { mForcedStlSolidCreation = what; };
  //void ForceGhostSolidCreation(bool what = true)      { mForcedGhostSolidCreation = what; };

  void SetStlQualityCoefficient(double cff)           { mStlQualityCoefficient = cff; };
  double GetStlQualityCoefficient( void )       const { return mStlQualityCoefficient; };

  //void SetMaterial(const char *material)              { if (material) mMaterial = TString(material); }
  TString mMaterial;

#if _CHECK_
  // If unset, the default OCC value of 0.001 (see StlAPI_Writer::StlAPI_Writer()) will 
  // be used both in EicCadFile::DumpAsStlSolid() and EicCompositeShape::LocalFillBuffer3D();
  // NB: prefer to assign this value of 0.01 right in the constructor in order not to 
  // depend on possible changes in OCC codes in future releases;
#endif
  double mStlQualityCoefficient;

  bool mBooleanSolidCreationAllowed;//, mStlSolidCreationAllowed;
  bool mForcedStlSolidCreation;//, mForcedGhostSolidCreation;

 private:

  ClassDef(EicCadWizardFileConfig,7);
};

class EicCadWizardFile: public TObject {
 public:
 EicCadWizardFile(const char *fname = 0, const char *material = 0): mFileName(fname) {
    mConfig = new EicCadWizardFileConfig(material);
  };
  ~EicCadWizardFile() { if (mConfig) delete mConfig; };

  const TString &GetFileName( void ) const { return mFileName; };

  TString mFileName;

  EicCadWizardFileConfig *GetConfig( void )                { return mConfig; };
  // Sorry, I like short C-style names too;
  EicCadWizardFileConfig *config( void )                   { return mConfig; };

  EicCadWizardFileConfig *mConfig;

 private:

  ClassDef(EicCadWizardFile,1);
};

class EicCadWizard: public TObject {
 public:
 EicCadWizard(): /*: mBoundarySphereCenter(NULL), mBoundarySphereRadius(0.0)*/ mCreateEicCompositeShape(true) {};
  ~EicCadWizard() {};

  EicCadWizardFile *AddSourceFile(const char *fname, const char *material);
  //void AddSourceDirectory(const char *dname);

  int ConvertSourceFilesToRoot( void );

  std::vector<EicCadWizardFile*> mSourceFiles;

  //enum FaceType {FaceTypeAny, FaceTypeFlat, FaceTypeCurved};

  //gp_Pnt *mBoundarySphereCenter; //!
  //double mBoundarySphereRadius; //!

  int AccountRootFile(const TString &fname);
  void AddRootDirectory(const char *dname);
  int AssembleRootFiles(const char *dirname = 0, const char *detname = "DUMMY");

  std::vector<TString> mRootFiles;

  int SplitAndRescaleSourceFile(const TString &fname);//const char *fname);//const char *fname);

  void CreateRegularTGeoCompositeShape( void ) { mCreateEicCompositeShape = false; };

 private:
  bool mCreateEicCompositeShape;

  int ConvertSourceFileToRoot(const TString &fname, EicCadWizardFileConfig *config, 
			      const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts);
  int ConvertSourceFileToRoot(EicCadWizardFile *wfile, 
			      const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts);
  int ConvertSourceDirectoryToRoot(const char *dname, EicCadWizardFileConfig *config, 
				   const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts);
  int StraightforwardSplit(const STEPControl_Reader &cReader, const char *bname);
  int FaceGuidedSplit(const STEPControl_Reader &cReader, const char *bname, 
		      EicCadWizardFileConfig *config, const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts);
  int FlatSurfaceSplit(const STEPControl_Reader &cReader, const char *bname, 
		       EicCadWizardFileConfig *config, const std::vector<std::pair<EicCadWizardCut*,bool> > *tried_cuts);

  Bool_t IsElementaryFace(const TopoDS_Face &face) const;
  Bool_t IsKnownFace(const TopoDS_Face &face) const;

  //Bool_t SplitSolidByInfiniteFace(const TopoDS_Shape &solid, const TopoDS_Face &face,
  //				  EicOpenCascadeShape &cuttingShape,
  //				  std::vector<TopoDS_Shape> &commonSolids,
  //				  std::vector<TopoDS_Shape> &cutSolids);
  EicCadWizardCut *GetCut(const TopoDS_Face &face, const gp_Pnt *bcenter, double bradius);
  int CreateStlSolidRootFile(const TopoDS_Shape &solid, const char *bname, EicCadWizardFileConfig *config);
  int CreateGhostSolidRootFile(const TopoDS_Shape &solid, const char *bname, EicCadWizardFileConfig *config);
  //int CreateBooleanSolidRootFile(const TopoDS_Shape &solid, 
  //				 std::vector< std::pair<unsigned, EicOpenCascadeShape> > &facets,
  //				 const char *bname);
  int CreateBooleanSolidRootFile(const TopoDS_Shape &solid, const gp_Pnt *bcenter, double bradius,
				 const std::vector<std::pair<EicCadWizardCut*,bool> > *cuts,
				 const char *bname, EicCadWizardFileConfig *config);

  ClassDef(EicCadWizard,3);
};

#endif
