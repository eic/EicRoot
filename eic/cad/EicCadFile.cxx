//
// AYK (ayk@bnl.gov), 2014/03/07; revamped in Oct'2017;
//
//  EicRoot CAD files manipulation routines; main C++ file; a major clean up 
//  occured on 2017/10/05; in particular commented out XCAF stuff removed 
//  (see older versions if ever want to recover it);
//
//  NB: one may want to limit the main loop in EicCadFile::HandleThisSolid()
//  to only plane and cylindrical cutting surfaces; these days one can also 
//  try out McCAD libraries (or implement more sophisticated logic similar to 
//  theirs, without breaking the existing EicCadFile code flow);
//

#include <assert.h>

// NB: certain functionality (.stl, .mphtxt import ) is available without 
// OpenCascade libraries; so make things configurable;
#ifdef _OPENCASCADE_
#include <STEPCAFControl_Reader.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSControl_TransferReader.hxx>
#include <Interface_InterfaceModel.hxx>
#include <StlAPI_Writer.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TransferBRep.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include <Geom_CylindricalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Plane.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepOffsetAPI_Sewing.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom_Curve.hxx>

#include <gp_Pln.hxx>
#include <gp_Torus.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Trsf.hxx>
#endif

#include <TColor.h>
#include <TGeoSphere.h>
#include <TGeoTorus.h>
#include <TGeoCone.h>
#include <TGeoTube.h>
#include <TGeoArb8.h>
#include <TGeoHalfSpace.h>
#include <TGeoCompositeShape.h>

#include <EicCompositeShape.h>
#include <EicCadFile.h>
#include <EicStlFactory.h>

// Optional StepCode support;
#ifdef _STEPCODE_
#include <EicStepMaterialReader.h>
#endif

// Optional Elmer include files;
#ifdef _ELMER_
#include "src/meshtype.h"
#include "plugins/egmain.h"
#endif

#define SQR(x) ((x)*(x))

// Make them configurable later;
#define _ANGULAR_TOLERANCE_ (1E-10)
#define _SPATIAL_TOLERANCE_ (1E-10)

// =======================================================================================

bool EicStlKeyCompare(const EicStlKey *lh, const EicStlKey *rh) { 
  return memcmp(lh->GetData(), rh->GetData(), rh->GetDim()*sizeof(/*float*/double)) < 0;
} // EicStlKeyCompare()

// ---------------------------------------------------------------------------------------

bool EicStlKeyEqual(const EicStlKey *lh, const EicStlKey *rh) { 
  return memcmp(lh->GetData(), rh->GetData(), rh->GetDim()*sizeof(/*float*/double)) == 0;
} // EicStlKeyEqual()

// =======================================================================================

#ifdef _OPENCASCADE_
bool EicCadShapeEqual(const std::pair<unsigned, EicOpenCascadeShape> &lh, 
		      const std::pair<unsigned, EicOpenCascadeShape> &rh)
{
  // No comparisons between faces of different topology;
  if (lh.second.sType != rh.second.sType) return false;

  // Half-space differs from other shapes since it can be defined in two equivalent ways 
  // (differ in cut/common and normal sign); so need to check on that;
  if (lh.second.sType == EicOpenCascadeShape::_OCC_PLANE_)
  {
    const gp_Pln *pll = (gp_Pln *)lh.second.object, *plr = (gp_Pln *)rh.second.object;

    if (pll->Axis().Direction().IsParallel(plr->Axis().Direction(), _ANGULAR_TOLERANCE_)) {
      if (pll->Distance(plr->Location()) > _SPATIAL_TOLERANCE_) return false;

      // Ok, do it better later; should not happen;
      assert(lh.first == rh.first);

      return true;
    } else {
      // Try anti-parallel option; gp_Dir does not have easy reflection methods, sorry;
      gp_Dir anti(-plr->Axis().Direction().X(), 
		  -plr->Axis().Direction().Y(), 
		  -plr->Axis().Direction().Z());

      if (pll->Axis().Direction().IsParallel(anti, _ANGULAR_TOLERANCE_))
      {
	if (pll->Distance(plr->Location()) > _SPATIAL_TOLERANCE_) return false;

	// Ok, do it better later; should not happen;
	assert(lh.first != rh.first);

	return true;
      } 
      else
	return false;
    } //if
  }
  else
  {
    switch (lh.second.sType) {
    case EicOpenCascadeShape::_OCC_SPHERE_:
      {
	// Allow for some tolerance later, please!;
	const gp_Sphere *spl = (gp_Sphere *)lh.second.object, *spr = (gp_Sphere *)rh.second.object;

	if (fabs(spl->Radius() - spr->Radius()) > _SPATIAL_TOLERANCE_) return false;

	{
	  double dc = sqrt(SQR(spl->Location().X() - spr->Location().X()) +
			   SQR(spl->Location().Y() - spr->Location().Y()) +
			   SQR(spl->Location().Z() - spr->Location().Z()));

	  if (dc > _SPATIAL_TOLERANCE_) return false;
	}
	break;
      }
    case EicOpenCascadeShape::_OCC_CYLINDER_:
      {
	const gp_Cylinder *cyl = (gp_Cylinder *)lh.second.object, *cyr = (gp_Cylinder *)rh.second.object;

	// Do not need to check length (assume cylinders are "infinitely" long);
	if (!cyl->Axis().Direction().IsParallel(cyr->Axis().Direction(), _ANGULAR_TOLERANCE_)) 
	  return false;

	// Radius match;
	if (fabs(cyl->Radius() - cyr->Radius()) > _SPATIAL_TOLERANCE_) return false;

	{
	  gp_XYZ diff(cyl->Location().X() - cyr->Location().X(),
		      cyl->Location().Y() - cyr->Location().Y(),
		      cyl->Location().Z() - cyr->Location().Z());

	  // Check transverse component only (it can be, that two origins are sitting 
	  // on the same axis);
	  gp_XYZ axis(cyl->Axis().Direction().X(), 
		      cyl->Axis().Direction().Y(), 
		      cyl->Axis().Direction().Z());
	  double pro = diff.Dot(axis);
	  gp_XYZ vpro = pro*axis, vnorm = diff - vpro;

	  if (vnorm.Modulus() > _SPATIAL_TOLERANCE_) return false;
	}
	break;
      }
    case EicOpenCascadeShape::_OCC_TORUS_:
      {
	const gp_Torus *tol = (gp_Torus *)lh.second.object, *tor = (gp_Torus *)rh.second.object;

	if (fabs(tol->MajorRadius() - tor->MajorRadius()) > _SPATIAL_TOLERANCE_) return false;
	if (fabs(tol->MinorRadius() - tor->MinorRadius()) > _SPATIAL_TOLERANCE_) return false;

	if (!tol->Axis().Direction().IsParallel(tor->Axis().Direction(), _ANGULAR_TOLERANCE_)) 
	  return false;

	{
	  double dc = sqrt(SQR(tol->Location().X() - tor->Location().X()) +
			   SQR(tol->Location().Y() - tor->Location().Y()) +
			   SQR(tol->Location().Z() - tor->Location().Z()));

	  if (dc > _SPATIAL_TOLERANCE_) return false;
	}
	break;
      }
    case EicOpenCascadeShape::_OCC_CONE_:
      {
	const gp_Cone *col = (gp_Cone *)lh.second.object, *cor = (gp_Cone *)rh.second.object;

	// Axis direction;
	if (!col->Axis().Direction().IsParallel(cor->Axis().Direction(), _ANGULAR_TOLERANCE_)) 
	  return false;

	// Opening angle;
	if (fabs(col->SemiAngle() - cor->SemiAngle()) > _ANGULAR_TOLERANCE_) return false;

	// Apex;
	{
	  double dc = sqrt(SQR(col->Location().X() - cor->Location().X()) +
			   SQR(col->Location().Y() - cor->Location().Y()) +
			   SQR(col->Location().Z() - cor->Location().Z()));

	  if (dc > _SPATIAL_TOLERANCE_) return false;  
	}

	break;
      }
    default: 
      assert(0);
    } //case 

    // Ok, do it better later; should not happen;
    assert(lh.first == rh.first);

    return true;
  } //if
} // EicCadShapeEqual()
#endif
// =======================================================================================

