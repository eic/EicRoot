
#include <EicCadWizardCut.h>

#ifndef _EIC_CAD_WIZARD_PLANE_
#define _EIC_CAD_WIZARD_PLANE_

class gp_Pln;

class EicCadWizardPlane: public EicCadWizardCut {
 public:
 EicCadWizardPlane(const gp_Pln &plane);
  ~EicCadWizardPlane() {};

  bool IsEqual(const EicCadWizardCut *cut) const;
  TGeoCombiTrans *BuildRootVolume(const char *vname, const char *tname);

 private:
  gp_Pln *mPlane;
};

#endif
