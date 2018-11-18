
#include <EicCadWizardCut.h>

#ifndef _EIC_CAD_WIZARD_CYLINDER_
#define _EIC_CAD_WIZARD_CYLINDER_

class gp_Pnt;
class gp_Cylinder;

class EicCadWizardCylinder: public EicCadWizardCut {
 public:
  EicCadWizardCylinder(const gp_Cylinder &cylinder, const gp_Pnt *bcenter, double bradius);
  ~EicCadWizardCylinder() {};

  bool IsEqual(const EicCadWizardCut *cut) const;
  TGeoCombiTrans *BuildRootVolume(const char *vname, const char *tname);

 private:
  gp_Cylinder *mCylinder;
};

#endif
