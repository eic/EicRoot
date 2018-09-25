//
// RMP (rpetti@bnl.gov), 2016/03/02
//
//  EIC magnetic field gradients;
//

#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <FairLogger.h>

#include <EicLibrary.h>
#include <EicMagneticFieldGrad.h>

// =======================================================================================

// constructor
EicMagneticFieldGrad::EicMagneticFieldGrad(const char *fileName, TGeoMatrix *transformation, 
					 TGeoShape *shape, int color):
  mTransformation(transformation), mColor(color), mInitialized(false)
{ 
  if (fileName) {
    mFileName = TString(fileName); 

   
  } //if
  else {
    FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' field!  \033[0m", 
		     fileName);
  }

  // NB: here and in other similar places have to call Clone() method and cast 
  // TObject* to TGeoShape* dynamically because this way a default constructor 
  // is called and the TGeoShape* pointer is not bound to the current TGeoManager;
  // any attempt to simplify things to 'mShape = new TGeoBBox(...)' cause segfault
  // in the main simulation code because TGeoManager stuff gets ~reloaded and 
  // (as far as I understand) pointers get "orphan" (so that data members are 
  // available and one can use something like (new TGeoShape(mShape))->Contains() 
  // call, but not mShape->Contains() directly);
  mShape = shape ? dynamic_cast<TGeoShape*>(shape->Clone()) : 0;
} // EicMagneticFieldGrad::EicMagneticFieldGrad()

// ---------------------------------------------------------------------------------------

int EicMagneticFieldGrad::Initialize()
{ 
  
  mInitialized = true; 

  return 0; 
} // EicMagneticFieldGrad::Initialize()

// ---------------------------------------------------------------------------------------


bool EicMagneticFieldGrad::Contains(const double xx[]) const
{
  // If the shape is not given, can not say anything -> return false; NB: it is 
  // assumed, that in a mixed environment (some maps have shapes, some not) voxel 
  // builder should make GetShape() call first (and if there is no shape returned, 
  // such a map is considered "global", ie will be queried basically for every xx[]);
  if (!mShape) return false;

  double local[3];

  // Move into the shape local coordinate system; if transformation is not given, 
  // assume map coordinate system matches the world one;
  if (mTransformation)
    mTransformation->MasterToLocal(xx, local);
  else
    memcpy(local, xx, sizeof(local));

  // And then use ROOT library call for the bounding shape; 
  return mShape->Contains(local);
} // EicMagneticFieldGrad::Contains()
