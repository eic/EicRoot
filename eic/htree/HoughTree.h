//
// AYK (ayk@bnl.gov)
//
//  Hough transform track finder;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <map>
#include <cstdio>

#include <ResolutionLevel.h>
#include <HoughNodeGroup.h>
#include <HoughCell.h>
#include <MatchCandidateGroup.h>

#ifndef _HOUGH_TREE_
#define _HOUGH_TREE_

class HoughDimension {
 public:
 HoughDimension(const char *name, double min, double max): mMin(min), mMax(max) {
    // Sanity check, please;
    if (!name || min >= max) {
      printf("\n   HoughDimension::HoughDimension(): illegal input!\n\n");
      throw;
    } //if
    mName = strdup(name); assert(mName);
    
    printf("%s -> %7.2f .. %7.2f\n", mName, mMin, mMax);
  };

  const char *GetName() const { return mName; };
  double GetMin()       const { return mMin; };
  double GetMax()       const { return mMax; };

 private:
  // For printouts and book-keeping purposes only;
  char *mName;

  // Parameter range;
  double mMin, mMax;
};

// May want to consider __u64 if ever come to a situation when multi-index composed 
// out of all the dimensions becomes larger than __u32 type; well, if 64-bits are 
// not enough, will have to change binary search code;
typedef __u32 t_btree_index;
// So depth will be counted in the range [0..31] for __u32 indices;
#define _MAX_BTREE_INDEX_DEPTH_  ((sizeof(t_btree_index) << 3)-1)

enum TrackCandidatePrintoutFlag {_ALL_CANDIDATES_, _ACTIVE_CANDIDATES_};

class HoughTree
{
 public:
  HoughTree();
  // FIXME: populate with something useful, please;
  ~HoughTree() {};

  int AddDimension(const char *name, double min, double max);
  int AddResolutionLevel(const unsigned div[]);

  virtual HoughNodeGroup *AllocateNodeGroup(unsigned id) { return new HoughNodeGroup(id); };

#if _LATER_
  HoughNodeGroup *AddNodeGroup(void *ptr,                                     double min,         double max, 
			       double gra,   SpaceGridType gridType);
  HoughNodeGroup *AddNodeGroup(void *ptr,                unsigned cdim, const double min[], const double max[], 
			       const double gra[], SpaceGridType gridType);
#endif
  HoughNodeGroup *AddNodeGroup(/*void *ptr,*/ unsigned id, unsigned cdim, const double min[], const double max[],
			       const double gra[]);
  unsigned GetGdim() const { return mGroups.size(); };

  unsigned LaunchPatternFinder();

  void SetVerbosityLevel(unsigned level) { mVerbosityLevel = level; };
  unsigned GetVerbosityLevel() const { return mVerbosityLevel; };

  unsigned GetDdim() const { return mDimensions.size(); };
  const HoughDimension *GetDimension(unsigned id) const { 
    return id < mDimensions.size() ? &mDimensions[id] : 0;
  };

  int SetBlindCellDecisionLevel(unsigned level) { 
    if (level >= GetLdim()) return -1;

    mBlindCellDecisionLevel = level; 
    return 0;
  };
  int SetOkHitCounterLimits(unsigned min, unsigned max) {
    // Here and below need to count node counter or such rather than just mGroups.size();
#if _LATER_
    if (min > max || max > mGroups.size()) return -1;
#endif

    mMinOkHitCounter = min; mMaxOkHitCounter = max;
    return 0;
  };
  int SetBorrowedHitCounterLimit(unsigned max) {
    if (max > mGroups.size()) return -1;
    
    mBorrowedHitCounterMax = max;
    return 0;
  };
  int SetBorrowedPlusMissingHitCounterLimit(unsigned max) {
    if (max > mGroups.size()) return -1;
    
    mBorrowedPlusMissingHitCounterMax = max;
    return 0;
  };

