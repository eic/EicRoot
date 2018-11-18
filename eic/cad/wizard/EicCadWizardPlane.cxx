
#include <assert.h>

#include <gp_Pln.hxx>

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>

#include <TGeoHalfSpace.h>

#include <EicCadWizardPlane.h>

// =======================================================================================


EicCadWizardPlane::EicCadWizardPlane(const gp_Pln &plane)
{ 
  gp_Pnt pnt = plane.Location();
  gp_Dir dir = plane.Axis().Direction(); 

  // Prefer to avoid issues with identical plane cuts of different orientation;
  // convert any direction vector to a "first non-zero coordinate positive" case;
  if (dir.X()) {
    if (dir.X() < 0.0) {
      dir.SetX(-dir.X()); dir.SetY(-dir.Y()); dir.SetZ(-dir.Z()); 
    } //if
  } else if (dir.Y()) {
    if (dir.Y() < 0.0) {
      dir.SetX(-dir.X()); dir.SetY(-dir.Y()); dir.SetZ(-dir.Z()); 
    } //if
  } else {
    if (dir.Z() < 0.0) {
      dir.SetX(-dir.X()); dir.SetY(-dir.Y()); dir.SetZ(-dir.Z());
    } //if
  } //if
  //printf("@@@ %f %f %f\n", dir.X(), dir.Y(), dir.Z());

  TopoDS_Face aFace = BRepBuilderAPI_MakeFace(gp_Pln(pnt, dir));//plane);

  // Whatever non-zero number;
  double t = 0.1;
  // NB: put "-" sign here, because dir[] is an outer normal in ROOT halfspace volume;
  gp_Pnt x0(plane.Location()), pt(x0.X() - t*dir.X(), x0.Y() - t*dir.Y(), x0.Z() - t*dir.Z());

  mSolid  = new TopoDS_Shape(BRepPrimAPI_MakeHalfSpace(aFace, pt).Solid());
  mPlane = new gp_Pln(x0, dir);
  assert(dir.IsEqual(gp_Pln(x0, dir).Axis().Direction(), _ANGULAR_TOLERANCE_));
} // EicCadWizardPlane::EicCadWizardPlane()

// ---------------------------------------------------------------------------------------

bool EicCadWizardPlane::IsEqual(const EicCadWizardCut *cut) const
{
  // Check object type first;
  const EicCadWizardPlane *other = dynamic_cast<const EicCadWizardPlane*>(cut);
  if (!other) return false;

  const gp_Pln *pll = mPlane, *plr = other->mPlane;

  if (pll->Axis().Direction().IsParallel(plr->Axis().Direction(), _ANGULAR_TOLERANCE_)) {
    if (pll->Distance(plr->Location()) > _SPATIAL_TOLERANCE_) return false;

    // Ok, do it better later; should not happen;
    //assert(lh.first == rh.first);

    return true;
  } else {
#if _THINK_
    // Try anti-parallel option; gp_Dir does not have easy reflection methods, sorry;
    gp_Dir anti(-plr->Axis().Direction().X(), 
		-plr->Axis().Direction().Y(), 
		-plr->Axis().Direction().Z());

    if (pll->Axis().Direction().IsParallel(anti, _ANGULAR_TOLERANCE_)) {
      if (pll->Distance(plr->Location()) > _SPATIAL_TOLERANCE_) return false;
      
      // Ok, do it better later; should not happen;
      //assert(lh.first != rh.first);

      // FIXME: check that can work with both orientations (flip the flag 
      // at respective assignment in EicCadWizard::FaceGuidedSplit() method);
      assert(0);
      return true;
    } 
    else
#endif
      return false;
  } //if
} // EicCadWizardPlane::IsEqual()

// ---------------------------------------------------------------------------------------

TGeoCombiTrans *EicCadWizardPlane::BuildRootVolume(const char *vname, const char *tname)
{
  double x0[3] = {mPlane->Location().X(),         mPlane->Location().Y(),         mPlane->Location().Z()};
  double n0[3] = {mPlane->Axis().Direction().X(), mPlane->Axis().Direction().Y(), mPlane->Axis().Direction().Z()};

  //printf("PL (%s) -> x0[]: %7.2f %7.2f %7.2f; n0[]: %7.2f %7.2f %7.2f\n", 
  //	 facets[fc].first ? "-" : "*", x0[0], x0[1], x0[2], n0[0], n0[1], n0[2]); 

  new TGeoHalfSpace(vname, x0, n0);

  return 0;
} // EicCadWizardPlane::BuildRootVolume()

// =======================================================================================
