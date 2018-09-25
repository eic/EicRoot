#include "PndGenfitField.h"
#include "FairRunAna.h"
#include "FairField.h"

PndGenfitField::PndGenfitField() : GFAbsBField()
{
}

TVector3 PndGenfitField::get(const TVector3& pos) const
{
  double x[3] = {pos.X(), pos.Y(), pos.Z()};
  double B[3] = {0,0,0};
  FairRunAna::Instance()->GetField()->Field(x,B);
  return TVector3(B[0], B[1], B[2]);
}
