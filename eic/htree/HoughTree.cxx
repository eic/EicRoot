//
// AYK (ayk@bnl.gov)
//
//  Hough transform track finder;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <HoughTree.h>

// ---------------------------------------------------------------------------------------

HoughTree::HoughTree(): 
  mInitialized(false), 
  mVerbosityLevel(0),
  mBorrowedHitCounterMax(0),
  mBorrowedPlusMissingHitCounterMax(0),
  mGeometryLocked(false),
  mMinOkHitCounter(0), 
  mMaxOkHitCounter(0),
  mBlindCellDecisionLevel(0),
  mCellAcceptanceDecisionLevel(0),
  mCellTree(0),
  mBinaryTree(0),
  mCurrMinOkHitCounter(0),
  mMatchCandidateCount(0),
  mSingleCellEdgeNum(1),
  mFastTreeSearchMode(false),
  mTrackQualityIterationNum(1)
{
  // Allocate blind cell pointer;
  mBlindCell = new HoughCell(this);
} // HoughTree::HoughTree()

// ---------------------------------------------------------------------------------------

int HoughTree::AddDimension(const char *name, double min, double max)
{
  // Some sanity check is needed;
  if (mGeometryLocked) return -1;

  mDimensions.push_back(HoughDimension(name, min, max));

  mSingleCellEdgeNum *= 2;

  return 0;
} // HoughTree::AddDimension()

// ---------------------------------------------------------------------------------------

int HoughTree::AddResolutionLevelCore(const unsigned div[])
{
  // Well, prefer do all the work in this constructor; passing 'this' looks
  // ugly, but is the easiest way to hide the ResolutionLevel internals;
  // follows below in this call;
  mResolutionLevels.push_back(new ResolutionLevel(this, div));

  // Yes, current highest level per default;
  mCellAcceptanceDecisionLevel = GetLdim() - 1;

  return 0;
} // HoughTree::AddResolutionLevelCore()

// ---------------------------------------------------------------------------------------

static unsigned bits(__u64 value)
{
  unsigned count = 0;

  // Subtract 1 and shift to the right till get zero value;
  for(value--; value; count++) 
    value >>= 1;

  return count;
} // bits()

// ---------------------------------------------------------------------------------------

int HoughTree::AllocateLookUpTable()
{
  t_btree_index multi = 1;
  ResolutionLevel *high = GetLevel(GetLdim() - 1);

  for(int iq=0; iq<mDimensions.size(); iq++) {
    // Put a crude overflow check; NB: '+1' because need number of cell 
    // edges, not cell centers;
    if (bits(multi) + bits(high->GetTotalDivisionNumber(iq)+1) > _MAX_BTREE_INDEX_DEPTH_ + 1) {
      printf("HoughTree::AllocateLookUpTable(): t_btree_index width too small for this granularity!\n");
      return -1;
    } //if

    multi *= high->GetTotalDivisionNumber(iq) + 1;
  } //for iq

  // NB: even on the 0-th level (no split) there are 2^ndim cube edges;
  // FIXME: a less efficient temporary hack (just a plain pointer array);
  mBinaryTree = new t_hough_range*[multi];
  memset(mBinaryTree, 0x00, multi*sizeof(t_hough_range*));

  return 0;
} // HoughTree::AllocateLookUpTable()

// ---------------------------------------------------------------------------------------

int HoughTree::AddResolutionLevel(const unsigned div[])
{
  unsigned ndim = mDimensions.size();

  if (!ndim || !div || mInitialized) return -1;

  // If the very first level is not defined yet, do it; just declare 
  // "no division" for all parameters;
  if (!GetLdim()) {
    unsigned pdim[ndim];

    for(int iq=0; iq<ndim; iq++)
      pdim[iq] = 1;

    if (AddResolutionLevelCore(pdim)) return -1;
  } //if

  // Lock parameter space geometry;
  mGeometryLocked = true;

  // Allocate next level and return;
  return AddResolutionLevelCore(div);
} // HoughTree::AddResolutionLevel()

// ---------------------------------------------------------------------------------------

HoughNodeGroup *HoughTree::AddNodeGroup(unsigned id, unsigned cdim, const double min[], 
					const double max[], const double gra[])
{
  // FIXME: move this check to HoughNodeGroup constructor;
  if (mInitialized || !cdim || !min || !max || !gra) return 0;

  HoughNodeGroup *ngroup = AllocateNodeGroup(id);
  ngroup->ConfigureCoordinateDescriptors(cdim, min, max, gra);
  mGroups.push_back(ngroup);

  // Just allocate these std::map entries once forever;
  mGlimits[id] = mGcounters[id] = 0;

  return ngroup;
} // HoughTree::AddNodeGroup()
#if _LATER_
HoughNodeGroup *HoughTree::AddNodeGroup(void *ptr, double min, double max, double gra, SpaceGridType gridType)
{
  //if (!wdim) return -1;

  const double qmin[1] = {min}, qmax[1] = {max}, qgra[1] = {gra};

  return AddNodeGroup(ptr, 0, 1, qmin, qmax, qgra, gridType);
} // HoughTree::AddNodeGroup()

HoughNodeGroup *HoughTree::AddNodeGroup(void *ptr, unsigned cdim, const double min[], const double max[], 
			    const double gra[], SpaceGridType gridType)
{
  return AddNodeGroup(ptr, 0, cdim, min, max, gra, gridType);
} // HoughTree::AddNodeGroup()
#endif
// ---------------------------------------------------------------------------------------

