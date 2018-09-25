//
// AYK (ayk@bnl.gov), 2015/08/06;
//
//  Parent-hierarchy-related class; see the codes for more details;
//

#include <utility>
#include <set>

#ifndef _EIC_BLACK_HOLE_
#define _EIC_BLACK_HOLE_

class EicBlackHole { 
 public:
 EicBlackHole() {};
  ~EicBlackHole() {};

  /// Figure out GEANT primary track ID which produced hit in the current volume 
  ///
  /// Dig parent tree up to the primary particle; want to store this info
  /// with every MC hit; useful at least for calorimeter ideal reconstruction;
  static std::pair<int, int> GetParentIDs();

  static void ResetTrackList() {mTracks->clear(); };
  static void InsertIntoTrackList(unsigned id) {mTracks->insert(id); };
  static bool IsInTrackList(unsigned id) { 
    return (mTracks->find(id) != mTracks->end());
  };

 private:
  static std::set<unsigned> *mTracks;
};

#endif
