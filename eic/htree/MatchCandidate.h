//
// AYK (ayk@bnl.gov)
//
//  Match candidate (possible track) class;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <TObject.h>

#include <HoughNodeGroup.h>

#ifndef _MATCH_CANDIDATE_
#define _MATCH_CANDIDATE_

class HoughTree;

//#define _USE_ALL_MEMBERS_VECTOR_

class MatchCandidate: public TObject 
{
 public:
  MatchCandidate(const HoughTree *tree = 0);
  ~MatchCandidate() {
    delete [] mMemberCount;

    // FIXME: and the internals, please;
    //for(unsigned mm=0; mm<_members.size(); ++) {
    //} //for mm
#ifdef _USE_ALL_MEMBERS_VECTOR_
    delete [] mAllMembers;
#endif
    delete [] mSelMembers;

    delete [] mId;
  };

  virtual void ShapeItUpForInspection(const HoughTree *tree, const unsigned id[]);
  virtual bool IsReadyForFinalFit() const = 0;

  void ResetToTheVirginState();

  void ResetOkGroupCounter()                 { mOkGroupCounter = 0; };
  unsigned GetOkGroupCounter()         const { return mOkGroupCounter; };
  void IncrementOkGroupCounter(unsigned add) { mOkGroupCounter += add; }; 

  //void SetAccountedAsGoodTrack(bool status)  { mAccountedAsGoodTrack = status; };
  //bool IsAccountedAsGoodTrack()        const { return mAccountedAsGoodTrack; };

  void SetGroupedStatus(bool status)         { mGrouped = status; };
  bool IsGrouped()                     const { return mGrouped; };

  void SetInactive()                         { 
    for(unsigned gr=0; gr<GetGdim(); gr++)
      for(unsigned mm=0; mm<GetLinearMemberCount(gr); mm++) 
	ResetMemberPtr(gr, mm);

    mActive = false; 
  };
  bool IsActive()                      const { return mActive; };
  // It looks like there is no need to overcomplicate this stuff (used for 
  // printout purposes only and HoughTree knows better the dimension of this array);
  const unsigned *GetIdPtr()           const { return mId; };

  bool IsSubset(const HoughTree *tree, const MatchCandidate *reference) const;

  void ResetMemberPtr(unsigned gr, unsigned mm) { 
    GroupMember *member = GetSelMember(gr, mm); 

    if (member) {
      member->EraseMatchCandidate(this);

      mSelMembers[gr][mm] = 0; 
    } //if
  };
  void ResetMemberPtr(unsigned gr, GroupMember *member) { 
    for(unsigned mm=0; mm<mMemberCount[gr]; mm++)
      if (mSelMembers[gr][mm] == member)
	ResetMemberPtr(gr, mm);
  };
  void ResetMemberCount(unsigned gr)         { mMemberCount[gr] = 0; };

  // This is just to do loops correctly; there can be gaps after member removal, etc;
  unsigned GetLinearMemberCount(unsigned gr) const { return mMemberCount[gr]; };

  // These two routines do not distinguish between BUSY and non-BUSY members;
  unsigned GetAliveMemberCount(unsigned gr) const {
    unsigned count = 0;

    for(unsigned mm=0; mm<mMemberCount[gr]; mm++)
      if (mSelMembers[gr][mm])
	count++;

    return count;
  };
  // FIXME: this call usage indeed assumes at most one hit per plane in the final fit; 
  GroupMember *GetFirstAliveMember(unsigned gr) const {
    for(unsigned mm=0; mm<mMemberCount[gr]; mm++)
      if (mSelMembers[gr][mm]) 
	return mSelMembers[gr][mm];

    return 0;
  };
  unsigned GetAliveGroupCount() const {
    unsigned count = 0;

    for(unsigned gr=0; gr<GetGdim(); gr++)
      if (GetAliveMemberCount(gr))
	count++;

    return count;
  };
  unsigned GetAliveMemberCount() const {
    unsigned count = 0;

    for(unsigned gr=0; gr<GetGdim(); gr++)
      count += GetAliveMemberCount(gr);

    return count;
  };

  void AddMember(unsigned gr, GroupMember *member);
  // FIXME: range check;
  GroupMember *GetSelMember(unsigned gr, unsigned mm) { return mSelMembers[gr][mm]; };

  virtual double GetTrackQualityParameter() const = 0;

  bool HasAmbiguousHits() const;
  __u64 Ambiguity() const;
  bool SiamGroupCandidate(unsigned minHitCount);

  // FIXME: perhaps do it better later;
  unsigned GetGdim()                   const { return mGdim; };

 private:
  // May be reset to 0 during purging (is a subset of some other candidate);
  bool mActive;                        //!

  // If 'true', this candidate is participating in some group already;
  bool mGrouped;                       //!

  // Is selected as a good track candidate already;
  //bool mAccountedAsGoodTrack;          //!

  // Just copy over HoughTree value; FIXME: do it better later;
  unsigned mGdim;                      //!

  // Just cell ndim-index at the highest level for now; it is more convenient 
  // to store it like this instead of the single __u32 1D-milti-index;
  // does not cost much RAM anyway; perhaps account weights later as well?;
  unsigned *mId;                       //!

  // THINK: during tree search procedure (depending on the road width) it can
  // happen, that there are (much) more hit candidates, than actually allowed
  // to be left during the final fit (typically 1 per plane); for now prefer to 
  // maintain two vectors, which are populated identically during tree search, 
  // but mSelMembers[] gets purged during ambiguity resolution pass; the idea
  // of having all original hits intact in mAllMembers[] vector is to allow 
  // more efficient conflict resolution later in case hit borrowing is allowed 
  // (basically want to be able to try hit replacement in some tracks as long 
  // as chi^2 CCDF value can be maintained above threshold); FIXME: once debugging
  // is over may want to incorporate more efficient storage scheme; 
  unsigned *mMemberCount;                 //!
#ifdef _USE_ALL_MEMBERS_VECTOR_
  std::vector<GroupMember*> *mAllMembers; //!
#endif
  std::vector<GroupMember*> *mSelMembers; //!

  // Basically the number of fired planes, no matter BUSY or new ones;
  unsigned mOkGroupCounter;               //!

  ClassDef(MatchCandidate,3);
};

#endif