static void mk_prefix(unsigned lv, char arr[])
{  
  sprintf(arr, "%2d", lv);
  for(unsigned iq=0; iq<lv; iq++)
    sprintf(arr + strlen(arr), "%s", "  ");
} // mk_prefix()

// ---------------------------------------------------------------------------------------

HoughCell *HoughTree::GetInitializedCell(unsigned lv, const unsigned id[]) 
{
  ResolutionLevel *level = GetLevel(lv);
  HoughCell *cell = new HoughCell(this);

#if _CHECK_
  // For now the logic is: 1) calculate track parameterizations for
  // all 2**GetDdim() cell parameter rectangle edges, 2) calculate crossing
  // of these tracks with all the GetGdim() registering planes; if all 
  // crossing points are in the acceptance, just assign range variables;
  // if at least one {edge,plane} combination falls out of acceptance, 
  // go to higher resolution levels up to 
  // cell_acceptance_decision_level, perform calculations there
  // and take range for this given cell as an OR of all its daughter
  // cells; NB: at the cell_acceptance_decision_level and higher
  // levels range for a given plane will just be the OR of ALL EDGES; 
  // this clearly exagerates acceptance a bit, but one has to consider 
  // using fine granularity at >=cell_acceptance_decision_level
  // resolution levels anyway;
#endif
  {
    int daughter_calculation_needed = 0;
    // gdim, ddim: save typing;
    unsigned ok_group_sum = 0, gdim = GetGdim(), ddim = GetDdim();
    // Reset all ok[] entries to false first;
    bool ok[gdim]; memset(ok, 0x00, sizeof(ok));

    // Loop through all cell rectangle edges;
    for(int ip=0; ip<mSingleCellEdgeNum; ip++) {
      unsigned lr[ddim];

      // Calculate individual ddim-indices (basically left-right); order does not 
      // really matter here?; reversed order below just for a better printout?;
      for(int iq=0; iq<ddim; iq++)
	lr[ddim-iq-1] = (ip >> iq) & 0x1;

      {
	double par[ddim];
	t_hough_range range[gdim], *rptr = range;
	t_btree_index idx = 0;

	// Try to get stored value first; calculate index;
	{
	  ResolutionLevel *high = GetLevel(GetLdim() - 1);

	  // Same logic as in add_hough_resolution_level_core(); NB: dimensions
	  // are +1 here (need cell edges); 
	  {
	    // id(a,b,c,d) = a*d2*d3*d4 + b*d3*d4 + c*d4 + d; sdim[] is just array of
	    // coefficients by {a,b,c,d}; FIXME: unify with earlier code!;
	    unsigned sdim[ddim];

	    for(int iq=ddim-1; iq>=0; iq--)
	      sdim[iq] = iq == ddim-1 ? 1 : (high->GetTotalDivisionNumber(iq+1)+1)*sdim[iq+1];

	    // Calculate cell edge multi-index at the highest resolution level;
	    for(int iq=0; iq<ddim; iq++)
	      idx += sdim[iq]*(id[iq]+lr[iq])*
		(high->GetTotalDivisionNumber(iq)/level->GetTotalDivisionNumber(iq));
	  }
	}

	// If lookup entry exists, use it; unless need to calculate offsets
	// (because otherwise average values will be screwed up);
	if (mBinaryTree[idx])
	  rptr = mBinaryTree[idx];
	else {
	  // Fill out parameter vector for this rectangle edge;
	  for(int iq=0; iq<ddim; iq++)
	    par[iq] = mDimensions[iq].GetMin() + (id[iq]+lr[iq])*level->GetCellSize(iq);
      
	  // Call user-defined routine which would fill range[] array;
	  MappingCall(par, range);

	  // Allocate the lookup entry;
	  {
	    t_hough_range *ptr = mBinaryTree[idx] = new t_hough_range[gdim];

	    // Copy over the result for further usage;
	    for(int iq=0; iq<gdim; iq++)
	      ptr[iq] = range[iq];
	  }
	} //if

	// Check "out of range" condition; fill out of range arrays; 
	for(int gr=0; gr<gdim; gr++) {
	  HoughNodeGroup *group = mGroups[gr];

	  // NB: for now do not break if any of the return indices is out 
	  // of range (well, this is inefficient, fix later);
	  if (rptr[gr] & __OUT_OF_RANGE_BIT_) {
	    if (lv < mCellAcceptanceDecisionLevel) { 
	      daughter_calculation_needed = 1;

	      // THINK: this fallout will affect offsets calculations; perhaps 
	      // just check "offsets" pointer here?; think & return back later;
#if _THINK_
	      // Well, if at least one Ok edge found for each of the groups,
	      // and daughter calculation will be required anyway, makes no
	      // sense to continue calculations on this level, right?;
	      if (ok_group_sum == gdim) goto _fall_out;
#endif

	      // So that cell->range[gr] will not be affected;
	      continue;
	    } //if
	  } 
	  else {
	    // Account once for each group;
	    if (!ok[gr]) ok_group_sum++;

	    // Well, at least one edge was Ok for this group;
	    ok[gr] = true;
	  } //if

	  // Update ranges; for cell_acceptance_decision_level and higher this 
	  // will be done no matter was this edge out of acceptance or not;
	  cell->UpdateRanges(this, rptr);
	} //for gr
      }
    } //for ip

_fall_out:
    // Well, at the mBlindCellDecisionLevel I still want to make sure that 
    // at least one edge was fine for a given group (NB: on this level cell ranges 
    // are assigned anyway, no matter in or out of acceptance edges were);
    cell->ResetRanges(this, ok);

    // Use absolutely min number of hits as a condition here (apparently 
    // this initialization is the same for the whole program life time, 
    // no matter with which mCurrMinOkHitCounter I'm running right now);
    if (GetUsefulGroupCount(cell) < mMinOkHitCounter && lv >= mBlindCellDecisionLevel)
      goto _cleanup;
    
    // Ok, if daughter calculation is requested (which basically means part
    // of the parameter rectangle edges were out of acceptance for at least 
    // one registering plane), allocate daughter memory and calculate these
    // cells recursively;
    if (daughter_calculation_needed) {
      // Unify with the other similar code later;
      unsigned idq[ddim];

      // For sure I can enter each cell initialization routine only once; so 
      // if daughter calculation is needed now, I have not touched any of these
      // daughters yet;
      assert(!cell->DaughtersArrayAllocated());

      // Allocate daughter array with NULL pointers; NB: "lv != lnum-1" condition 
      // has been checked when assigning "daughter_calculation_needed" flag;
      ResolutionLevel *next = GetLevel(lv+1);
      cell->AllocateDaughterCells(next->GetDaughterCellNumber());

      // Well, make it clean; so reset to _OUT_OF_RANGE_BIT_ first;
      cell->ResetRanges(this);  

      // Now loop through all the daughters and use their from[]/to[] ranges;
      for(unsigned iq=0; iq<next->GetDaughterCellNumber(); iq++) {
	for(int ip=0; ip<ddim; ip++)
	  idq[ip] = id[ip]*next->GetParameterSplitFactor(ip) + next->Remap(iq,ip);

	{
	  // Yes, initialization is for sure needed here (handle this cell
	  // for the first time, sure);
	  HoughCell *dcell = GetInitializedCell(lv+1, idq);
	  cell->SetDaughter(iq, dcell);

	  // Update ranges on parent cell using daughter cell ones;
	  if (dcell != mBlindCell) {
	    cell->UpdateRanges(this, dcell->From());
	    cell->UpdateRanges(this, dcell->To());
	  } //if
	}
      } //for iq
    } //if
  }

  // Now, after all tricks including daughter cell range calculations, count 
  // useful group number on this (parent) cell; FIXME: yes, perhaps once again; 
  if (GetUsefulGroupCount(cell) >= mMinOkHitCounter) return cell;

_cleanup:
  delete cell;

  return mBlindCell;
} // HoughTree::GetInitializedCell()

