
#include <EicCadWizardCut.h>

#ifndef _EIC_CAD_WIZARD_TORUS_
#define _EIC_CAD_WIZARD_TORUS_

class gp_Torus;

class EicCadWizardTorus: public EicCadWizardCut {
 public:
  EicCadWizardTorus(const gp_Torus &torus);//, const gp_Pnt *bcenter, double bradius);
  ~EicCadWizardTorus() {};

  bool IsEqual(const EicCadWizardCut *cut) const;
  TGeoCombiTrans *BuildRootVolume(const char *vname, const char *tname);

 private:
  gp_Torus *mTorus;
};

#endif