void EicCadFile::ResetVariables( void )
{
  mStlVolumeCounter = 0;
  mConfig = new EicCadFileConfig();
#ifdef _OPENCASCADE_
  boundarySphereCenter = NULL;
  boundarySphereRadius = 0.0;
#endif

  // FIXME: return back to some reasonable defaults;
  mRootSolidCreationAllowed = mStepSolidDecompositionAllowed = true;
  mStlSolidCreationAllowed = false;

#ifdef _ELMER_
  mElmerMesh = NULL;
#endif

  // Set whatever default;
  mFillColor = _COLOR_DEFAULT_;//kBlue;
  mSwapXY = mKillerFlag = false; //mWireframeMode = false;
} // EicCadFile::ResetVariables()

// ---------------------------------------------------------------------------------------

EicCadFile::EicCadFile(const char *Name, char *geometryName, char *mediaName, int color) : 
  EicDummyDetector(Name, geometryName), mCreateStlMirrorCopyXY(false), mCreateStlMirrorCopyXZ(false)
{ 
  ResetVariables();
  mFillColor = color;

  mLogger = FairLogger::GetLogger();

  //mediaHub = new EicMediaHub(mediaName);
  mConfig->CreateMediaHub(mediaName);
} // EicCadFile::EicCadFile()

// ---------------------------------------------------------------------------------------

#ifdef _OPENCASCADE_
Bool_t EicCadFile::elementaryFaceType(TopoDS_Face &face, FaceType fType)
{
  Handle(Geom_Surface) SS = BRep_Tool::Surface(face);

  if (SS->IsKind(STANDARD_TYPE(Geom_Plane)) && fType != FaceTypeCurved) return kTRUE;

  if ((SS->IsKind(STANDARD_TYPE(Geom_CylindricalSurface)) 
       || SS->IsKind(STANDARD_TYPE(Geom_ConicalSurface)) 
       || SS->IsKind(STANDARD_TYPE(Geom_ToroidalSurface)) 
       || SS->IsKind(STANDARD_TYPE(Geom_SphericalSurface))
       ) && 
      fType != FaceTypeFlat)
    return kTRUE;

  return kFALSE;
} // EicCadFile::elementaryFaceType()

// ---------------------------------------------------------------------------------------

//#include <ShapeFix_Shape.hxx>

Bool_t EicCadFile::splitSolidByInfiniteFace(const TopoDS_Shape &solid, const TopoDS_Face &face,
					    EicOpenCascadeShape &cuttingShape,
					    std::vector<TopoDS_Shape> &commonSolids,
					    std::vector<TopoDS_Shape> &cutSolids)
{
  Handle(Geom_Surface) SS = BRep_Tool::Surface(face);

  if (SS->IsKind(STANDARD_TYPE(Geom_Plane))) {
    Handle(Geom_Plane) es = Handle(Geom_Plane)::DownCast(SS);

    // Arrange an infinite plane -> convert to face -> make an infinite half-space;
    //printf("Here-1!\n");
    TopoDS_Face aFace = BRepBuilderAPI_MakeFace(es->Pln());
    gp_Dir dir = es->Pln().Axis().Direction();
    // Whatever non-zero number;
    double t = 0.1;
    //double t = -0.0001;
    // Well, put "-" sign here, because dir[] is an outer normal in ROOT halfspace volume;
    gp_Pnt x0(es->Location()), pt(x0.X() - t*dir.X(), x0.Y() - t*dir.Y(), x0.Z() - t*dir.Z());

    //printf("Here-2!\n");
    cuttingShape.solid = new TopoDS_Shape(BRepPrimAPI_MakeHalfSpace(aFace, pt).Solid());

    cuttingShape.sType = EicOpenCascadeShape::_OCC_PLANE_;
    //cuttingShape.object = new gp_Pln(es->Pln());
    cuttingShape.object = new gp_Pln(x0, dir);
    //printf("Here-3!\n");
    assert(dir.IsEqual(gp_Pln(x0, dir).Axis().Direction(), _ANGULAR_TOLERANCE_));
  }
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) {
    Handle(Geom_CylindricalSurface) es = Handle(Geom_CylindricalSurface)::DownCast(SS);
 
    gp_Cylinder gcylinder = es->Cylinder();
    double r = gcylinder.Radius();

    gp_Ax1 ax1 = gcylinder.Axis();
    gp_Dir dir = ax1.Direction();
    gp_Ax2 ax2 = gcylinder.Position().Ax2();
    gp_Pnt x0 = gcylinder.Location();
    double dist_to_sphere = es->Location().Distance(*boundarySphereCenter);
    double safe_dimension = dist_to_sphere + boundarySphereRadius;
    double t = safe_dimension;
    gp_Pnt x1(x0.X() - t*dir.X(), x0.Y()- t*dir.Y(), x0.Z()- t*dir.Z());
    ax2.SetLocation(x1);
    ax2.SetDirection(dir);
    cuttingShape.solid = 
      new TopoDS_Shape(BRepPrimAPI_MakeCylinder(ax2, r, 2 * safe_dimension).Solid());

    cuttingShape.sType = EicOpenCascadeShape::_OCC_CYLINDER_;
    cuttingShape.object = new gp_Cylinder(gp_Ax3(ax2), r);
    cuttingShape.dimension = safe_dimension;
  }
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) {
    Handle(Geom_SphericalSurface) es = Handle(Geom_SphericalSurface)::DownCast(SS);
 
    gp_Sphere sphere = es->Sphere();

    // Just need exactly this sphere, please;
    cuttingShape.sType = EicOpenCascadeShape::_OCC_SPHERE_;
    cuttingShape.object = new gp_Sphere(es->Sphere());
    cuttingShape.solid = 
      new TopoDS_Shape(BRepPrimAPI_MakeSphere(sphere.Location(), sphere.Radius()).Solid());
  }
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) {
    Handle(Geom_ToroidalSurface) es = Handle(Geom_ToroidalSurface)::DownCast(SS);
 
    gp_Torus torus = es->Torus();

    // Just need exactly this torus, please;
    cuttingShape.sType = EicOpenCascadeShape::_OCC_TORUS_;
    cuttingShape.object = new gp_Torus(es->Torus());
    cuttingShape.solid = 
      new TopoDS_Shape(BRepPrimAPI_MakeTorus(torus.Position().Ax2(), torus.MajorRadius(), 
					     torus.MinorRadius()).Solid());
  }
  else
  if (SS->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) {
    Handle(Geom_ConicalSurface) es = Handle(Geom_ConicalSurface)::DownCast(SS);

    gp_Cone cone = es->Cone();
    gp_Pnt apex = cone.Apex();
    gp_Ax1 axis = cone.Axis();
    gp_Ax2 ax2(apex, axis.Direction());

    double dist_to_sphere = apex.Distance(*boundarySphereCenter);
    // This should suffice in all cases;
    double safe_dimension = dist_to_sphere + boundarySphereRadius;
    double semiAngle = cone.SemiAngle(), r0 = 0.0, r1 = safe_dimension*tan(semiAngle);

    cuttingShape.sType = EicOpenCascadeShape::_OCC_CONE_;
    cuttingShape.object = new gp_Cone(gp_Ax3(ax2), semiAngle, r0);
    cuttingShape.dimension = safe_dimension;
    cuttingShape.solid = 
      new TopoDS_Shape(BRepPrimAPI_MakeCone(ax2, r0, r1, safe_dimension).Solid());
  }
  else
    assert(0);

  {
    //ShapeFix_Shape sfix(solid);
    //sfix.Perform();

    //printf("Here-4!\n");
    TopoDS_Shape commonHalf = BRepAlgoAPI_Common(solid, *cuttingShape.solid);
    //TopoDS_Shape commonHalf = BRepAlgoAPI_Common(sfix.Shape(), *cuttingShape.solid);
    //printf("Here-5!\n");
    for (TopExp_Explorer itq(commonHalf,TopAbs_SOLID); itq.More(); itq.Next())
      commonSolids.push_back(itq.Current());

    //printf("Here-6!\n");
    TopoDS_Shape cutHalf = BRepAlgoAPI_Cut(solid, *cuttingShape.solid);
    //printf("Here-7!\n");
    for (TopExp_Explorer itq(cutHalf,TopAbs_SOLID); itq.More(); itq.Next())
      cutSolids.push_back(itq.Current());
    
    return (commonSolids.size() + cutSolids.size() >= 2);
  }
} // EicCadFile::splitSolidByInfiniteFace()

