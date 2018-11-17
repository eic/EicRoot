//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; ordering key class;
//

#include <cstring>

#ifndef _EIC_STL_KEY_
#define _EIC_STL_KEY_

/// \brief Naive multi-float-value key used for ordering objects in CAD-related classes;
///
/// Basically just packs coordinates of N related 3D vertices into double[3*N] arrays
/// and uses memcmp() call for comparisons; FIXME: consider to use hash tables (?);
class EicStlKey {
 public:
  /// Main constructor
  ///
  /// @param dim          \a full array dimension (so 3N for an object identified by N 3D points)
  /// @param ptr          pointer to respective double[dim] data array
  EicStlKey(unsigned dim, const double *ptr) {
    mDim  = dim;
    mData = new double[dim];
    memcpy(mData, ptr, dim*sizeof(double));
  };
  /// Destructor; just need to clean up the storage
  ///
  ~EicStlKey() { delete [] mData; };

  /// Access method; returns \a mDim
  ///
  unsigned GetDim() const { return mDim;};
  /// Access method; returns read-only \a mData pointer
  ///
  const double *GetData() const { return mData;};

 private:
  /*! Number of floating point values defining this key */
  unsigned mDim;

  /*! Array of floating point values */
  double *mData;
};

bool EicStlKeyCompare(const EicStlKey *lh, const EicStlKey *rh);
bool EicStlKeyEqual(const EicStlKey *lh, const EicStlKey *rh);

#endif
