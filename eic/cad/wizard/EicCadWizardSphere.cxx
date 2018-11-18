
#include <gp_Sphere.hxx>

#include <BRepPrimAPI_MakeSphere.hxx>

#include <TGeoMatrix.h>
#include <TGeoSphere.h>

#include <EicCadWizardSphere.h>

// =======================================================================================

EicCadWizardSphere::EicCadWizardSphere(const gp_Sphere &sphere)
{
  // Just need exactly this sphere, please;
  mSphere = new gp_Sphere(sphere);
  mSolid = new TopoDS_Shape(BRepPrimAPI_MakeSphere(sphere.Location(), sphere.Radius()).Solid());
} // EicCadWizardSphere::EicCadWizardSphere()

// ---------------------------------------------------------------------------------------

bool EicCadWizardSphere::IsEqual(const EicCadWizardCut *cut) const
{
  // Check object type first;
  const EicCadWizardSphere *other = dynamic_cast<const EicCadWizardSphere*>(cut);
  if (!other) return false;

  // FIXME: llow for some tolerance later, please! (?);
  const gp_Sphere *spl = mSphere, *spr = other->mSphere;

  if (fabs(spl->Radius() - spr->Radius()) > _SPATIAL_TOLERANCE_) return false;

  {
    double dc = sqrt(SQR(spl->Location().X() - spr->Location().X()) +
		     SQR(spl->Location().Y() - spr->Location().Y()) +
		     SQR(spl->Location().Z() - spr->Location().Z()));
    
    if (dc > _SPATIAL_TOLERANCE_) return false;
  }

  return true;
} // EicCadWizardSphere::IsEqual()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *EicCadWizardSphere::BuildRootVolume(const char *vname, const char *tname)
{
  new TGeoSphere(vname, 0.0, mSphere->Radius());

  return new TGeoCombiTrans(tname, mSphere->Location().X(), 
			    mSphere->Location().Y(), 
			    mSphere->Location().Z(), 0);

  //printf("SP (%s) x0[]: %f %f %f, r = %f\n", facets[fc].first ? "-" : "*", 
  //	 sphere->Location().X(), 
  //	 sphere->Location().Y(), sphere->Location().Z(), sphere->Radius());
} // EicCadWizardSphere::BuildRootVolume()

// =======================================================================================
