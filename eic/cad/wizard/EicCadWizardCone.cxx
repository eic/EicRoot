
#include <gp_Cone.hxx>

#include <BRepPrimAPI_MakeCone.hxx>

#include <EicCadWizardCone.h>

#include <TGeoCone.h>

// =======================================================================================

EicCadWizardCone::EicCadWizardCone(const gp_Cone &cone, const gp_Pnt *bcenter, double bradius)
{
  //gp_Cone cone = es->Cone();
  gp_Pnt apex = cone.Apex();
  gp_Ax1 axis = cone.Axis();
  gp_Ax2 ax2(apex, axis.Direction());
  
  double dist_to_sphere = apex.Distance(*bcenter);
  // This should suffice in all cases;
  double safe_dimension = dist_to_sphere + bradius;
  double semiAngle = cone.SemiAngle(), r0 = 0.0, r1 = safe_dimension*tan(semiAngle);
  
  mCone  = new gp_Cone(gp_Ax3(ax2), semiAngle, r0);
  mDimension = safe_dimension;
  mSolid = new TopoDS_Shape(BRepPrimAPI_MakeCone(ax2, r0, r1, safe_dimension).Solid());
} // EicCadWizardCone::EicCadWizardCone()

// ---------------------------------------------------------------------------------------

bool EicCadWizardCone::IsEqual(const EicCadWizardCut *cut) const
{
  // Check object type first;
  const EicCadWizardCone *other = dynamic_cast<const EicCadWizardCone*>(cut);
  if (!other) return false;

  const gp_Cone *col = mCone, *cor = other->mCone;

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
  
  return true;
} // EicCadWizardCone::IsEqual()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *EicCadWizardCone::BuildRootVolume(const char *vname, const char *tname)
{
  new TGeoCone(vname, mDimension/2.,
	       0.0, 0.0, 
	       0.0, mDimension*tan(mCone->SemiAngle()));
  
  return Ax3ToCombiTrans(tname, mCone->Position(), mDimension/2.);
  
  //printf("CO (%s) x0[]: %f %f %f\n", facets[fc].first ? "-" : "*", 
  //	 cone->Location().X(), cone->Location().Y(), cone->Location().Z());
} // EicCadWizardCone::BuildRootVolume()

// =======================================================================================
