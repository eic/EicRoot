//
// AYK (ayk@bnl.gov)
//
//  Match candidate group (overlap via hits) class;
//
//

#include <MatchCandidateGroup.h>

// ---------------------------------------------------------------------------------------

void MatchCandidateGroup::AddCandidate(MatchCandidate *match) 
{ 
  // Mark this candidate as "some-group-bound";
  match->SetGroupedStatus(true);
  
  // Place it into the candidate vector;
  mMatchCandidates.insert(std::pair<__u64, MatchCandidate*>(match->Ambiguity(), match)); 

  // Loop through all its hits in all groups and pull out other candidates;
  for(unsigned gr=0; gr<match->GetGdim(); gr++) {
    for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
      GroupMember *member = match->GetSelMember(gr,mm); //assert(member);

      // If hit was accounted already, skip all the rest;
      if (!member || mHits.find(member) != mHits.end()) continue;

      // Otherwise insert it and recursively loop through all track candidates
      // which contain it;
      mHits.insert(member);

      for(std::set<MatchCandidate*>::iterator it=member->Begin(); it != member->End(); it++) {
	MatchCandidate *qmatch = *it;

	if (!qmatch->IsGrouped() && qmatch->IsActive() && qmatch != match) 
	  AddCandidate(qmatch);
      } //for iq
    } //for mm
  } //for gr
} // MatchCandidateGroup::AddCandidate()

// ---------------------------------------------------------------------------------------