// ---------------------------------------------------------------------------------------

void EicCadFile::DumpAsStlSolid(const TopoDS_Shape &solid, TGeoMedium *medium)
{
  char buffer[128], vname[128];
  snprintf(vname,  128-1, "%s-%06d",  dname->Name().Data(),    mStlVolumeCounter); 
  snprintf(buffer, 128-1, "/tmp/%05d-%06d-step.stl", getpid(), mStlVolumeCounter++); 

  {
    StlAPI_Writer writer;  

    writer.ASCIIMode() = true;
    //printf("GetStlQualityCoefficient(): %f\n", config()->GetStlQualityCoefficient());
    printf("Here!\n");
#if _TODAY_
    writer.SetCoefficient(config()->GetStlQualityCoefficient());
#endif
    writer.Write(solid, buffer);
    //printf("ret: %d\n", ret);
  }

  {
    // 'false': yes, here I should ignore units and scaling given in mConfig because 
    // they were taken into account when creating the STEP model; 
    EicStlFactory stl_factory(vname, buffer, mConfig, false);

    // FIXME: (0,0,0) or mExtraStlTranslation here?;
    stl_factory._ConstructGeometry(mCave, TVector3());
  }
} // EicCadFile::DumpAsStlSolid()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *EicCadFile::Ax3ToCombiTrans(char *name, const gp_Ax3 &ax3, double offset)
{
  gp_Pnt x0 = ax3.Location();
  gp_Dir xDir = ax3.XDirection(), yDir = ax3.YDirection(), zDir = ax3.Axis().Direction(); 

  double data[9] = {xDir.X(), yDir.X(), zDir.X(),
		    xDir.Y(), yDir.Y(), zDir.Y(),
		    xDir.Z(), yDir.Z(), zDir.Z()};
  TGeoRotation *grr = new TGeoRotation();
  grr->SetMatrix(data); 

  return new TGeoCombiTrans(name, 
			    x0.X() + offset * zDir.X(), 
			    x0.Y() + offset * zDir.Y(), 
			    x0.Z() + offset * zDir.Z(), grr);
} // EicCadFile::Ax3ToCombiTrans()

// ---------------------------------------------------------------------------------------

void EicCadFile::DumpAsRootSolid(const TopoDS_Shape &solid,
				 std::vector< std::pair<unsigned, EicOpenCascadeShape> > &facets,
				 TGeoMedium *medium, double *color)
{
  TString cmd(":");

  {
    static unsigned counter;
    char vname[128], tname[128];

    //printf("counter=%d\n", counter);

    snprintf(vname, 128-1, "VS%04d", counter);
    snprintf(tname, 128-1, "TS%04d", counter++);

    cmd = vname + cmd + tname;

    // Define big enough ROOT sphere;
    TGeoSphere *shell = new TGeoSphere(vname, 0.0, boundarySphereRadius + 1E-10*counter);
    TGeoCombiTrans *ts = new TGeoCombiTrans(tname, boundarySphereCenter->X(), 
					    boundarySphereCenter->Y(), 
					    boundarySphereCenter->Z(), 0);
    ts->RegisterYourself();
  }

  for(unsigned fc=0; fc<facets.size(); fc++) {
    EicOpenCascadeShape &cuttingShape = facets[fc].second;

    {
      static unsigned counter;
      char vname[128], tname[128];

      snprintf(vname, 128-1, "V%04d", counter);
      snprintf(tname, 128-1, "T%04d", counter++);

      TGeoCombiTrans *ts = 0;
      switch (cuttingShape.sType) {
      case EicOpenCascadeShape::_OCC_PLANE_:
	{
	  const gp_Pln *plane = (gp_Pln *)cuttingShape.object;

	  double x0[3] = {plane->Location().X(),         plane->Location().Y(),         plane->Location().Z()};
	  double n0[3] = {plane->Axis().Direction().X(), plane->Axis().Direction().Y(), plane->Axis().Direction().Z()};

	  printf("PL (%s) -> x0[]: %7.2f %7.2f %7.2f; n0[]: %7.2f %7.2f %7.2f\n", 
		 facets[fc].first ? "-" : "*", x0[0], x0[1], x0[2], n0[0], n0[1], n0[2]); 

	  TGeoHalfSpace *shell  = new TGeoHalfSpace(vname, x0, n0);
	  break;
	}
      case EicOpenCascadeShape::_OCC_SPHERE_:
	{
	  gp_Sphere *sphere = (gp_Sphere *)cuttingShape.object;

	  TGeoSphere *shell  = new TGeoSphere(vname, 0.0, sphere->Radius());
	  ts = new TGeoCombiTrans(tname, sphere->Location().X(), 
				  sphere->Location().Y(), 
				  sphere->Location().Z(), 0);
	  printf("SP (%s) x0[]: %f %f %f, r = %f\n", facets[fc].first ? "-" : "*", 
		 sphere->Location().X(), 
		 sphere->Location().Y(), sphere->Location().Z(), sphere->Radius());
	  break;
	}
      case EicOpenCascadeShape::_OCC_CYLINDER_:
	{
	  gp_Cylinder *cylinder = (gp_Cylinder *)cuttingShape.object;

	  TGeoTube *shell  = new TGeoTube(vname, 0.0, cylinder->Radius(), cuttingShape.dimension);

	  ts = Ax3ToCombiTrans(tname, cylinder->Position(), cuttingShape.dimension);

	  printf("CY (%s) x0[]: %f %f %f, r = %f\n", facets[fc].first ? "-" : "*", 
		 cylinder->Location().X(), 
		 cylinder->Location().Y(), cylinder->Location().Z(), cylinder->Radius());
	  break;
	}
      case EicOpenCascadeShape::_OCC_CONE_:
	{
	  gp_Cone *cone = (gp_Cone *)cuttingShape.object;

	  TGeoCone *shell  = new TGeoCone(vname, cuttingShape.dimension/2.,
					  0.0, 0.0, 
					  0.0, cuttingShape.dimension*tan(cone->SemiAngle()));

	  ts = Ax3ToCombiTrans(tname, cone->Position(), cuttingShape.dimension/2.);

	  printf("CO (%s) x0[]: %f %f %f\n", facets[fc].first ? "-" : "*", 
		 cone->Location().X(), cone->Location().Y(), cone->Location().Z());
	  break;
	}
      case EicOpenCascadeShape::_OCC_TORUS_:
	{
	  gp_Torus *torus = (gp_Torus *)cuttingShape.object;

	  TGeoTorus *shell  = new TGeoTorus(vname, torus->MajorRadius(), 
					    0.0, torus->MinorRadius(), 0.0, 360.0);

	  ts = Ax3ToCombiTrans(tname, torus->Position(), 0.0);

	  printf("TO (%s) x0[]: %f %f %f, rMa = %f, rMi = %f\n", facets[fc].first ? "-" : "*", 
		 torus->Location().X(), 
		 torus->Location().Y(), torus->Location().Z(), 
		 torus->MajorRadius(), torus->MinorRadius());
	  break;
	}
      default: 
	assert(0);
      } //switch

      if (ts) ts->RegisterYourself();

      cmd = "(" + cmd + ")" + (facets[fc].first  ? "-" : "*") + vname + (ts ? TString(":") + tname : "");
    }
  _skip_it:;
  } //for fc

  printf("%s\n", cmd.Data()); 

  TopoDS_Shape myShape = solid;
#if _OLD_
    BRepPrimAPI_MakeSphere(*boundarySphereCenter, boundarySphereRadius).Shape();

    for(unsigned fc=0; fc<facets.size(); fc++) {
    EicOpenCascadeShape &cuttingShape = facets[fc].second;

    if (facets[fc].first)
      myShape = BRepAlgoAPI_Cut   (myShape, *cuttingShape.solid);
    else
      myShape = BRepAlgoAPI_Common(myShape, *cuttingShape.solid);
  } //for fc
#endif

  {
    static unsigned ccounter;
    char cname[128];

    snprintf(cname,  128-1, "C%04d", ccounter++);

    //TGeoCompositeShape *comp = new TGeoCompositeShape(cname, cmd.Data());
    //TGeoBBox *comp = new TGeoBBox(cname, 1., 1., 2.); 
    EicCompositeShape *comp = new EicCompositeShape(cname, cmd.Data(), &myShape);
    comp->LocalFillBuffer3D(config()->GetStlQualityCoefficient());
    TGeoVolume *vcomp = new TGeoVolume(cname, comp, medium);
#if 1
    {
      static unsigned current_color_id = 1000;
      static std::vector<TColor*> colors;

      int cid = -1;
      for(unsigned c=0; c<colors.size(); c++)
      {
	TColor *clr = colors[c];

	if (clr->GetRed()   == float(color[0]) &&
	    clr->GetGreen() == float(color[1]) &&
	    clr->GetBlue()  == float(color[2]))
	{
	  cid = clr->GetNumber();
	  break;
	} //if
      } //for c

      if (cid == -1)
      {
	TColor *clr  = new TColor(++current_color_id, color[0], color[1], color[2]);
	colors.push_back(clr);
	printf(" @C@ %d, %f, %f, %f\n", current_color_id, 
	       float(color[0]), float(color[1]), float(color[2]));

	cid = current_color_id;
      } //if

      //EicColorExtension *ext = new EicColorExtension();
      //ext->r = 1.11;

      //static unsigned flag = 0;
      //unsigned id = 1115;
#if 0
      if (!flag)
      {
	TColor *color  = new TColor(id, 1., 0., 1.);
	//color->RegisterYourself();
	flag = 1;
      }
#endif
      vcomp->SetLineColor(cid);
      vcomp->SetFillColor(cid);
      //vcomp->SetUserExtension(ext);
    }
#endif
    mCave->AddNode(vcomp, 0, new TGeoCombiTrans(0, 0, 0, 0));
    vcomp->RegisterYourself();
  }
} // EicCadFile::DumpAsRootSolid()

