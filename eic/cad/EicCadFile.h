//
// AYK (ayk@bnl.gov), 2014/03/07; revamped in Oct'2017;
//
//  EicRoot CAD files manipulation routines; main C++ file;
//
//  NB: EicColorExtension class and other debugging codes were removed on 2017/10/04;
//

#include <TGeoMedium.h>

#include <EicUnits.h>
#include <EicDetector.h>

#ifndef _EIC_CAD_FILE_
#define _EIC_CAD_FILE_

#include <EicCadFileConfig.h>

#ifdef _OPENCASCADE_
class TopoDS_Shape;
class TopoDS_Face;
class gp_Pnt;
class gp_Ax3;

class EicOpenCascadeShape {
  friend class EicCadFile;
  friend class EicCadWizard;
  friend bool EicCadShapeEqual(const std::pair<unsigned, EicOpenCascadeShape> &lh, 
			       const std::pair<unsigned, EicOpenCascadeShape> &rh);

 public:
 EicOpenCascadeShape(): object(0), dimension(0), solid(0) {};
  ~EicOpenCascadeShape() {};

  enum ShapeType {_OCC_PLANE_, _OCC_CYLINDER_, _OCC_CONE_, _OCC_SPHERE_, _OCC_TORUS_};

 private:
  ShapeType sType;
  void *object;
  double dimension;

  TopoDS_Shape *solid;
};
#endif

#ifdef _ELMER_
class mesh_t;
#endif

#define _COLOR_DEFAULT_ (kBlue)

// Main class for CAD file import;
class EicCadFile: public EicDummyDetector {
 public:
  /// Main constructor
  ///
  /// @param Name           detector name
  /// @param geometryName   input STL (or SLP) file name
  /// @param mediaName      either media mapping file or single medium name (should then 
  ///                       be present in media.geo file)  
  EicCadFile(const char *Name, char *geometryName, char *mediaName = 0, int color = _COLOR_DEFAULT_);
  // Dummy constructor;
  EicCadFile() { ResetVariables(); };
  // Destructor;
  ~EicCadFile() { if (mConfig) delete mConfig; };

  EicCadFileConfig *GetConfig( void )                { return mConfig; };
  // Sorry, I like short C-style names too;
  EicCadFileConfig *config( void )                   { return mConfig; };

  void AllowRootSolidCreation(bool what = true)      { mRootSolidCreationAllowed      = what; };
  void AllowStlSolidCreation(bool what = true)       { mStlSolidCreationAllowed       = what; };
  void AllowStepSolidDecomposition(bool what = true) { mStepSolidDecompositionAllowed = what; };

  void SetExtraStlTranslation(double dx, double dy, double dz) {
    mExtraStlTranslation = TVector3(dx, dy, dz);
  };
  void SetKillerFlag( void ) { mKillerFlag = true; };
  void SwapXY( void ) { mSwapXY = true; };
  //void SetWireframeMode( void ) { mWireframeMode = true; };
  // FIXME: assume reflection about XZ- or XY-plane; what a crap!;
  void CreateStlMirrorCopyXZ( void ) { mCreateStlMirrorCopyXZ = true; };
  void CreateStlMirrorCopyXY( void ) { mCreateStlMirrorCopyXY = true; };

  void WizardTestbed( void );

 private:
  /*! Duplicate of the FairRoot logger singleton pointer */ 
  FairLogger *mLogger; //!

  /*! CAVE volume pointer */
  TGeoVolume *mCave; //!

  bool mKillerFlag;

  unsigned mStlVolumeCounter;
  // If 'false' EicCadFile::HandleThisSolid() will always call DumpAsStlSolid() rather 
  // than DumpAsRootSolid() at the end; NB: one can also consider to perform STL-ization
  // right from the imported STEP solid, without making attempts to decompose it; 
  bool mRootSolidCreationAllowed, mStlSolidCreationAllowed;

  // If 'false', STEP shape goes directly to STL factory;
  bool mStepSolidDecompositionAllowed;

  EicCadFileConfig *mConfig;

  void ResetVariables( void );

  /// Create the detector geometry out of STL file; 
  ///
  void ConstructGeometry();

#ifdef _OPENCASCADE_
  // Well, it is convenient to have these "global" working variables here rather
  // than pass them to handleThisSolid() and all the other functions it calls;
  // NB: these variables are marked as "//!", so seemingly ROOT sreamer should 
  // never become confused by handling .root files created with and without 
  // OpenCascade support instances of this class;
  gp_Pnt *boundarySphereCenter; //!
  double boundarySphereRadius; //!

  void ConstructDummyStepGeometry();
  void ConstructStepGeometry();
  void ConstructIgesGeometry();

  enum FaceType {FaceTypeAny, FaceTypeFlat, FaceTypeCurved};

  void HandleThisSolid(const TopoDS_Shape &solid, TGeoMedium *medium, double *color);
  void HandleThisSolidWrapper(const TopoDS_Shape &solid, TGeoMedium *medium, double *color);

  void DumpAsStlSolid(const TopoDS_Shape &solid, TGeoMedium *medium);
  TGeoCombiTrans *Ax3ToCombiTrans(char *name, const gp_Ax3 &ax3, double offset = 0.0);

  void DumpAsRootSolid(const TopoDS_Shape &solid, 
		       std::vector< std::pair<unsigned, EicOpenCascadeShape> > &facets, 
		       TGeoMedium *medium, double *color);
  Bool_t elementaryFaceType(TopoDS_Face &face, FaceType fType = FaceTypeAny);
  Bool_t splitSolidByInfiniteFace(const TopoDS_Shape &solid, const TopoDS_Face &face,
				  EicOpenCascadeShape &cuttingShape,
				  std::vector<TopoDS_Shape> &commonSolids,
				  std::vector<TopoDS_Shape> &cutSolids);
  //Bool_t elementaryFlatEdgeType(TopoDS_Edge &edge);
#endif

  int mFillColor;
  bool mSwapXY /*, mWireframeMode*/;
  // FIXME: apply to other objects later (as well as implement rotation);
  TVector3 mExtraStlTranslation;
  Bool_t mCreateStlMirrorCopyXY, mCreateStlMirrorCopyXZ;

#ifdef _ELMER_
  void ConstructElmerGeometry();

  mesh_t *mElmerMesh;
#endif

  ClassDef(EicCadFile,20)  
};

#endif
