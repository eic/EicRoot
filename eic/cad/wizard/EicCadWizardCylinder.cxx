

#include <gp_Cylinder.hxx>

#include <BRepPrimAPI_MakeCylinder.hxx>

#include <TGeoTube.h>

#include <EicCadWizardCylinder.h>

// =======================================================================================

EicCadWizardCylinder::EicCadWizardCylinder(const gp_Cylinder &cylinder, const gp_Pnt *bcenter, double bradius)
{
  double r = cylinder.Radius();

  gp_Ax1 ax1 = cylinder.Axis();
  gp_Dir dir = ax1.Direction();
  gp_Ax2 ax2 = cylinder.Position().Ax2();
  gp_Pnt x0 = cylinder.Location();
  double dist_to_sphere = cylinder.Location().Distance(*bcenter);
  double safe_dimension = dist_to_sphere + bradius;
  double t = safe_dimension;
  gp_Pnt x1(x0.X() - t*dir.X(), x0.Y()- t*dir.Y(), x0.Z()- t*dir.Z());
  ax2.SetLocation(x1);
  ax2.SetDirection(dir);
  mSolid = new TopoDS_Shape(BRepPrimAPI_MakeCylinder(ax2, r, 2 * safe_dimension).Solid());
  
  mCylinder = new gp_Cylinder(gp_Ax3(ax2), r);
  mDimension = safe_dimension;
} // EicCadWizardCylinder::EicCadWizardCylinder()

// ---------------------------------------------------------------------------------------

bool EicCadWizardCylinder::IsEqual(const EicCadWizardCut *cut) const
{
  // Check object type first;
  const EicCadWizardCylinder *other = dynamic_cast<const EicCadWizardCylinder*>(cut);
  if (!other) return false;

  const gp_Cylinder *cyl = mCylinder, *cyr = other->mCylinder;
  
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

  return true;
} // EicCadWizardCylinder::IsEqual()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *EicCadWizardCylinder::BuildRootVolume(const char *vname, const char *tname)
{
  new TGeoTube(vname, 0.0, mCylinder->Radius(), mDimension);

  return Ax3ToCombiTrans(tname, mCylinder->Position(), mDimension);

  //printf("CY (%s) x0[]: %f %f %f, r = %f\n", facets[fc].first ? "-" : "*", 
  //	 mCylinder->Location().X(), 
  //	 mCylinder->Location().Y(), mCylinder->Location().Z(), mCylinder->Radius());
  //break;
} // EicCadWizardCylinder::BuildRootVolume()

// =======================================================================================
