//
// AYK (ayk@bnl.gov)
//
//  STAR forward track candidate class;
//
//

#include <math.h>

#include <FwdMatchCandidate.h>

// ---------------------------------------------------------------------------------------

void FwdMatchCandidate::SetVtxMomentum(double sx, double sy, double invp)
{
  assert(invp);
  // THINK: may want to preserve sign here?; well, mCharge will do;
  double p = fabs(1/invp), norm = sqrt(1.0 + sx*sx + sy*sy);

  mCharge = invp > 0.0 ? 1 : -1;

  printf("%f %f %f\n", sx, sy, p);
  mMomentum = p * TVector3(sx/norm, sy/norm, 1.0/norm);
} // FwdMatchCandidate::SetVtxMomentum()

// ---------------------------------------------------------------------------------------

ClassImp(FwdMatchCandidate)