// ---------------------------------------------------------------------------------------

#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>

void EicCadFile::HandleThisSolid(const TopoDS_Shape &solid, TGeoMedium *medium, double *color)
{
  // See whether I can split this solid into a set of more basic ones;
  // loop through all Geom_Plane faces and try to cut a solid along this plane;
  printf("EicCadFile::HandleThisSolid() ...\n");

  std::vector< std::pair<unsigned, EicOpenCascadeShape> > facets;

  unsigned hasCurvedFaces = 0;
  
  // Want them in this sequence; not dramatically efficient; optimize later;
  const Standard_Transient *shapeIds[] = {
    STANDARD_TYPE(Geom_Plane),
    STANDARD_TYPE(Geom_CylindricalSurface),
    STANDARD_TYPE(Geom_SphericalSurface),
    STANDARD_TYPE(Geom_ToroidalSurface),
    STANDARD_TYPE(Geom_ConicalSurface)};

  // For now limit myself by only plane and cylindrical surface cuts;
  //for(unsigned pass=1; pass<2; pass++) {
  for(unsigned pass=0; pass<(sizeof(shapeIds)/sizeof(shapeIds[0])); pass++) {
    for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) {
      TopoDS_Face &face = (TopoDS_Face&)itg.Current();

      //if (!elementaryFaceType(face)) hasComplexFaces = 1;

      Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
      cout << " " << SS->DynamicType() << endl;

      if (SS->IsKind((const Standard_Type*)shapeIds[pass])) {
	EicOpenCascadeShape cuttingShape;
	std::vector<TopoDS_Shape> commonSolids, cutSolids;
	
	assert(elementaryFaceType(face));
	
	printf("Splitting ...\n");
	if (splitSolidByInfiniteFace(solid, face, cuttingShape, 
				     commonSolids, cutSolids)) {
	  printf("  Will now handle %d (common) & %d (cut) sub-solids ...\n", 
		 commonSolids.size(), cutSolids.size());
#if 1//_TODAY_
	  for(unsigned sh=0; sh<commonSolids.size(); sh++) 
	    HandleThisSolid(commonSolids[sh], medium, color);
	  
	  for(unsigned sh=0; sh<cutSolids.size(); sh++) 
	    HandleThisSolid(cutSolids[sh], medium, color);
	  
	  return;
#endif
	} else {
	  assert(commonSolids.size() + cutSolids.size() == 1);

#if 1//_TODAY_
	  // Solids, which have plane faces only, will go directly to STL as well;
	  // may want to optimize at least for BOX shapes later;
	  if (!SS->IsKind(STANDARD_TYPE(Geom_Plane))) hasCurvedFaces = 1;
	  
	  // Loop through already defined shapes and check that there is no 
	  // such one is already present; NB: I guess ordering here does not 
	  // make sense -> vector suffices;
	  {
	    unsigned exists = 0;
	    std::pair<unsigned, EicOpenCascadeShape> qPair(cutSolids.size(), cuttingShape);

	    for(unsigned fc=0; fc<facets.size(); fc++)
	      if (EicCadShapeEqual(qPair, facets[fc])) {
		exists = 1;
		break;
	      } //for fc..if

	    if (!exists) facets.push_back(qPair);
	  }
#endif
	} //if
      } //if
    } //for itg
  } //for pass

#if 1//_TODAY_
  // Ok, solid can not be split any longer; see whether it consists of "basic" faces
  // only (plane, sphere, cone, cylinder); in this case pass it through the logical 
  // decomposition filter (and represent *exactly* in ROOT equivalents); otherwise 
  // pass it through STL modeler (and represent *approximately* as a collection of 
  // tetrahedra;
  {
    unsigned hasComplexFaces = 0;

    for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) {
      TopoDS_Face &face = (TopoDS_Face&)itg.Current();

      //if (!elementaryFaceType(face)) 
      {
	//printf("    One more complex face: \n");
#if 1//_DEBUG_
	Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
	cout << " " << SS->DynamicType() << endl;
	if (SS->IsKind(STANDARD_TYPE(Geom_Plane))) {

	} //if

	for (TopExp_Explorer ith(itg.Current(),TopAbs_EDGE); ith.More(); ith.Next()) {
	  TopoDS_Edge &edge = (TopoDS_Edge&)ith.Current();
	  double first, last;
	  Handle(Geom_Curve) SC = BRep_Tool::Curve(edge, first, last);

	  //printf("    One more edge: ");
	  cout << " --> " << SC->DynamicType() << endl;
	  if (SC->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) {
	    Handle(Geom_BSplineCurve) spline = Handle(Geom_BSplineCurve)::DownCast(SC);

	    gp_Pnt start = spline->StartPoint(), end = spline->EndPoint();

	    printf("%d %d\n", spline->NbKnots(), spline->NbPoles());
	    printf("%d %d\n", spline->Degree(), spline->MaxDegree());
	    printf("%f %f %f -> %f %f %f\n", start.X(), start.Y(), start.Z(), 
		   end.X(), end.Y(), end.Z());
	  } //if
#if 1
	  if (SC->IsKind(STANDARD_TYPE(Geom_Circle))) {
	    Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(SC);
	    gp_Pnt center = circle->Location();

	    printf("%f %f %f; %f\n", center.X(), center.Y(), center.Z(), circle->Radius()); 
	  } //if
#endif
	} //for ith
#endif
	if (!elementaryFaceType(face)) hasComplexFaces = 1;
      } //if
    } //for itg

    if (hasComplexFaces /*|| !hasCurvedFaces*/) {
      printf("Complex face found -> dump as STL!\n");
      assert(mStlSolidCreationAllowed);
      DumpAsStlSolid(solid, medium);
      return;
    } //if
  }

  // Ok, now the tricky part comes; represent this solid as an overlap of
  // simple basic shapes;
  if (mRootSolidCreationAllowed) {
    printf("Can decompose into basic ROOT shapes!\n");
    DumpAsRootSolid(solid, facets, medium, color);
  } else {
    assert(mStlSolidCreationAllowed);
    DumpAsStlSolid(solid, medium);
  } //if
