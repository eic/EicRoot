//
// AYK (ayk@bnl.gov), 2014/09/03
//
//  EIC magnetic field map;
//

#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifdef __APPLE__
#include <unistd.h>
#include <libgen.h>
#endif

#include <FairLogger.h>

#include <EicLibrary.h>
#include <EicMagneticFieldMap.h>

// =======================================================================================

const char *EicMagneticFieldMap::BasenameWrapper(const char *fname) const {
  // basename() can screw up this buffer, fine;                                                                                                                  
  char buffer[1024];
  snprintf(buffer, 1024-1, "%s", fname);

  return basename(buffer);
} // EicMagneticFieldMap::BasenameWrapper()

// --------------------------------------------------------------------------------------- 

int EicMagneticFieldMap::GetMD5Signature(unsigned char output[])
{
  int fID = open(ExpandedFileName("input/", mFileName.Data()), O_RDONLY);

  if (fID < 0) return -1;

  struct stat statbuf;
  if(fstat(fID, &statbuf) < 0) return -1;

  // I guess this should work for my purposes;
  void *file_buffer = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fID, 0);
  MD5((unsigned char*) file_buffer, statbuf.st_size, output);

  close(fID);

  return 0;
} // EicMagneticFieldMap::GetMD5Signature()

// ---------------------------------------------------------------------------------------


EicMagneticFieldMap::EicMagneticFieldMap(const char *fileName, TGeoMatrix *transformation, 
					 TGeoShape *shape, int color):
  mTransformation(transformation), mInitialized(false), mMD5BufferSize(0), mMD5Signature(0),
  mColor(color)
{ 
  if (fileName) {
    mFileName = TString(fileName); 

#if _TODAY_
    unsigned char md5[MD5_DIGEST_LENGTH];
    
    if (GetMD5Signature(md5))
      FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' field!  \033[0m", 
		     fileName);

    // Succeeded -> copy over;
    mMD5BufferSize = MD5_DIGEST_LENGTH;
    mMD5Signature = new UChar_t[MD5_DIGEST_LENGTH];
    memcpy(mMD5Signature, md5, sizeof(md5));
#endif
  } //if

  // NB: here and in other similar places have to call Clone() method and cast 
  // TObject* to TGeoShape* dynamically because this way a default constructor 
  // is called and the TGeoShape* pointer is not bound to the current TGeoManager;
  // any attempt to simplify things to 'mShape = new TGeoBBox(...)' cause segfault
  // in the main simulation code because TGeoManager stuff gets ~reloaded and 
  // (as far as I understand) pointers get "orphan" (so that data members are 
  // available and one can use something like (new TGeoShape(mShape))->Contains() 
  // call, but not mShape->Contains() directly);
  mShape = shape ? dynamic_cast<TGeoShape*>(shape->Clone()) : 0;
} // EicMagneticFieldMap::EicMagneticFieldMap()

// ---------------------------------------------------------------------------------------

int EicMagneticFieldMap::Initialize()
{ 
#if _TODAY_
  // If file name is available, check MD5 signature;
  if (!mFileName.IsNull()) {
    // Yes, then MD5 signature should have been encoded;
    assert(mMD5Signature);

    unsigned char md5[MD5_DIGEST_LENGTH];
  
    if (GetMD5Signature(md5))
      FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m Failed to open '%s' field!  \033[0m", 
				     mFileName.Data());

    // FIXME: may want to release this constraint later (force from command line);
    if (memcmp(md5, mMD5Signature, sizeof(md5)))
      FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "\033[5m\033[31m File '%s' MD5 signature mismatch!  \033[0m", 
				     mFileName.Data());
  } //if
#endif

  mInitialized = true; 

  return 0; 
} // EicMagneticFieldMap::Initialize()

// ---------------------------------------------------------------------------------------

bool EicMagneticFieldMap::Contains(const double xx[]) const
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
} // EicMagneticFieldMap::Contains()

// =======================================================================================

ClassImp(EicMagneticFieldMap)
