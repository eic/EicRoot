//
// AYK (ayk@bnl.gov)
//
//  Code helping to remap MC tracks into RC ones;
//
//

#include <map>

#include <TObject.h>
#include <TClonesArray.h>

#include <PndMCTrack.h>

#include <FwdMatchCandidate.h>

#ifndef _FWD_MC_TRACK_MAPPER_
#define _FWD_MC_TRACK_MAPPER_

class FwdMcTrackMapper: public TObject
{
 public:
 FwdMcTrackMapper(TClonesArray *mcarr = 0, TClonesArray *rcarr = 0): 
  mcTrackArray(mcarr), rcTrackArray(rcarr) {};
  ~FwdMcTrackMapper() {};

  int Rebuild();

  // FIXME: not exactly efficient stuff; first call allows to get number 
  // of RC tracks which associated themselves with this particular MC track;
  // second call basically builds the same pair of iterators and pulls out
  // N-th track out of the matching set;
  unsigned GetRcTrackCount(PndMCTrack *mctrack);
  FwdMatchCandidate *GetRcTrack(PndMCTrack *mctrack, unsigned id);

 private:
  TClonesArray *mcTrackArray, *rcTrackArray;

  std::multimap<PndMCTrack*, FwdMatchCandidate*> mMcToRcMap;

  ClassDef(FwdMcTrackMapper,2);
};

#endif