#endif
} // EicCadFile::HandleThisSolid()

// ---------------------------------------------------------------------------------------

void EicCadFile::HandleThisSolidWrapper(const TopoDS_Shape &solid, TGeoMedium *medium, double *color)
{
  // This should be determined once (for the "main" shape) -> optimize later;
  double xMin, yMin, zMin, xMax, yMax, zMax;
  Bnd_Box Boundary;
  BRepBndLib::Add(solid, Boundary);

  Boundary.Get(xMin, yMin, zMin, xMax, yMax, zMax);

  if (boundarySphereCenter) delete boundarySphereCenter;

  // Be lazy; determine bounding sphere;
  boundarySphereCenter = new gp_Pnt((xMin+xMax)/2., (yMin+yMax)/2., (zMin+zMax)/2.);
  boundarySphereRadius = sqrt(SQR(xMax-xMin) + SQR(yMax-yMin) + SQR(zMax-zMin))/2.;

  // FIXME: was the beginning of this routine necessary in case shape goes directly to STL factory?;
  if (mStepSolidDecompositionAllowed)
    HandleThisSolid(solid, medium, color);
  else
    DumpAsStlSolid(solid, medium);
} // EicCadFile::HandleThisSolidWrapper()

// ---------------------------------------------------------------------------------------

//
// FIXME: it looks like units will be default here, so [mm] (?);
//

void EicCadFile::ConstructDummyStepGeometry()
{
  gp_Pnt p0(-1,-1,-2), p1(1,1,2), p2(1, 0, 0), p3(0,0,-2);
  //gp_Pnt p0(-1,-1,2), p1(1,1,5), p2(1, 0, 0), p3(0,0,-2);
  TopoDS_Solid rect1   = BRepPrimAPI_MakeBox(p0, p1).Solid();
  TopoDS_Solid sphere = BRepPrimAPI_MakeSphere(p3, 0.7).Solid();
  
  gp_Pnt p000(0,0,0), p111(3,3,3);
  TopoDS_Solid rect2 = BRepPrimAPI_MakeBox(p000, p111).Solid();
  
  //gp_Pnt q000(4,4,4), q111(5,5,5);
  //gp_Pnt q000(-2,-2,2), q111(2,2,6);
  gp_Pnt q000(-2,-1,2), q111(1,1,6);
  TopoDS_Solid rect3 = BRepPrimAPI_MakeBox(q000, q111).Solid();
  
  gp_Pnt r000(-1,-1,-4), r111(1,1,-3);
  TopoDS_Solid rect4 = BRepPrimAPI_MakeBox(r000, r111).Solid();
  
  //gp_Dir dir(0,1/sqrt(2.),1/sqrt(2.));
  gp_Dir dir(0,1,0);
  gp_Pnt x0(0,-3,-4);
  gp_Ax2 ax2;
  ax2.SetLocation(x0);
  ax2.SetDirection(dir);
  TopoDS_Solid cylinder = BRepPrimAPI_MakeCylinder(ax2, 3., 10.).Solid();
  
  TopoDS_Solid torus = BRepPrimAPI_MakeTorus(ax2, 3., 2.).Solid();
  
  TopoDS_Solid cone = BRepPrimAPI_MakeCone(ax2, 0.0, 5.0, 10.0).Solid();
  
  //TopoDS_Shape solid = BRepAlgoAPI_Fuse(rect1, rect2);
  //TopoDS_Shape solid = BRepAlgoAPI_Fuse(rect1, cylinder);
  //const TopoDS_Shape solid = BRepAlgoAPI_Fuse(rect1, cone);
  TopoDS_Shape solid = BRepAlgoAPI_Fuse(rect1, sphere);
  
  {
    double color[3] = {0., 0., 0.};
    
    HandleThisSolidWrapper(solid, mConfig->mhub()->fSingleMedium, color); 
    //HandleThisSolidWrapper(rect1, mediaHub->fSingleMedium, color);  
    //HandleThisSolidWrapper(rect2, mediaHub->fSingleMedium); 
    //HandleThisSolidWrapper(rect4, mediaHub->fSingleMedium); 
    //HandleThisSolidWrapper(sphere, mediaHub->fSingleMedium, color); 
    //HandleThisSolidWrapper(cylinder, mediaHub->fSingleMedium, color); 
    //HandleThisSolidWrapper(torus, mediaHub->fSingleMedium, color); 
    //HandleThisSolidWrapper(cone, mediaHub->fSingleMedium, color); 
  } 
} // EicCadFile::ConstructDummyStepGeometry()

// ---------------------------------------------------------------------------------------

void EicCadFile::ConstructStepGeometry()
{
#ifdef _STEPCODE_
  EicStepMaterialReader stReader;
#endif

  // Well, if medium name (then the same for all volumes in this STEP file) was 
  // given, use it; otherwise do a fairly stupid thing: read .stp file in using
  // StepCode, extract material->volume information from it and later establish 
  // a relationship between StepCode internals and OpenCascade internals in 
  // order to couple OpenCascade solids with StepCode-imported material names;
  if (!mConfig->mhub()->fSingleMedium) {
#ifdef _STEPCODE_
    if (stReader.ReadFile(GetGeometryFileName()))
      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to parse '%s' "
		     "using StepCode library! \033[0m", GetGeometryFileName().Data());
#else
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m STEP file '%s': no media name "
		   "given and StepCode support is not compiled in! \033[0m", 
		   GetGeometryFileName().Data());
#endif
  } //if

  // Well, consider to use plain STEPControl_Reader for now; eventually should be able 
  // to get materials out of the file using STEPCAFControl_Reader; see EicCadFile.cxx 
  // versions before 2017/10/05; for now just use StepCode bypass;
  STEPControl_Reader cReader;
  // Read file; need to check import result later, here and below;
  IFSelect_ReturnStatus cstatus = cReader.ReadFile(GetGeometryFileName()); 
  // FIXME: may want to loop through all roots; should be easy fix?;
  //assert(cReader.NbRootsForTransfer() == 1);
  cReader.TransferRoots();//(1);
  printf("%d root(s) in STEP file; %d shape(s)\n", cReader.NbRootsForTransfer(), cReader.NbShapes());
  //cReader.TransferRoot(1);

  // A tricky part: find STEP entity number in the original file and use this 
  // tag in order to match against StepCode numbering scheme;
  Handle(Transfer_TransientProcess) TP = cReader.WS()->TransferReader()->TransientProcess();
  Handle(Interface_InterfaceModel) Model =cReader.Model();
  printf("OpenCascade Model->NbEntities(): %d\n", Model->NbEntities());

  {
    gp_Trsf scaling;
    scaling.SetScaleFactor(config()->scale() * config()->units());
    BRepBuilderAPI_Transform sc(scaling);

    // Loop through all shapes one by one; NB: it can happen, that this sequence 
    // of actions does not work for ANY .stp file; generalize later as needed;
    for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
      TopoDS_Shape shape = cReader.Shape(sh+1);

      // Loop through all solids in this shape; 
      for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next()) {
	TGeoMedium *medium = mConfig->mhub()->fSingleMedium;
	
	if (!medium) {
	  // Try to get medium tag directly from STEP file; FIXME: requires further debugging; 
#ifdef _STEPCODE_
	  // Figure out STEP file entity number; assume entity numbering is the same 
	  // in OpenCascade and StepCode -> bridge to that internal tree and get access
	  // to material properties; somebody smart please make original OpenCascade 
	  // do this, I'm too lazy/stupid, sorry; optimize later ...;
	  for (unsigned iqq=0; iqq<Model->NbEntities(); iqq++) {
	    Handle(Standard_Transient) enti = Model->Value(iqq+1);
	    
	    // This may not always work -> comment out completely and loop through all 
	    // entities in this case; once again, need to optimize this loop anyway;
	    if(enti->IsKind(STANDARD_TYPE(StepRepr_NextAssemblyUsageOccurrence))) {
	      Standard_Integer index = TP->MapIndex(enti);
	      
	      if (index > 0) {
		Handle(Transfer_Binder) binder = TP->MapItem (index);
		TopoDS_Shape S = TransferBRep::ShapeResult(binder);
		if (S.IsEqual(itf.Current())) {
		  printf("   .. found (%4d)!\n", iqq); 
		  
		  {
		    const char *mediumName = stReader.GetMediumName(iqq);
		    if (!mediumName)
		      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Solid (%d) has no material assignment"
		    		     "in step file (and no medium given by hand)! \033[0m", iqq);
		    
		    medium = mConfig->mhub()->GetMedium(mediumName);
		    // Otherwise stReader.GetMedium() would have triggered mLogger->Fatal();
		    //assert(medium);
		    if (!medium)
		      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m No such medium - %s - "
				     "check media.geo file! \033[0m", mediumName);
		  }
		}
	      }
	    }
	  } //for iqq
