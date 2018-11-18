
#include <EicCadWizardCut.h>

#ifndef _EIC_CAD_WIZARD_SPHERE_
#define _EIC_CAD_WIZARD_SPHERE_

class gp_Sphere;

class EicCadWizardSphere: public EicCadWizardCut {
 public:
  EicCadWizardSphere(const gp_Sphere &sphere);//, const gp_Pnt *bcenter, double bradius);
  ~EicCadWizardSphere() {};

  bool IsEqual(const EicCadWizardCut *cut) const;
  TGeoCombiTrans *BuildRootVolume(const char *vname, const char *tname);

 private:
  gp_Sphere *mSphere;
};

#endif
