//
// AYK (ayk@bnl.gov)
//
//  Match candidate group (overlap via hits) class;
//
//

#include <map>

#include <MatchCandidate.h>

#ifndef _MATCH_CANDIDATE_GROUP_
#define _MATCH_CANDIDATE_GROUP_

class MatchCandidateGroup
{
 public:
  MatchCandidateGroup(MatchCandidate *match) { AddCandidate(match); };
  ~MatchCandidateGroup() {};

  void AddCandidate(MatchCandidate *match);
  
  unsigned GetCandidateCount() const { return mMatchCandidates.size(); };

  // FIXME: allocate iterators once?;
  std::multimap<__u64, MatchCandidate*>::iterator Begin() { return mMatchCandidates.begin(); };
  std::multimap<__u64, MatchCandidate*>::iterator End()   { return mMatchCandidates.end(); };

 private:
  std::set<GroupMember*> mHits;
  std::multimap<__u64, MatchCandidate*> mMatchCandidates;
};

#endif