#else
	    // FIXME: isn't this message redundant (see the very beginning of this routine)?;
	    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m STEP file '%s': no medium name "
			   "given and StepCode support is not compiled in! \033[0m", 
			   GetGeometryFileName().Data());
#endif
	} //if

	double color[3] = {0., 0., 0.};
	// Try to get color directly from STEP file; FIXME: requires further debugging; 
#ifdef _STEPCODE_
	{
	  // Pick up the very first face; assume they are all of the same color;
	  TopExp_Explorer itg(itf.Current(),TopAbs_FACE);
	  
	  // Figure out STEP file entity number; assume entity numbering is the same 
	  // in OpenCascade and StepCode -> bridge to that internal tree and get access
	  // to material properties; somebody smart please make original OpenCascade 
	  // do this, I'm too lazy/stupid, sorry; optimize later ...;
	  for (unsigned iqq=0; iqq<Model->NbEntities(); iqq++) {
	    Handle(Standard_Transient) enti = Model->Value(iqq+1);
	    
	    // This may not always work -> comment out completely and loop through all 
	    // entities in this case; once again, need to optimize this loop anyway;
	    //if(enti->IsKind(STANDARD_TYPE(StepRepr_NextAssemblyUsageOccurrence))) 
	    {
	      //cout << enti->DynamicType() << endl;
	      
	      Standard_Integer index = TP->MapIndex(enti);
	      
	      if (index > 0) {
		Handle(Transfer_Binder) binder = TP->MapItem (index);
		TopoDS_Shape S = TransferBRep::ShapeResult(binder);
		if (S.IsEqual(itg.Current())) {
		  //printf("   .. found (%4d)!\n", iqq); exit(0);
		  
		  if (!stReader.GetColor(iqq, color))
		    {
		      //printf("@@@ Success: %f %f %f\n", arr[0], arr[1], arr[2]);
		      
		    } //if
		  
		  {
#if 0
		    const char *mediumName = stReader.GetMediumName(iqq);
		    if (!mediumName)
		      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Solid (%d) has no material assignment"
		    		     "in step file (and no medium given by hand)! \033[0m", iqq);
		    
		    medium = mediaHub->GetMedium(mediumName);
		    // Otherwise stReader.GetMedium() would have triggered mLogger->Fatal();
		    //assert(medium);
		    if (!medium)
		      mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m No such medium - %s - "
				     "check media.geo file! \033[0m", mediumName);
#endif
		  }
		}
	      }
	    }
	  } //for iqq
	} //if
#endif

	// Rescale the object;
	sc.Perform(itf.Current());
	{
	  static unsigned counter;

	  //counter++;
	  printf("counter: %d\n", counter);
	  //if (counter == 188)
	  //if (/*counter == 10 || counter == 12 ||*/ counter == 5 || //counter == 8 ||
	  //  counter == 1 || counter == 4 || counter == 9 || counter == 11)
	  //if (counter == 12)
	  //  counter == 3 || counter == 5 || counter == 6 || counter == 8)
	    //if (counter == 3 || counter == 5 || counter == 6 || counter == 8)
	    //if (counter != 2 && counter != 9)
	    //if (counter == 1 || counter == 4 || counter == 7 || counter == 6 || counter == 10 || counter == 11 || counter == 12)
	    //if (counter == 1 || (counter >= 7 && counter <= 8) || (counter >= 10 && counter <= 12)) 
	  HandleThisSolidWrapper(sc.ModifiedShape(itf.Current()), medium, color);

	  counter++;
	}
      } //for itf
    } //for sh
  }
} // EicCadFile::ConstructStepGeometry()

// ---------------------------------------------------------------------------------------

#include <STEPControl_Writer.hxx>
//#include <STEPControl_StepModelType.hxx>