  unsigned GetGroupCount() const { return mGroups.size(); };
  HoughNodeGroup *GetGroup(unsigned gr) const { return mGroups[gr]; };

  unsigned GetLdim() const { return mResolutionLevels.size(); };
  ResolutionLevel *GetLevel(unsigned lv) const { 
    return (lv < mResolutionLevels.size() ? mResolutionLevels[lv] : 0);
  };

  // Just to arrange a loop;
  unsigned GetLinearMatchCandidateCount() const { return mMatchCandidateCount; };
  MatchCandidate *GetMatchCandidate(unsigned tc) { 
    return (tc < mMatchCandidateCount ? mMatchCandidates[tc] : 0);
  };

  int AllocateLookUpTable();

  void SetFastTreeSearchMode(unsigned qualityItrNum) { 
    mTrackQualityIterationNum = qualityItrNum; assert(qualityItrNum);
    mFastTreeSearchMode = true; 
  };

 protected:
  //void PrintTrackCandidateArray(unsigned from, unsigned to, TrackCandidatePrintoutFlag flag) const;

  virtual MatchCandidate *AllocateMatchCandidate() = 0;
  //virtual void SeparateSiamTracks(MatchCandidate *match, unsigned minHitCount, 
  //				  std::vector<MatchCandidate*> *newMatches) = 0;
  //virtual void ResolveAmbiguities(MatchCandidate *match) = 0;
  virtual void ResolveAmbiguitiesNg(MatchCandidate *match) = 0;
  virtual void FinalFit(MatchCandidate *match) = 0;

  // Registering detector groups to be AND'ed during calculations; configuration 
  // is assumed to be fixed upon start-up; basically *planes* in the track finder context;
  std::vector<HoughNodeGroup*> mGroups;

  // Track finder loop will probably be arranged between max and min values;
  unsigned mCurrMinOkHitCounter;

 private:
  int AddResolutionLevelCore(const unsigned div[]);

  unsigned PurgeDuplicateTracks();

  // Do not want to make respective HoughCell constructor; InitializeCell() iteratively
  // calls itself to allocate daughter cells and I sort of feel uncomfortable to do
  // this in the constructor;
  HoughCell *GetInitializedCell(unsigned lv, const unsigned id[]);
  unsigned CheckCell(unsigned lv, const unsigned id[], HoughCell **pcell, 
		     const std::vector<GroupMember*> members[]);

  bool IsSubset(MatchCandidate *match) const;

  bool IsBusy(const GroupMember *member) const {
    return (mFastTreeSearchMode ? member->IsBooked() : member->IsBusy());
  };

  virtual void SetupTrackQualityIteration(unsigned itr) = 0;

  unsigned GetUsefulGroupCount(const HoughCell *cell) const {
    unsigned counter = 0;

    for(unsigned gr=0; gr<GetGdim(); gr++)
      if (cell->From(gr) != __OUT_OF_RANGE_BIT_)
	counter++;

    return counter;
  };

  MatchCandidate *GetCurrentMatchBufferPtr() {
    if (mMatchCandidateCount == mMatchCandidates.size()) 
      mMatchCandidates.push_back(AllocateMatchCandidate());

    MatchCandidate *match = mMatchCandidates[mMatchCandidateCount];

    match->ResetToTheVirginState();

    return match;
  };

  // Function which calculates expected gnum-dimensioned {wire} 
  // hit index array id[] for a given ddim-dimensioned parameter vector par[];
  virtual void MappingCall(const double par[], t_hough_range id[]) = 0;

  // Some allocation should be done after all the add_hough_...() 
  // configuration routines succeeded;
  bool mInitialized;

  // New dimensions can be added until division strategy is selected;
  bool mGeometryLocked;

  // Well, just propagate from XML config for now;
  unsigned mVerbosityLevel;

