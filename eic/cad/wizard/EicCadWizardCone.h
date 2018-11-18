
#include <EicCadWizardCut.h>

#ifndef _EIC_CAD_WIZARD_CONE_
#define _EIC_CAD_WIZARD_CONE_

class gp_Cone;

class EicCadWizardCone: public EicCadWizardCut {
 public:
  EicCadWizardCone(const gp_Cone &cone, const gp_Pnt *bcenter, double bradius);
  ~EicCadWizardCone() {};

  bool IsEqual(const EicCadWizardCut *cut) const;

  TGeoCombiTrans *BuildRootVolume(const char *vname, const char *tname);

 private:
  gp_Cone *mCone;
};

#endif
