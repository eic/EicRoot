//
// AYK (ayk@bnl.gov)
//
//  Match candidate (possible track) class;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <MatchCandidate.h>
#include <HoughTree.h>

// ---------------------------------------------------------------------------------------

void MatchCandidate::ResetToTheVirginState()
{
  ResetOkGroupCounter();

  // THINK: this is in fact needed on highest resolution level only; overhead is small though;
  if (mGdim)
    for(unsigned gr=0; gr<mGdim; gr++) 
      ResetMemberCount(gr);
    
  mActive = mGrouped = false;
} // MatchCandidate::ResetToTheVirginState()

// ---------------------------------------------------------------------------------------

MatchCandidate::MatchCandidate(const HoughTree *tree):
  mActive(false),
  mGrouped(false),
  //mAccountedAsGoodTrack(false),
  mOkGroupCounter(0),
  mGdim(0),
  mId(0),
  mMemberCount(0),
#ifdef _USE_ALL_MEMBERS_VECTOR_
  mAllMembers(0),
#endif
  mSelMembers(0)
{
  if (tree) {
    mGdim        = tree->GetGdim(); 
    
    mId          = new unsigned[tree->GetDdim()];
    
    mMemberCount = new unsigned[mGdim];
#ifdef _USE_ALL_MEMBERS_VECTOR_
    mAllMembers  = new std::vector<GroupMember*>[mGdim];
#endif
    mSelMembers  = new std::vector<GroupMember*>[mGdim];
  } //if
} // MatchCandidate::MatchCandidate()

// ---------------------------------------------------------------------------------------

void MatchCandidate::ShapeItUpForInspection(const HoughTree *tree, const unsigned id[])
{
  mActive = true;

  for(unsigned iq=0; iq<tree->GetDdim(); iq++)
    mId[iq] = id[iq];

  for(unsigned gr=0; gr<GetGdim(); gr++) {   
    for(unsigned mm=0; mm<GetLinearMemberCount(gr); mm++) {
      GroupMember *member = GetSelMember(gr,mm);

      // Here all pointers should be non-zero I believe?;
      assert(member);

      member->InsertMatchCandidate(this);
      //printf("A: gr=%2d, mm=%2d -> %p hit sharing: %3d\n", gr, mm, hit, hit->mMatchCandidates.size());

      //member->SetBusyFlag();
    } //for mm
  } //for gr
} // MatchCandidate::ShapeItUpForInspection()

// ---------------------------------------------------------------------------------------

void MatchCandidate::AddMember(unsigned gr, GroupMember *member) {
  // Allocate yet another entry if needed;
  if (mMemberCount[gr] == mSelMembers[gr].size()) {
#ifdef _USE_ALL_MEMBERS_VECTOR_
    mAllMembers[gr].push_back(member);
#endif
    mSelMembers[gr].push_back(member);
  } else {
#ifdef _USE_ALL_MEMBERS_VECTOR_
    mAllMembers[gr][mMemberCount[gr]] = member;
#endif
    mSelMembers[gr][mMemberCount[gr]] = member;
  } //if

  mMemberCount[gr]++;
} // MatchCandidate::AddMember()

// ---------------------------------------------------------------------------------------

bool MatchCandidate::IsSubset(const HoughTree *tree, const MatchCandidate *reference) const
{
  for(int gr=0; gr<tree->GetGdim(); gr++) {
#if 1
    // Well, if this present candidate has MORE hits than the 
    // reference one, it can not be a subset, right?;
    if (GetAliveMemberCount(gr) > reference->GetAliveMemberCount(gr)) return false;

      // Current track candidate has LESS hits; need to check 
      // that they are ALL listed in the reference one, then (and
      // only then) it is a true subset;
#endif

#ifdef _USE_ALL_MEMBERS_VECTOR_
    // THINK: better use AllMembers[] here?;
#endif
    for(unsigned mm=0; mm<mMemberCount[gr]; mm++) {
      // Skip 0 pointers;
      if (!mSelMembers[gr][mm]) continue;

      bool found = false;
      
      for(unsigned qm=0; qm<reference->mMemberCount[gr]; qm++)
	if (mSelMembers[gr][mm] == reference->mSelMembers[gr][qm]) {
	  found = true;
	  break;
	} //for qm .. if
      
      if (!found) return false;
    } //for mm
  } //for gr

  // Ok, a subset in all hit groups -> return success;
  return true;
} // MatchCandidate::IsSubset()

// ---------------------------------------------------------------------------------------

__u64 MatchCandidate::Ambiguity() const
{
  __u64 ret = 1;

  // THINK: no active flag checks, etc?; FIXME: may want to check on non-zero 
  // member pointers (this is what will happen after the clean up pass);
  for(int gr=0; gr<GetGdim(); gr++) {
    __u64 multi = 0;

    for(unsigned mm=0; mm<mMemberCount[gr]; mm++) 
      if (mSelMembers[gr][mm])
	multi++;

    // THINK: this can probably happen after final smoother pass; anyway, 
    // want to account here only those nodes which have >1 hits;
    assert(multi);
    if (multi) ret *= multi;
  } //for gr

  return ret;
} // MatchCandidate::Ambiguity()

// ---------------------------------------------------------------------------------------

bool MatchCandidate::HasAmbiguousHits() const
{
  return (Ambiguity() != 1);
} // MatchCandidate::HasAmbiguousHits()

// ---------------------------------------------------------------------------------------

bool MatchCandidate::SiamGroupCandidate(unsigned minHitCount)
{
  unsigned hCounters[2] = {0, 0}, toggle = 0;

  for(int gr=0; gr<GetGdim(); gr++) {
#if _OLD_
    __u64 multi = 0;

    for(unsigned mm=0; mm<mMemberCount[gr]; mm++) 
      if (mMembers[gr][mm])
	multi++;
#else
    unsigned multi = GetAliveMemberCount(gr);
#endif

    // The idea is clear: if >1 hit in this plane, both potential 
    // tracks can be happy; otherwise alternate, this obviously gives 
    // highest possible hit count per track;
    if (multi == 1) {
      hCounters[toggle]++;
      toggle = (toggle+1)%2;
    } else if (multi >= 2) {
      hCounters[0]++;
      hCounters[1]++;
    } //if
  } //for gr

  //printf("Double counts: %2d %2d\n", hCounters[0], hCounters[1]);
  return (hCounters[0] >= minHitCount && hCounters[1] >= minHitCount);
} // MatchCandidate::SiamGroupCandidate()

// ---------------------------------------------------------------------------------------
#if _LATER_
void MatchCandidate::AssertHitActiveFlags(const HoughTree *tree)
{
  for(int gr=0; gr<tree->GetGdim(); gr++) 
    for(unsigned mm=0; mm<mMemberCount[gr]; mm++)
      mMembers[gr][mm]->SetActive(true);
} // MatchCandidate::AssertHitActiveFlags()
#endif
// ---------------------------------------------------------------------------------------

ClassImp(MatchCandidate)