  // Min number of hits in a given branch in the track finder context; 
  // max number of hits has a separate sense; it looks one can hardly 
  // estimate the most efficient branch direction (select for instance 
  // 6-hit branches first, then investigate 5-hit branches, etc, because
  // cell calculation takes most of the CPU already; also it is not 
  // guaranteed that track candidate with max number of hits will be 
  // found earlier than any smaller-number=of-hits combination because
  // branching decision on lower level could be wrong already (say I 
  // have 9 planes and a minimum of 7 hits; there are 2 branches, with 
  // 9 and 8 hit planes respectively; I select 9-fold one, but it 
  // effectively had noise hits in 2 planes, so track candidate will 
  // at the end have 7 hits; the 8-fold branch had all hits true, and 
  // will at the end have 8-hit track candidate; apparently a better -
  // 8-fold track - will be found later tan 7-fold one); so for now prefer
  // to arrange a loop from max to min, and suppress track candidates 
  // which have same hits as the ones in already found tracks, up to the
  // missing ones; track fitting will take care to check whether there
  // were outlier hits or not; do not want to take hroot->gnum as max,
  // because entering with this limit is not necessarily the most 
  // efficient strategy (if there are either too many planes or plane 
  // efficiency is too low);
  unsigned mMinOkHitCounter, mMaxOkHitCounter;

  // For now allocation is perhaps not too much efficient (pointers); 
  // the convention then is: if a certain daughter cell is NULL, it 
  // has NOT been allocated yet; if pointer is mBlindCell, then cell
  // is a "blind end", so it has no overlap with real tracks in the
  // suggested parameter space rectangle, 
  // including higher resolution levels; since the parameter 
  // space has in general larger volume than allowed by chamber
  // acceptance, the lower resolution cell(s) may have all edges out 
  // of acceptance (for sure can be so for 0-th level, where useful
  // parameter space can be "enclosed" in a larger volume); then it is 
  // more safe to check their daughters up to the 
  // mBlindCellDecisionLevel, and only if they all have no
  // overlap with acceptance, declare them all (and parent cell) as "blind";
  unsigned mBlindCellDecisionLevel;
  HoughCell *mBlindCell;

  // When calculation acceptance of lower level cells increase resolution
  // up to this level only, and take largest estimate; THINK: highest resolution 
  // level for now per default and no method to change this;
  unsigned mCellAcceptanceDecisionLevel;

  // During track finder iterations "good" track mark their hits as "sort of 
  // used"; new track candidates can borrow few such hits if needed; "suspicious"
  // node is such where hit is either borrowed or missing at all;
  unsigned mBorrowedHitCounterMax, mBorrowedPlusMissingHitCounterMax;

  // Number of parameter cube dimensions; here and below prefer arrays
  // instead of linked lists; mSingleCellEdgeNum=2**Dimensions.size(): number of 
  // single cell edges (is reset to 1 in the constructor and will be doubled with 
  // every new dimension declared by the AddDimension() call;
  int mSingleCellEdgeNum;
  std::vector<HoughDimension> mDimensions;

  // Hit counter limits may be assigned plane-group specific; otherwise 
  // they will be global for group #0;
  std::map<unsigned,unsigned> mGlimits, mGcounters;

  // Number of resolution levels; their description;
  std::vector<ResolutionLevel*> mResolutionLevels;

  // The actual Hough cell tree;
  HoughCell *mCellTree;

  // Binary tree of 3D calculation lookup table; 
  // FIXME: a less economic allocation scheme is used for now (?);
  t_hough_range **mBinaryTree;

 protected:
  // HoughTree::IsBusy() call is based on member->IsBooked() rather than 
  // member->IsBusy(); so track candidates grab hits right during tree search;
  bool mFastTreeSearchMode;
  unsigned mTrackQualityIterationNum;

  // Match (track) candidates at the highest resolution level; prefer to 
  // use std::vector as a convenient way to arrange permanent storage (do not 
  // call clear() at the end of every event), but use another running variable 
  // which indicates current number of allocated track candidates in this event;
  unsigned mMatchCandidateCount;
  std::vector<MatchCandidate*> mMatchCandidates;
};

#endif
