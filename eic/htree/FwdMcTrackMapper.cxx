//
// AYK (ayk@bnl.gov)
//
//  Code helping to remap MC tracks into RC ones;
//
//

#include <FwdMcTrackMapper.h>

// ---------------------------------------------------------------------------------------

int FwdMcTrackMapper::Rebuild()
{
  if (!mcTrackArray || !rcTrackArray) return -1;

  mMcToRcMap.clear();

  for(unsigned rc=0; rc<rcTrackArray->GetEntriesFast(); rc++) {
    FwdMatchCandidate *rctrack = (FwdMatchCandidate *)rcTrackArray->At(rc);

    // FIXME: used in rc2mc.C as well -> make a routine;
    int mcTrackId = rctrack->GetMcTrackId();

    assert(mcTrackId >= 0 && mcTrackId < mcTrackArray->GetEntriesFast());
    // Select only correctly rc->mc identified tracks;
    if (mcTrackId < 0 || mcTrackId >= mcTrackArray->GetEntriesFast()) continue;
    
    // Find MC track associated with this reconstructed track;
    PndMCTrack *mctrack = (PndMCTrack *)mcTrackArray->At(mcTrackId);

    mMcToRcMap.insert(std::pair<PndMCTrack*, FwdMatchCandidate*>(mctrack, rctrack));
  } //for rc

  return 0;
} // FwdMcTrackMapper::Rebuild()

// ---------------------------------------------------------------------------------------

unsigned FwdMcTrackMapper::GetRcTrackCount(PndMCTrack *mctrack)
{
  // Get range of matching entries;
  std::pair <std::multimap<PndMCTrack*, FwdMatchCandidate*>::iterator, 
    std::multimap<PndMCTrack*, FwdMatchCandidate*>::iterator> ret = mMcToRcMap.equal_range(mctrack);

  // And count them now;
  unsigned counter = 0;

  for(std::multimap<PndMCTrack*, FwdMatchCandidate*>::iterator it=ret.first; it != ret.second; it++)
    counter++;

  return counter;
} // FwdMcTrackMapper::GetRcTrackCount()

// ---------------------------------------------------------------------------------------

FwdMatchCandidate *FwdMcTrackMapper::GetRcTrack(PndMCTrack *mctrack, unsigned id)
{
  // Get range of matching entries;
  std::pair <std::multimap<PndMCTrack*, FwdMatchCandidate*>::iterator, 
    std::multimap<PndMCTrack*, FwdMatchCandidate*>::iterator> range = mMcToRcMap.equal_range(mctrack);

  // And loop through them now;
  unsigned counter = 0;

  for(std::multimap<PndMCTrack*, FwdMatchCandidate*>::iterator it=range.first; it != range.second; it++) 
    if (counter++ == id) return it->second;

  // 'id' was too big (no this N-th entry in the multimap);
  return 0;
} // FwdMcTrackMapper::GetRcTrack()

// ---------------------------------------------------------------------------------------

ClassImp(FwdMcTrackMapper)