void EicCadFile::WizardTestbed()
{
  //EicGeoParData *geo = new EicGeoParData("DUMMY", 0, 0);
  EicGeoParData *geo = new EicGeoParData(dname->NAME().Data(), 0, 0);
  char fname[1024];
  snprintf(fname, 1024-1, "%s.root", dname->name().Data());
  geo->SetFileName(fname);

  mConfig->mhub()->Init(); mCave = geo->GetTopVolume();
  //mCave = gGeoManager->GetTopVolume(); assert(mCave);

  STEPControl_Reader cReader;
  IFSelect_ReturnStatus cstatus = cReader.ReadFile(GetGeometryFileName()); 
  // FIXME: may want to loop through all roots; should be easy fix?;
  //assert(cReader.NbRootsForTransfer() == 1);
  cReader.TransferRoots();//(1);
  printf("%d root(s) in STEP file; %d shape(s)\n", cReader.NbRootsForTransfer(), cReader.NbShapes());
  //cReader.TransferRoot(1);

  // A tricky part: find STEP entity number in the original file and use this 
  // tag in order to match against StepCode numbering scheme;
  Handle(Transfer_TransientProcess) TP = cReader.WS()->TransferReader()->TransientProcess();
  Handle(Interface_InterfaceModel) Model = cReader.Model();
  printf("OpenCascade Model->NbEntities(): %d\n", Model->NbEntities());

  {
    gp_Trsf scaling;
    if (config()->GetRotationAxis() != EicCadFileConfig::_UNDEFINED_) {
      gp_Pnt x0(0.0, 0.0, 0.0);
      // FIXME: do it better later;
      assert(config()->GetRotationAxis() == EicCadFileConfig::_Y_);
      gp_Dir n0(0.0, 1.0, 0.0);
      gp_Ax1 axis(x0, n0);

      scaling.SetRotation(axis, config()->GetRotationAngle() * TMath::DegToRad());
    } //if
    // Yes, first rotate, then re-scale (otherwise scaling takes no effect);
    scaling.SetScaleFactor(config()->scale() * config()->units());
    BRepBuilderAPI_Transform sc(scaling);

    // Loop through all shapes one by one; NB: it can happen, that this sequence 
    // of actions does not work for ANY .stp file; generalize later as needed;
    for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
      TopoDS_Shape shape = cReader.Shape(sh+1);

      // Loop through all solids in this shape; 
      for (TopExp_Explorer itf(shape,TopAbs_SOLID); itf.More(); itf.Next()) {
	TGeoMedium *medium = mConfig->mhub()->fSingleMedium;
	
	double color[3] = {0., 0., 0.};

	// Rotate and rescale the object;
	sc.Perform(itf.Current());
#if _TODAY_
	{
	  static unsigned counter;
	  printf("counter: %d\n", counter);
	  const TopoDS_Shape &solid = sc.ModifiedShape(itf.Current());
	  
	  STEPControl_Writer cWriter;
	  cWriter.Transfer(solid, STEPControl_ManifoldSolidBrep);
	  char name[1024];
	  snprintf(name, 1024-1, "%s-%05d.stp", GetGeometryFileName().Data(), counter);
	  cWriter.Write(name);
	  
	  counter++;
	} 
#endif	  
#if _TODAY_
	HandleThisSolidWrapper(sc.ModifiedShape(itf.Current()), medium, color);
#endif
#if _TODAY_
	{
	  const TopoDS_Shape &solid = sc.ModifiedShape(itf.Current());
	  std::vector<TopoDS_Shape> parts;
	  gp_Pnt x0(0.0, 0.0, 0.0);
	  gp_Dir n0(0.0, 1.0, 0.0);
	  gp_Pln pl(x0, n0);
	  TopoDS_Face face = BRepBuilderAPI_MakeFace(pl);
	  double t = 0.1;
	  gp_Pnt pt(x0.X() - t*n0.X(), x0.Y() - t*n0.Y(), x0.Z() - t*n0.Z());
	  TopoDS_Shape *cut = new TopoDS_Shape(BRepPrimAPI_MakeHalfSpace(face, pt).Solid());
	  
	  TopoDS_Shape commonHalf = BRepAlgoAPI_Common(solid, *cut);
	  for (TopExp_Explorer itq(commonHalf,TopAbs_SOLID); itq.More(); itq.Next())
	    parts.push_back(itq.Current());
	  
	  TopoDS_Shape cutHalf = BRepAlgoAPI_Cut(solid, *cut);
	  for (TopExp_Explorer itq(cutHalf,TopAbs_SOLID); itq.More(); itq.Next())
	    parts.push_back(itq.Current());
	  {
	    unsigned rCounter = 0;
	    printf("--> %d\n", parts.size());
	    for(unsigned pt=0; pt<parts.size(); pt++) {
	      STEPControl_Writer cWriter;
	      cWriter.Transfer(parts[pt], STEPControl_ManifoldSolidBrep);
	      char name[1024];
	      snprintf(name, 1024-1, "test-%05d.stp", rCounter);
	      cWriter.Write(name);
	      
	      rCounter++;
	    } //for pt
	    //for(unsigned pt=0; pt<parts.size(); pt++) {
	    //DumpAsStlSolid(parts[pt], medium);
	    //} //for pt
	  }
	}
#endif
#if 1//_TODAY_
	{
	  //const TopoDS_Shape &solid = sc.ModifiedShape(itf.Current());
	  const TopoDS_Shape &solid = itf.Current();

	  {
	    double xMin, yMin, zMin, xMax, yMax, zMax;
	    Bnd_Box Boundary;
	    BRepBndLib::Add(solid, Boundary);
	    
	    Boundary.Get(xMin, yMin, zMin, xMax, yMax, zMax);
	    
	    if (boundarySphereCenter) delete boundarySphereCenter;
	    
	    // Be lazy; determine bounding sphere;
	    boundarySphereCenter = new gp_Pnt((xMin+xMax)/2., (yMin+yMax)/2., (zMin+zMax)/2.);
	    boundarySphereRadius = sqrt(SQR(xMax-xMin) + SQR(yMax-yMin) + SQR(zMax-zMin))/2.;
	  }

	  unsigned faceCounter = 0;
	  int rfCounter = -1;
	  for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) 
	    faceCounter++;
	  printf("%3d faces total!\n", faceCounter);
	  for (TopExp_Explorer itg(solid,TopAbs_FACE); itg.More(); itg.Next()) {
	    TopoDS_Face &face = (TopoDS_Face&)itg.Current();

	    rfCounter++;
	    //assert(elementaryFaceType(face));

	    Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
	    cout << " " << SS->DynamicType() << endl; //continue;
	    if (!elementaryFaceType(face)) continue;
	    //if (SS->IsKind(STANDARD_TYPE(Geom_Plane))) continue;
	    //if (SS->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))) continue;
	    //if (SS->IsKind(STANDARD_TYPE(Geom_ConicalSurface))) continue;
	    //if (SS->IsKind(STANDARD_TYPE(Geom_SphericalSurface))) continue;
	    //if (SS->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))) continue;

	    {
	      EicOpenCascadeShape cuttingShape;
	      std::vector<TopoDS_Shape> commonSolids, cutSolids;
	      std::vector<TopoDS_Shape> *arr[2] = {&commonSolids, &cutSolids};
	      unsigned pCounter[2], fCounter[2] = {0, 0};
	      
	      splitSolidByInfiniteFace(solid, face, cuttingShape, commonSolids, cutSolids);
	      //printf("%3d %3d\n", commonSolids.size(), cutSolids.size());

	      for(unsigned gr=0; gr<2; gr++) {
		std::vector<TopoDS_Shape> *group = arr[gr];

		pCounter[gr] = group->size();
		for(unsigned pt=0; pt<group->size(); pt++) {
		  const TopoDS_Shape &shape = (*group)[pt];

		  for (TopExp_Explorer itx(shape,TopAbs_FACE); itx.More(); itx.Next()) 
		    fCounter[gr]++;
		} //for pt

		//printf("  gr#%d -> %3d solids & %4d faces\n", gr, group->size(), fCounter[gr]);
	      } //for gr

	      if (pCounter[0] + pCounter[1] >= 2) {
		for(unsigned gr=0; gr<2; gr++) 
		  printf("  gr#%d -> %3d solids & %4d faces\n", gr, pCounter[gr], fCounter[gr]);
		printf("    --> %4d faces total!\n", fCounter[0] + fCounter[1]); 
	      } //if
#if 1//_OFF_
	      if (pCounter[0] + pCounter[1] >= 2) {
		unsigned rCounter = 0;

		for(unsigned gr=0; gr<2; gr++) {
		  std::vector<TopoDS_Shape> *group = arr[gr];

		  for(unsigned pt=0; pt<group->size(); pt++) {
		    const TopoDS_Shape &shape = (*group)[pt];

		    STEPControl_Writer cWriter;
		    //TopoDS_Face body = BRepBuilderAPI_MakeSolid(TopoDS_Shell(shape));

		    cWriter.Transfer(shape, STEPControl_ManifoldSolidBrep);
		    char name[1024];
		    snprintf(name, 1024-1, "test-%03d-%05d.stp", rfCounter, rCounter);
		    cWriter.Write(name);
		    
		    rCounter++;
		  } //for pt
		} //for gr
	      } //if
#endif
	    }

	    
	  } //for itg
	}
#endif
      } //for itf
    } //for sh
  }

  //geo->GetColorTable()->AddPatternMatch       ("00", kGray);
  //geo->GetTransparencyTable()->AddPatternMatch("00", 0);
  geo->GetColorTable()->AddPatternMatch       ("Dummy-666", kRed);
  geo->GetColorTable()->AddPatternMatch       ("Dummy-777", kBlue);
  geo->GetColorTable()->AddPatternMatch       ("Dummy-888", kGray);
  geo->GetColorTable()->AddPatternMatch       ("Dummy-999", kGreen);
  geo->GetTransparencyTable()->AddPatternMatch("00", 0);

  geo->FinalizeOutput();
} // EicCadFile::WizardTestbed()

// ---------------------------------------------------------------------------------------

//
// -> FIXME: merge with ConstructStepGeometry() later; NB: admittedly this code is very raw
//    and can handle only very "clean" shapes only;
//
// -> FIXME: need to set units!;
//

void EicCadFile::ConstructIgesGeometry()
{
  // Keep it easy for now; FIXME: re-implement XDE data parser via IGESCAFControl_Reader;
  IGESControl_Reader cReader;

  // Read file; FIXME: need to check import result later, here and below;
  IFSelect_ReturnStatus cstatus = cReader.ReadFile(GetGeometryFileName()); 
  //assert(cReader.NbRootsForTransfer() == 1);
  cReader.TransferRoots();
  //printf("OpenCascade cReader.NbShapes(): %d\n", cReader.NbShapes()); 

  {
    BRepBuilderAPI_Sewing *Sew = new BRepBuilderAPI_Sewing();

    // Loop through all shapes one by one; NB: it can happen, that this sequence 
    // of actions does not work for ANY .igs file; generalize later as needed;
    for(unsigned sh=0; sh<cReader.NbShapes(); sh++) {
      const TopoDS_Shape shape = cReader.Shape(sh+1);

      // Prefer to keep these debugging printouts here and below;
#if _DEBUG_
      for (TopExp_Explorer itf(shape,TopAbs_EDGE); itf.More(); itf.Next()) {
	double first, last;
	TopoDS_Edge &edge = (TopoDS_Edge&)itf.Current();
	Handle(Geom_Curve) SC = BRep_Tool::Curve(edge, first, last);

	cout << SC->DynamicType() << endl;
      } //for itf
#endif

      // Loop through all faces;
      for (TopExp_Explorer itf(shape,TopAbs_FACE); itf.More(); itf.Next()) {
	static unsigned counter;
	//printf("adding face #%04d ...\n", counter++);
	Sew->Add(itf.Current());

#if _DEBUG_
	TopoDS_Face &face = (TopoDS_Face&)itf.Current();
	if (!elementaryFaceType(face)) {
	  printf("    One more complex face: \n");
	  Handle(Geom_Surface) SS = BRep_Tool::Surface(face);
	  cout << " " << SS->DynamicType() << endl;
#if _DEBUG_
	  for (TopExp_Explorer ith(itg.Current(),TopAbs_EDGE); ith.More(); ith.Next()) {
	    TopoDS_Edge &edge = (TopoDS_Edge&)ith.Current();
	    double first, last;
	    Handle(Geom_Curve) SC = BRep_Tool::Curve(edge, first, last);
	    
	    //printf("    One more edge: ");
	    cout << " --> " << SC->DynamicType() << endl;
	  } //for ith
#endif
	} //if
#endif
      } //for itf

    } //for sh

    {
      Sew->Perform(); Sew->Dump(); 
      TopoDS_Shape result = Sew->SewedShape();
      for (TopExp_Explorer itf(result,TopAbs_SHELL); itf.More(); itf.Next()) {
	TopoDS_Shell &shell = (TopoDS_Shell&)itf.Current();
	
	HandleThisSolidWrapper(BRepBuilderAPI_MakeSolid(shell).Solid(), mConfig->mhub()->fSingleMedium, 0);
      } //for itf
    }
  } 
} // EicCadFile::ConstructIgesGeometry()
#endif

