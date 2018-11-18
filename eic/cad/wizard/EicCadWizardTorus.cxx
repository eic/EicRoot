
#include <gp_Torus.hxx>

#include <BRepPrimAPI_MakeTorus.hxx>

#include <TGeoTorus.h>

#include <EicCadWizardTorus.h>

// =======================================================================================

EicCadWizardTorus::EicCadWizardTorus(const gp_Torus &torus)
{
  // Just need exactly this torus, please;
  mTorus  = new gp_Torus(torus);
  mSolid = new TopoDS_Shape(BRepPrimAPI_MakeTorus(torus.Position().Ax2(), torus.MajorRadius(), 
						  torus.MinorRadius()).Solid());
} // EicCadWizardTorus::EicCadWizardTorus()

// ---------------------------------------------------------------------------------------

bool EicCadWizardTorus::IsEqual(const EicCadWizardCut *cut) const
{
  // Check object type first;
  const EicCadWizardTorus *other = dynamic_cast<const EicCadWizardTorus*>(cut);
  if (!other) return false;

  const gp_Torus *tol = mTorus, *tor = other->mTorus;

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

  return true;
} // EicCadWizardTorus::IsEqual()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *EicCadWizardTorus::BuildRootVolume(const char *vname, const char *tname)
{
  new TGeoTorus(vname, mTorus->MajorRadius(), 0.0, mTorus->MinorRadius(), 0.0, 360.0);

  return Ax3ToCombiTrans(tname, mTorus->Position(), 0.0);

  //printf("TO (%s) x0[]: %f %f %f, rMa = %f, rMi = %f\n", facets[fc].first ? "-" : "*", 
  //		 mTorus->Location().X(), 
  //		 mTorus->Location().Y(), mTorus->Location().Z(), 
  //		 mTorus->MajorRadius(), mTorus->MinorRadius());
} // EicCadWizardTorus::BuildRootVolume()

// =======================================================================================