// ---------------------------------------------------------------------------------------

bool HoughTree::IsSubset(MatchCandidate *match) const
{
  std::map<MatchCandidate*, unsigned> competitors;

  // Clearly track candidate can only be a subset of some other track 
  // it shares part of its member hits; this approach is clearly more efficient
  // than arranging a loop through ALL other track candidates (even if applying 
  // some neighborehood restricitions);
  for(int gr=0; gr<mGroups.size(); gr++) {
    for(unsigned mm=0; mm<match->GetLinearMemberCount(gr); mm++) {
      GroupMember *member = match->GetSelMember(gr, mm); 

      if (!member) continue;

      // Loop through all track candidates which claim to own this member hit;
      for(std::set<MatchCandidate*>::iterator it=member->Begin(); it != member->End(); it++) {
	MatchCandidate *qmatch = *it;

	if (qmatch != match) competitors[qmatch]++;
      } //for it
    } //for mm
  } //for gr

  // Loop through all the competitors and compare overall hit counts;
  unsigned myHitCount = match->GetAliveMemberCount();
  for(std::map<MatchCandidate*, unsigned>::iterator it=competitors.begin(); 
      it != competitors.end(); it++) 
    if (it->second >= myHitCount)
      return true;

  return false;
} // HoughTree::IsSubset()

// ---------------------------------------------------------------------------------------

#define _NEW_SUBSET_LOGIC_

