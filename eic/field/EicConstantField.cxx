// AYK (ayk@bnl.gov), 2014/09/03
//
//  Constant field handler;
//

#include <EicConstantField.h>

// =======================================================================================

int EicConstantField::Initialize() 
{
  if (mTransformation)
    mTransformation->LocalToMasterVect(mFieldLocal, mFieldGlobal);
  else
    memcpy(mFieldGlobal, mFieldLocal, sizeof(mFieldGlobal));

  // Basically set mInitialized flag;
  return EicMagneticFieldMap::Initialize();
} // EicConstantField::Initialize() 

// ---------------------------------------------------------------------------------------

int EicConstantField::GetFieldValue(const double xx[], double B[]) const 
{
  if (!mInitialized) return -1;
  
  // Well, otherwise basically assume a constant field everywhere;
  if (GetShape() && !Contains(xx)) return -1;

  //printf("%f %f %f\n", mFieldGlobal[0], mFieldGlobal[1], mFieldGlobal[2]);
  // And return once calculated rotated field value;
  for(unsigned iq=0; iq<3; iq++)
    B[iq] = mFieldGlobal[iq];
  
  return 0;
} // EicConstantField::GetFieldValue()

// =======================================================================================

ClassImp(EicConstantField)
ClassImp(EicConstantBoxField)
ClassImp(EicConstantTubeField)
