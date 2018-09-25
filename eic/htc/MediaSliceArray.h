//
// AYK (ayk@bnl.gov), shaped up in Nov'2015;
//
//  Array of media slices;
//

#include <vector>

#ifndef _MEDIA_SLICE_ARRAY_
#define _MEDIA_SLICE_ARRAY_

class MediaBank;
class t_particle_group;

#include <MediaSlice.h>

class MediaSliceArray {
 public:
  MediaSliceArray(MediaBank *bank, double z1, double z2);

  double GetZ0()                         const { return mZ0; };
  double GetThickness()                  const { return mThickness; };
  unsigned GetMediaSliceCount()          const { return mMediaSlices.size(); };
  MediaSlice *GetMediaSlice(unsigned iq) { 
    return (iq < mMediaSlices.size() ? &mMediaSlices[iq] : 0); 
  };

  // dE over this material slice array for a given particle type and energy; 
  double GetDE(t_particle_group *pgroup, int charge, double e0_kin);

 private:
  double mZ0, mThickness;

  std::vector<MediaSlice> mMediaSlices;
};

#endif