unsigned HoughTree::CheckCell(unsigned lv, const unsigned id[], HoughCell **pcell, 
			      const std::vector<GroupMember*> members[])
{
#ifdef __APPLE__
  assert(0);
#else
  // So this trick does not work with Clang; figure a workaround later if even want to run 
  // this code under Mac OS;
  std::vector<GroupMember*> qmembers[mGroups.size()];

  ResolutionLevel *level = GetLevel(lv);

  // Well, if cell has not been calculated yet, do it; allocate the frame,
  // call overlap calculation routine (track finder context: calculate 
  // track parameters at the parameter rectangle edges and assign overlap 
  // ranges with the registering planes); NB: this may require daughter 
  // cell initialization/calculation (see GetInitializedCell() call details)!;
  if (!*pcell) *pcell = GetInitializedCell(lv, id);

  {
    unsigned ok_hit_counter = 0, ok_added_group_counter = 0;
    HoughCell *cell = *pcell;
    // NB: match_candidate_num will NOT be changed between here 
    // and below track candidate filling routine (just because recursion 
    // is not involved inbetween); is actually used on track finder decision 
    // level only; assign always though (make compiler happy);
    MatchCandidate *match = GetCurrentMatchBufferPtr();

    for(std::map<unsigned,unsigned>::iterator it=mGcounters.begin(); 
	it!=mGcounters.end(); it++) 
      it->second = 0;

    // Ok, if after initialization cell pointer was released and 
    // assigned back to a "blind" one, cell has no overlap with real
    // tracks in *ANY* of the registering planes; nothing to talk about,
    // just return non-error code;
    if (cell == mBlindCell) return 0;

    // Ok, otherwise this cell in parameter space has overlap with 
    // real tracks; check how many hits do I actually have here;
    // loop through all the groups (registering planes);
    for(int gr=0; gr<mGroups.size(); gr++) {
      int ok_group = 0, ok_added_group = 0;
      HoughNodeGroup *group = mGroups[gr];

      // NB: some plane(s) may have no overlap with tracks from this 
      // parameter space cell (out of acceptance); check on that;
      if (cell->From(gr) != __OUT_OF_RANGE_BIT_)
	// Calculate OR of all its members on this level;
	for(unsigned mm=0; mm<members[gr].size(); mm++) {
	  GroupMember *member = members[gr][mm];

	  // Sort of a CPU-saving hack; see logic in 'if (group->Overlap())';
	  if (IsBusy(member) && !mBorrowedHitCounterMax) continue;

	  t_hough_range gfrom = member->From();
	  t_hough_range gto   = member->To();
	  // THINK: by construction cell->From(gr)<=cell->To(gr) in all components (see 
	  // HoughCell::UpdateRanges() call); then this range expansion makes sense (From() 
	  // components go down, To() components go up and they are all still guaranteed 
	  // to be properly ordered); see assert() in group->Overlap() call though;
	  unsigned smearing = group->GetPhaseSpaceSmearing();
	  // Check if smearing=0 (save some CPU time; not much though);
	  t_hough_range gmin  = smearing ? group->OffsetThisValue(cell->From(gr), -(int)smearing) : cell->From(gr);
	  t_hough_range gmax  = smearing ? group->OffsetThisValue(cell->To  (gr),       smearing) : cell->To  (gr);

	  // NB: do not break once one Ok hit in a given group found, since
	  // need an overall number of participating hits;
	  // FIXME: may be just pass 4 parameters?;
	  if (group->Overlap(std::pair<t_hough_range,t_hough_range>(gfrom, gto), 
			     std::pair<t_hough_range,t_hough_range>(gmin,  gmax))) 		       
	  {
	    ok_group = 1;
	    ok_hit_counter++;

	    // Yes, count groups which have "new" (not BUSY) hits separately;
	    if (!IsBusy(member)) ok_added_group = 1;
	    
	    // Same hack -> see comment below;
	    if (!IsBusy(member) || mBorrowedHitCounterMax)
	      qmembers[gr].push_back(member);

	    // Yes, needed only on the highest level (or whatever track 
	    // finder decision level -> fix this place then!);
	    if (lv == GetLdim() - 1) {
	      // A small hack: if borrowed hits are not allowed at all, does not make 
	      // any sense to add such a member; FIXME: should do ambiguity resolution
	      // correctly in case of non-zero mBorrowedHitCounterMax (so that 
	      // non-borrowed hit count does not go below limit -> part of the hits 
	      // should just become immune);
	      if (!IsBusy(member) || mBorrowedHitCounterMax)
		match->AddMember(gr, member);
	    } //if
	  } //if
	} //if..for mm

      // FIXME: arrange an early-drop-out condition here (once too many plane 
      // groups have no hits, there is no reason to continue);
      //if (!ok_group) return 0;

      match->IncrementOkGroupCounter(ok_group);
      ok_added_group_counter  += ok_added_group;
      // And also increment special node group counters separately 
      // (feature not used for now in STAR FWD case);
      if (ok_group) mGcounters[group->GetGroupId()]++;

      if (mVerbosityLevel >= 5 && !ok_group) {
	char str[1000];
	mk_prefix(lv, str);

	printf("%s   -       gr=%d [%4d .. %4d]\n", str, gr, cell->From(gr), cell->To(gr));
	if (cell->From(gr) != __OUT_OF_RANGE_BIT_)
	  for(unsigned mm=0; mm<group->GetMemberCount(); mm++) {
	    GroupMember *member = group->GetMember(mm);

	    // THINK: may also want to mark BUSY members here?;

	    t_hough_range gfrom = member->From(); 
	    
	    // Should not I print out 'gto' as well?;
	    printf("%s   -*      %4d\n", str, gfrom);
	  } //for mm .. if
      }
#if 0
      else
      {
	char str[1000];
	mk_prefix(lv, str);

	printf("%s   +       gr=%d [%4d .. %4d]\n", str, gr, cell->from[gr], cell->to[gr]);
      }
#endif
    } //for gr

    if (mVerbosityLevel >= 5) {
      char str[1000];

      // Well, could be done better; let it be;
      mk_prefix(lv, str);

      printf("%s  --> check result: %d\n", str, match->GetOkGroupCounter());
      fflush(stdout);
    } //if

    // Ok, now if hit count is smaller than threshold (or perhaps have 
    // more sophisticated logic later?), just return (branch is dead);
    if (match->GetOkGroupCounter() < mCurrMinOkHitCounter) return 0;

    for(std::map<unsigned,unsigned>::iterator it=mGlimits.begin(); 
	it!=mGlimits.end(); it++) 
      if (it->second && mGcounters[it->first] < it->second)
	return 0;

    {
      unsigned borrowed_hit_counter = 
	match->GetOkGroupCounter() - ok_added_group_counter;

      // And also number of added "new" fired planes should be sufficiently high;
      // allow to borrow only (very) few already used (BUSY) hits;
      if (borrowed_hit_counter > mBorrowedHitCounterMax) return 0;

      // And the sum of borrowed and missing should not be too high;
      if (borrowed_hit_counter + (mMaxOkHitCounter - match->GetOkGroupCounter()) > 
	  mBorrowedPlusMissingHitCounterMax)
	return 0;
    }
    
    // If I'm at the highest resolution level, store track candidate;
    if (lv == GetLdim() - 1) {
#ifndef _NEW_SUBSET_LOGIC_
      int duplicate = 0;

      // Check against all previous candidates; indeed if member hits are 
      // the same (even that match->mId[] differ), makes no sense to duplicate;
      for(unsigned tc=0; tc<mMatchCandidateCount; tc++) {
	MatchCandidate *reference = mMatchCandidates[tc];

	if (match->IsSubset(this, reference))	{
	  // Ok, same set found --> set "duplicate" flag & fall out;
	  duplicate = 1;
	  goto _fall_out;
	} //if
      } //for tc

      //bool qq = IsSubset(match);
      //assert((qq && duplicate) || (!qq && !duplicate));

_fall_out:
      if (!duplicate) {
#else
	if (!IsSubset(match)) {
#endif
	// Fine, then assign a new track candidate entry;
	match->ShapeItUpForInspection(this, id);

#if 1
	if (mFastTreeSearchMode) {
	  while (!match->IsReadyForFinalFit()) 
	    ResolveAmbiguitiesNg(match);

	  FinalFit(match);  

	  if (match->IsActive()) {
	    for(unsigned gr=0; gr<mGroups.size(); gr++) {
	      // Indeed by this point track has given away all other hits but single
	      // per plane group; FIXME: allow >1 hit per plane group later;
	      assert(match->GetAliveMemberCount(gr) <= 1);
		
	      GroupMember *member = match->GetFirstAliveMember(gr);
	      // THINK: can happen on lower 'min' loop levels?;
	      if (!member) continue;

	      member->SetBusyFlag();
	    } //for gr

	    mMatchCandidateCount++;
	  } //if
	}
	else
#endif
	  mMatchCandidateCount++;
      } //if

      // Yes, return overall number of participating hits; calling routine
      // (like lower resolution cell handler) should compare this number to 
      // its hit counter and if they match will NOT try to search for tracks
      // in other daughter cells; THINK: no matter, was this a new track or not?;
      return ok_hit_counter;
    } 
    else {
      // Otherwise call this routine recursively at higher resolution 
      // level(s), for all this cell daughters' cells (except for the "blind"
      // ones, of course);
      unsigned idq[mDimensions.size()];

      // Allocate daughter array with NULL pointers if not done yet; 
      // if memory allocation fails, return without any clean-up;
      ResolutionLevel *next = GetLevel(lv+1);
      if (!cell->DaughtersArrayAllocated()) 
	cell->AllocateDaughterCells(next->GetDaughterCellNumber());

      // Loop through all the daughter cells;
      for(unsigned iq=0; iq<next->GetDaughterCellNumber(); iq++) {
	// FIXME: well, this is dirty style of course;
	HoughCell **dcell = cell->GetDaughterPtr(iq);

	// Do not bother to check "blind end" cells;
	if (*dcell == mBlindCell) continue;
	
	// idq[] calculation is a bit tricky;
	for(int ip=0; ip<mDimensions.size(); ip++)
	  idq[ip] = id[ip]*next->GetParameterSplitFactor(ip) + next->Remap(iq,ip);

	// Well, candidate array overflow (or out-of-memory condition) 
	// are almost the only error return codes;
	{
	  unsigned ret = CheckCell(lv+1, idq, dcell, qmembers);

	  // Well, if '-1' return code, fall out immediately;
	  //if (ret == -1) 
	  //return -1;
	  //else

	    // If track was found, and number of member hits is the same 
	    // as I have in this (parent) cell, makes no sense to look for 
	    // track candidates in other daughter cells, right?;
	  if (ret == ok_hit_counter) return ret;
	}
      } //for iq

      // Well, if daughter(s) succeeded to find track(s), but used less 
      // hits, then other hit combinations could be successful either, so 
      // can not help parent cell to take a decision to stop other 
      // daughter investigation; just return 0;
      return 0;
    } //if
  }
#endif
} // HoughTree::CheckCell()

// ---------------------------------------------------------------------------------------

#if _LATER_
void HoughTree::PrintTrackCandidateArray(unsigned from, unsigned to, 
					 TrackCandidatePrintoutFlag flag) const
{
  for(unsigned tc=from; tc<=to; tc++) {
    const MatchCandidate *match = mMatchCandidates[tc];

    // Just skip track candidates which were merged into the other ones;
    if (flag == _ACTIVE_CANDIDATES_ && !match->IsActive()) continue;
      
    unsigned multi_max = 0, ok_hit_counter = 0, ok_group_counter = 0;

    for(int gr=0; gr<mGroups.size(); gr++) {
      if (match->GetMemberCount(gr) > multi_max) 
	multi_max = match->GetMemberCount(gr);

      if (match->GetMemberCount(gr)) ok_group_counter++;
      ok_hit_counter += match->GetMemberCount(gr);
    } //for iq

    for(unsigned iq=0; iq<multi_max; iq++) {
      if (iq) 
	printf("                                                                                  ");
      else {
	const unsigned *id = match->GetIdPtr();

	printf("%2d -> tr.candidate#%02d (%c): id[]: %4d/%4d/%4d:: %2d/%2d hits/groups total -> ",
	       GetLdim()-1, tc, match->IsActive() ? '*' : ' ', 
	       id[0], id[1], mDimensions.size() == 3 ? id[2] : 0,
	       ok_hit_counter, ok_group_counter); 
      } //if
#if _LATER_      
      for(int gr=0; gr<mGroups.size(); gr++)
	// NB: Kalman filter pass could have removed certain member hits (members[][] = NULL);
	if (match->mnum[gr] > iq && match->members[gr][iq])
	  // So here I'm using only 'from' wire for printout, right?;
	  printf(" %3d", match->members[gr][iq]->_from >> (groups[gr]._nd+groups[gr]._nh));
	else
	  printf("    ");
#endif
      printf(iq == multi_max-1 ? "\n\n" : "\n");
    } //for iq
  } //for tc
} // HoughTree::PrintTrackCandidateArray()
#endif

// ---------------------------------------------------------------------------------------

//
//  THINK: this routine was actually checked carefully only in HoughTree::LaunchPatternFinder();
//

unsigned HoughTree::PurgeDuplicateTracks()
{
  unsigned counter = 0;

  // Ok, now deal with all track candidates found so far; first purge 
  // track candidate array; it can happen that earlier found tracks 
  // are in fact subsets of the later found ones (this possibility is not 
  // checked in checkCell()); to make it clear: number of fired planes 
  // may even be the same, but later found track candidates (say, at the 
  // same current "min" value) may have extra 
  // hits in some of the planes; use the same algorithm as in checkCell(); 
#if _THINK_
  // NB: per design it can NOT happen, that earlier found subset tracks were 
  // passed through KF already (since only tracks with number of planes hit 
  // matching current "min" limit were analyzed);
#endif
  for(unsigned tc=0; tc<mMatchCandidateCount; tc++) {
    MatchCandidate *match = mMatchCandidates[tc];
    
    // Do not touch track candidates already selected as BEST (and perhaps 
    // lost part of their hits in KF-pass);
    if (!match->IsActive() /*|| match->IsAlreadyAccountedAsGoodTrack()*/) continue;
    
#ifdef _NEW_SUBSET_LOGIC_
    if (IsSubset(match)) {
      match->SetInactive();
      counter++;
    } //if
#else
    for(unsigned tcr=tc+1; tcr<mMatchCandidateCount; tcr++)	{
      MatchCandidate *reference = mMatchCandidates[tcr];
      
      if (!reference->IsActive()) continue;
      
      if (match->IsSubset(this, reference)) {
	// Can not happen; otherwise something was wrong in logic;
	//assert(!match->passed_kalman_filter);
	
	//printf("Found subset track candidate -> de-activating!\n"); 
	match->SetInactive();
	counter++;
	
	// No need to check against the other patterns;
	goto _next_match;
      } /*if*/
    } /*for tcr*/
    
  _next_match:
    continue;
#endif
  } //for tc

  return counter;
} // HoughTree::PurgeDuplicateTracks()

// ---------------------------------------------------------------------------------------

unsigned HoughTree::LaunchPatternFinder()
{
  // Lock configuration; do not mind to do it several times;
  mInitialized = true;

  // Reset number of match candidates (track candidates at highest 
  // resolution level in tracking context);
  mMatchCandidateCount = 0;

  //
  // Original OLYMPUS implementation was aimed at finding a single "best" track 
  // candidate; STAR forward tracker upgrade has completely different requirements,
  // so rework the logic of this call completely:
  //
  //   - arrange a single loop over min. allowed hits; start from highest value 
  //     (indeed efficiency is expected to be high); so if there are N planes and 
  //     min. allowed hit count is M=N-2, there will be 3 loops (wich this limit 
  //     first set to N, then to N-1, and then to N-2); this is probably not the 
  //     most efficient way of doing things, but should - at least if the highest
  //     level resolution is pushed to its limits - allow to avoid having a "bush" 
  //     of lower-hit-count track candidates around true tracks with max. possible 
  //     hit count; yes, they would probably be gone during purging process, yet 
  //     this can cost a combinatorial CPU overhead;
  //
  //   - in each cycle of this loop:
  //
  //      - purge track candidate array (it can happen, that two track candidates
  //        have pretty much identical set of hits, except for say one plane where 
  //        one of the tracks has a 2-d hit not present in the first track);
  //
  //      - for each of remaining track candidates try to find at least one 
  //        combination of M hits (see M definition above), which produces a fit 
  //        with sufficiently low chi^2; this is costly indeed; do not consider
  //        combinatorial approach if track candidate has >1 hit in some of the 
  //        planes, but rather "anneal" such configurations in a way fit converges 
  //        in number of steps, proportional to the number of excess hits;
  //
  //      - discard track candidates which fail this primary fit;
  //
  //      - build array of vertex overlaps which have "hit deficit" (say, 3 tracks
  //        want to share 2 hits in some plane); build arrays of tracks which are 
  //        mutually interconnected by these vertices; figure out, whether part
  //        of the member tracks have to be dropped, because min. hit condition can 
  //        not be met for all of them at once (say 2 track candidates share 5 hits,
  //        but differ in the other N-5 ones; clearly if N-2 is the minimum allowed
  //        hit count per track, one of them should be discarded); do this based on 
  //        1) max fired plane counter, 2) if equal, lowest chi^2 after the fit;
  //
  //      - mark hits of remaining tracks as BUSY for the cases when say 3 tracks 
  //        share hit group consisting of 3 hits; in redundant cases (say a single 
  //        track has 2 hits in some plane) either do not set BUSY flag or set it 
  //        conditionally (THINK later);
  //        
  //      - enter next cycle with presumably several hits marked as BUSY; continue
  //        track search with less stringent min. allowed hit count condition, assuming, 
  //        that part of the hits can be "borrowed" from already found tracks; 
  //
  //      - eventually will have a set of track candidates, which probably overlap 
  //        with each other through their plane hits; build isolated groups of track
  //        candidates and allocate hits within these groups using some sort of 
  //        "annealing" procedure;
  //
  // -> if track candidate had >1 hit in some planes, KF-fit starts under 
  //    assumption that coordinate in this plane is defined only within the 
  //    suggested hit range - so it is (last-first)/2 with a flat distribution;
  //    then the first track approximation is built, closest hit in each of
  //    ambiguos planes selected and final KF-pass is run; THINK: may be in steps?;
  //

  // Want to find track candidates with max hit count first --> arrange 
  // a loop decreasing min number of hit planes limit; this may not be the 
  // most efficient way of doing things, but it is very logical;
  for(unsigned min=mMaxOkHitCounter; min>=mMinOkHitCounter; min--) {
    mCurrMinOkHitCounter = min;

    // Yes, min-qua loops should be in this order;
  for(unsigned qua=0; qua<mTrackQualityIterationNum; qua++) {
    SetupTrackQualityIteration(qua);

    // Loop indefinitely with this "min" value until there are no more new tracks found;
    for( ; ; ) {
      unsigned last_track_candidate_count = mMatchCandidateCount;

      {
#ifdef __APPLE__
	assert(0);
#else
	// Same story as in HoughTree::CheckCell(): need a Mac OS workaround at some point;
	std::vector<GroupMember*> buffer[mGroups.size()];

	for(int gr=0; gr<mGroups.size(); gr++) {
	  HoughNodeGroup *group = mGroups[gr];

	  for(unsigned mm=0; mm<group->GetMemberCount(); mm++) {
	    GroupMember *member = group->GetMember(mm);

	    buffer[gr].push_back(member);
	  } //for mm
	} //for gr
	
	unsigned id[mDimensions.size()];
	// Yes, just enter the 0-level with its single cell;
	memset(id, 0x00, sizeof(id));
	
	/*int ret =*/ CheckCell(0, id, &mCellTree, buffer);
	
	// THINK: can happen only if memory allocation fails (or some other generic 
	// problem like user call failure or mapping table import problem);
	//if (ret == -1) return -1;
	//assert(ret != -1);
#endif
      }

      printf("\n\n------------------------------------------------------------------\n");
      printf("%3d new track candidate(s) ... -> ", mMatchCandidateCount - last_track_candidate_count);
      
      unsigned counter = PurgeDuplicateTracks(); 
      
      printf("%3d survived ... \n", mMatchCandidateCount - last_track_candidate_count - counter);

      // Split track candidates into non-overlapping groups;
      std::vector<MatchCandidateGroup> mMatchCandidateGroups;

#if 1
      if (mFastTreeSearchMode) goto _fast_tree_search_mode;
#endif

      // NB: match->mGrouped & match->mAccountedAsGoodTrack are false here
      // as reset in ShapeItUpForInspection() during tree search;
      for(unsigned tc=last_track_candidate_count; tc<mMatchCandidateCount; tc++) {
	MatchCandidate *match = mMatchCandidates[tc];

	// Skip disabled candidates (most likely subsets of other ones);
	if (!match->IsActive()) continue;
	
	// Also skip those, which are grouped already as a result of recursion;
	if (match->IsGrouped()) continue;

	// Start new group and initialize it with this match candidate; the 
	// recursion happens inside this call;
	mMatchCandidateGroups.push_back(MatchCandidateGroup(match));
      } //for tc

      // FIXME: siam track splitter will modify mMatchCandidateCount internally, so 
      // for safety reasons one should not use mMatchCandidateCount rather than in the 
      // later GetLinearMatchCandidateCount() call in FwdTrackFinder::Exec();

      printf("   --> %2d group(s) ...\n", (unsigned)mMatchCandidateGroups.size());
      //return 0;

      // Now can work with groups of non-overlapping track candidates separately;
      for(unsigned gr=0; gr<mMatchCandidateGroups.size(); gr++) {
	MatchCandidateGroup *mgroup = &mMatchCandidateGroups[gr];

	printf("\n      gr#%02d -> %2d track(s)\n", gr, mgroup->GetCandidateCount());

#if _OLD_
	// Split siam tracks first; FIXME: at max missing hit level only for 
	// now (and assuming this limit means "no missing hits");
	if (min == mMaxOkHitCounter) {
	  unsigned trCounter = 0;
	  std::vector<MatchCandidate*> newMatches;

	  for(std::multimap<__u64, MatchCandidate*>::iterator it=mgroup->Begin();
	      it != mgroup->End(); it++) {
	    MatchCandidate *match = it->second;

	    if (match->SiamGroupCandidate(min)) {
	      printf("tr#%2d -> siam group candidate: doit!\n", trCounter++);

	      // NB: this call will change mMatchCandidateGroups.size() internally;
	      SeparateSiamTracks(match, min, &newMatches);
	    } //if
	  } //for it
	  //continue;

	  // Add newcomers to the group pool;
	  for(unsigned tr=0; tr<newMatches.size(); tr++) {
	    mgroup->AddCandidate(newMatches[tr]);
	  } //for tr

	  if (newMatches.size())
	    printf("\n      gr#%02d -> %2d track(s)\n", gr, mgroup->GetCandidateCount());
	}
#endif

	unsigned trCounter = 0;
	for(std::multimap<__u64, MatchCandidate*>::iterator it=mgroup->Begin();
	    it != mgroup->End(); it++) {
	  MatchCandidate *match = it->second;

	  if (!match->IsActive()) continue;

	  // Easy case; debug later;
#if 0
	  //if (!match->HasAmbiguousHits()) {
	  if (match->HasAmbiguousHits()) {
	    //printf("tr#%2d -> no ambiguities        : skip!\n", trCounter++);
	    printf("tr#%2d -> ambiguities        : skip!\n", trCounter++);
	    match->SetInactive();
	    continue;
	  } //if
#endif

#if _OLD_
	  // FIXME: precaution for "missing hit non-trivial" case (see code above);
	  if (match->SiamGroupCandidate(min)) {
	    match->SetInactive();
	    continue;
	  } //if

	  // Get rid of hit ambiguities; perhaps in a few iterations; 
	  // NB: in Fwd-case at this point match->mPassedKalmanFilterOnce is 'false' ->
	  // ResolveAmbiguities() will be called at least once;
	  while (!match->IsReadyForFinalFit()) {
	    //printf("            tc#%02: ambiguous!\n", tc);
	    
	    // This call is supposed to resolve at least one ambiguity, 
	    // so the 'while' loop can not run indefinitely;
	    ResolveAmbiguities(match);
	  } //while
#else
	  while (!match->IsReadyForFinalFit()) 
	    // This call is supposed to resolve at least one ambiguity, 
	    // so the 'while' loop can not run indefinitely;
	    ResolveAmbiguitiesNg(match);
#endif

	  assert(match->IsActive());
	  FinalFit(match);
	} //for it

	// Tracks may still have overlaps in part of their hits; this basically 
	// means, that in the present approach part of the tracks should go; 
	// since at least one more global iteration with the same "min" hit 
	// counter will be done, removed tracks have chance to return back, 
	// taking hits which they refused (with a hope for better chi^2 fit) 
	// during present iteration; FIXME: logic here needs to be optimized further;
	{
	  // Will be ordered according to chi^2 CCDF (or such, a virtual method);
	  std::multimap<double, MatchCandidate*> finalCandidates;

	  for(std::multimap<__u64, MatchCandidate*>::iterator it=mgroup->Begin();
	      it != mgroup->End(); it++) {
	    MatchCandidate *match = it->second;
	  
	    finalCandidates.insert(std::pair<double, 
				   MatchCandidate*>(match->GetTrackQualityParameter(), match));
	  } //for it

	  // Now loop through all tracks ordered in chi^2 CCDF and effectively purge 
	  // those which probably not own proper hits; NB: reversed order in CCDF values;
	  for(std::multimap<double, MatchCandidate*>::reverse_iterator it = finalCandidates.rbegin();
	      it != finalCandidates.rend(); it++) {
	    MatchCandidate *match = it->second;

	    // SKip those which are dead already;
	    if (!match->IsActive()) continue;
	    
	    // Check remaining hit count; it may have fallen below the limit because other (better)
	    // tracks acquired part of the hits;
	    if (match->GetAliveGroupCount() < min) {
	      //assert(0);
	      match->SetInactive();
	    }
	    else {
	      for(unsigned gr=0; gr<mGroups.size(); gr++) {
		// Indeed by this point track has given away all other hits but single
		// per plane group; FIXME: allow >1 hit per plane group later;
		assert(match->GetAliveMemberCount(gr) <= 1);
		
		GroupMember *member = match->GetFirstAliveMember(gr);
		// THINK: can happen on lower 'min' loop levels?;
		if (!member) continue;

		// Remove used hits from other tracks which belong to this group; 
		for(std::multimap<double, MatchCandidate*>::reverse_iterator is = finalCandidates.rbegin();
		    is != finalCandidates.rend(); is++) {
		  MatchCandidate *qmatch = is->second;
		  
		  if (!qmatch->IsActive()) continue;
		  
		  // Skip myself; FIXME: optimize loop starting point;
		  if (qmatch == match) continue;

		  // For the same reason as in case of 'match' pointer, right?;
		  assert(qmatch->GetAliveMemberCount(gr) <= 1);
		  GroupMember *qmember = qmatch->GetFirstAliveMember(gr);
		  // THINK: can happen on lower 'min' loop levels?;
		  if (!qmember) continue;
		  
		  // But if they match, clean up;
		  if (member == qmember) qmatch->ResetMemberPtr(gr, qmember);
		} //for is

		// Yes, only now eventually claim, that this hit is busy; so other track 
		// candidates who wanted to own it will effectively die during this 
		// search+fit pass and will have to try to recover using other hit(s) 
		// in this plane group during the next iteration on the same 'min" level 
		// (which is guaranteed to happen as long as at least one new track was 
		// found and indeed it was);
		member->SetBusyFlag();
	      } //for gr
	    } //if
	  } //for it
	}
      } //for gr

    _fast_tree_search_mode:
      // Check whether good candidates were added; break and switch to lower "min"
      // hit count limit if none; 
      {
	unsigned newTrackCount = 0;

	for(unsigned tc=last_track_candidate_count; tc<mMatchCandidateCount; tc++) {
	  MatchCandidate *match = mMatchCandidates[tc];

	  if (match->IsActive()) newTrackCount++;
	} //for tc

	//if (mCurrMinOkHitCounter == 5 && newTrackCount) printf("@555@\n");
	printf("@GGG@: min=%2d, added %4d new track(s) ...\n", min, newTrackCount);

	if (!newTrackCount) break;
      }
    } //for inf
  } //for qua
  } //for min

  {
    unsigned counter = PurgeDuplicateTracks();
    if (counter) printf(" @PPP@ Purged %2d more duplicate track(s)!\n", counter);
  } 

  return 0;
} // HoughTree::LaunchPatternFinder()

// ---------------------------------------------------------------------------------------