// ---------------------------------------------------------------------------------------

#ifdef _ELMER_
void EicCadFile::ConstructElmerGeometry()
{
  mElmerMesh = new mesh_t();

  // Perform basic Elmer import; FIXME: check error codes;
  if (eg_loadmesh(GetGeometryFileName().Data()) || eg_transfermesh(mElmerMesh, "test"))
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Elmer library import calls failed on %s ..! \033[0m",
		   GetGeometryFileName().Data()); 

  printf("@@@: %d elements\n", mElmerMesh->getElements());
  for(unsigned el=0; el<mElmerMesh->getElements(); el++) {
    element_t *element = mElmerMesh->getElement(el);

    assert(element->getCode() == 504 && element->getNodes() == 4);
    //if (element->getCode() != 504 || element->getNodes() != 4) continue;
    //if (element->getCode() != 504)
    //printf("@@@ element %5d -> code# %4d; %d nodes; index: %5d\n", 
    //	   el, element->getCode(), element->getNodes(), element->getIndex());

    TVector3 points[4];
    for(unsigned nd=0; nd<element->getNodes(); nd++) {
      //unsigned node = element->getNodeIndex(nd);
      //printf("  -> %5d\n", node);
      node_t *node = mElmerMesh->getNode(element->getNodeIndex(nd));//node);
      //printf("%10.4f %10.4f %10.4f; %5d\n", 
      //     node->getXvec()[0], node->getXvec()[1], node->getXvec()[2], node->getIndex()); 

      points[nd] = config()->scale() * config()->units() * TVector3(node->getXvec());
    } //for nd

    {
      char name[128];
      snprintf(name, 128-1, "%sTet%06d", dname->Name().Data(), el);

      TGeoVolume *vtet = CreateTetrahedron(points[0], points[1], points[2], points[3], name, mCave, mConfig->mhub()->fSingleMedium, 
					   mExtraStlTranslation);//
      vtet->SetFillColor(mFillColor);
      //vtet->SetTransparency(70);
      // FIXME: this is crap of course;
      if (mCreateStlMirrorCopyXZ) {
	TVector3 qpoints[4];
	char qname[128];
	snprintf(qname, 128-1, "%sTet%06dxz", dname->Name().Data(), el);

	for(unsigned pt=0; pt<4; pt++) {
	  qpoints[pt] = points[pt];
	  qpoints[pt].SetY(-qpoints[pt].y());
	} //for pt

	CreateTetrahedron(qpoints[0], qpoints[1], qpoints[2], qpoints[3], qname, mCave, mConfig->mhub()->fSingleMedium, 
			  mExtraStlTranslation)->SetFillColor(mFillColor);
      } //if
      if (mCreateStlMirrorCopyXY) {
	TVector3 qpoints[4];
	char qname[128];
	snprintf(qname, 128-1, "%sTet%06dxy", dname->Name().Data(), el);

	for(unsigned pt=0; pt<4; pt++) {
	  qpoints[pt] = points[pt];
	  qpoints[pt].SetZ(-qpoints[pt].z());
	} //for pt

	CreateTetrahedron(qpoints[0], qpoints[1], qpoints[2], qpoints[3], qname, mCave, mConfig->mhub()->fSingleMedium, 
			  mExtraStlTranslation)->SetFillColor(mFillColor);
      } //if
      if (mCreateStlMirrorCopyXY && mCreateStlMirrorCopyXZ) {
	TVector3 qpoints[4];
	char qname[128];
	snprintf(qname, 128-1, "%sTet%06dxyz", dname->Name().Data(), el);

	for(unsigned pt=0; pt<4; pt++) {
	  qpoints[pt] = points[pt];
	  qpoints[pt].SetY(-qpoints[pt].y());
	  qpoints[pt].SetZ(-qpoints[pt].z());
	} //for pt

	CreateTetrahedron(qpoints[0], qpoints[1], qpoints[2], qpoints[3], qname, mCave, mConfig->mhub()->fSingleMedium, 
			  mExtraStlTranslation)->SetFillColor(mFillColor);
      } //if
    } //for nd
  } //for el
} // EicCadFile::ConstructElmerGeometry()
#endif

// ---------------------------------------------------------------------------------------

void EicCadFile::ConstructGeometry()
{
  mConfig->mhub()->Init();

  // Well, for IGES file fSingleMedium should be clearly non-zero; check right here;
  if (GetGeometryFileName().EndsWith(".igs") && !mConfig->mhub()->fSingleMedium)
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Unknown medium (%s) for .igs file! \033[0m", 
		   mConfig->mhub()->fMediaName.Data());

  // Assume CAVE volume is available as well (and it is the top volume);
  mCave = gGeoManager->GetTopVolume();

  if (GetGeometryFileName().EndsWith(".stp")) {
#ifdef _OPENCASCADE_
    //ConstructDummyStepGeometry();
    ConstructStepGeometry();
#else
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m .stp file import error: OpenCascade "
		   "support is not compiled in! \033[0m"); 
#endif
  }
  else if (GetGeometryFileName().EndsWith(".igs"))
#ifdef _OPENCASCADE_
    ConstructIgesGeometry();
#else
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m .igs file import error: OpenCascade "
		   "support is not compiled in! \033[0m"); 
#endif
  else if (GetGeometryFileName().EndsWith(".stl") || GetGeometryFileName().EndsWith(".slp")) {
    EicStlFactory stl_factory(dname->Name().Data(), GetGeometryFileName(), mConfig);//, mConfig->mhub()mediaHub);
    std::vector<TGeoVolume*> volumes;

    //stl_factory.SetVertexMergingTolerance(mStlVertexMergingTolerance);
    stl_factory._ConstructGeometry(mCave, mExtraStlTranslation, mSwapXY, /*mWireframvmeMode,*/ &volumes);
    if (mKillerFlag)
      for(unsigned iq=0; iq<volumes.size(); iq++)
	AddKillerVolume(volumes[iq]);
  }
  // NB: this may (in fact should) work with any other 3D mesh, as long as 1) Elmer 
  // supports respective format, 2) 3D elements are tetrahedra; FIXME: would not be 
  // a big deal to extend functionality to other elementary cell shapes (like pyramids,
  // prisms, etc);
  else if (GetGeometryFileName().EndsWith(".mphtxt")) 
#ifdef _ELMER_
    ConstructElmerGeometry();
#else
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m .mphtxt file import error: Elmer "
		   "support is not compiled in! \033[0m"); 
#endif
  else
    mLogger->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Only .stp, .igs, .stl, .slp, .mphtxt files "
		   "allowed (and only if support is compiled in)! \033[0m"); 
} // EicCadFile::ConstructGeometry()

// =======================================================================================

ClassImp(EicCadFile)
