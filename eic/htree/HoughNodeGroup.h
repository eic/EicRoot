//
// AYK (ayk@bnl.gov)
//
//  Hough transform node definitions; 
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#include <set>
#include <cassert>
#include <math.h>

#include <TVector3.h>

#include <SensitiveVolume.h>
#include <EicMagneticField.h>
#include <EicTrackingDigiHit.h>

#ifndef _HOUGH_NODE_GROUP_
#define _HOUGH_NODE_GROUP_

class MatchCandidate;

// FIXME: use system definitions, please;
typedef unsigned short     __u16;
typedef unsigned int       __u32;
typedef unsigned long long __u64;

// Well, in principle one would like to have a scheme where planes with 
// different t_hough_range can be used at the same time; however from practical 
// point of view even if one wants say to mix 1D and 2D detectors, it is easier 
// to use the same type (sufficiently wide for everybody) and just pack the 
// bits differently; this allows to use arrays of t_hough_range variables
// in several places rather than calculate nasty offsets or arrange more 
// complicated STL objects (?); in this case there is no real advantage 
// of "templatizing" GroupMember & HoughNodeGroup classes because will have 
// to do the same with HoughTree (and then move all the sources to HoughTree.h
// file); so for now consider to use a single hardcoded type throughout the 
// code (and if width becomes insufficient for certain application, just 
// take __u64 or so and recompile);
typedef __u32 t_hough_range;

// So these definitions are already invariant over t_hough_range type; and in this
// approach they become global define's;
#define _MAX_USEFUL_BIT_COUNT_     ((sizeof(t_hough_range) << 3)-1)
#define __OUT_OF_RANGE_BIT_        (((t_hough_range)0x1) << _MAX_USEFUL_BIT_COUNT_)

class GroupMember {
public:
 GroupMember(const std::pair<void *, void *> &ptr, t_hough_range from, t_hough_range to): 
  mPtr(ptr), mFrom(from), mTo(to), mBusy(false) {};

  // Want to encapsulate everything and ultimately switch to a completely
  // templated version with policies;
  t_hough_range From()  const { return mFrom; };
  t_hough_range To()    const { return mTo; };

  // Member can be booked during tree search, but not yet busy; vice versa is 
  // always true however; FIXME: may actually want to limit IsBooked() call to the 
  // current tree search pass (so that do not take into account owners from previous passes);
  bool IsBusy()         const { return mBusy; };
  bool IsBooked()       const { return (mMatchCandidates.size() != 0); };
  void SetBusyFlag()          { mBusy = true; };

  // Something which allows to identify this member hit back in the 
  // caller fit routine; FIXME: should be done better;
  std::pair <void *, void *> mPtr;

  void InsertMatchCandidate(MatchCandidate *match) {  mMatchCandidates.insert(match); };
  void EraseMatchCandidate(MatchCandidate *match)  {  mMatchCandidates.erase(match); };

  // FIXME: allocate iterators once?;
  std::set<MatchCandidate*>::iterator Begin() { return mMatchCandidates.begin(); };
  std::set<MatchCandidate*>::iterator End()   { return mMatchCandidates.end(); };

 private:
  // Back-door pointers to match candidates which claim to own this hit;
  // NB: since hit borrowing is allowed, there can in principle be more than 
  // one track candidate pretending to own any given hit (may eventually want to 
  // allow hit sharing, etc -> THINK later, but for now keep std::set here rather
  // than a single pointer;);
  std::set<MatchCandidate*> mMatchCandidates; 

  // NB: can not use mMatchCandidates.size() as a BUSY sign, since several track 
  // candidates may want to claim a given hit during tree search; mBusy flag will be 
  // asserted only after tree search is finished and "best track candidate" grabs 
  // the hit (and then other track candidates with a lower chi^2 CCDF will not have 
  // access to it any longer during this particular search+fit pass);
  bool mBusy;

  // Micro-cell geographical coordinates; NB: they are not guaranteed to 
  // be ordered in all componenets (ngroup->Overlap() call will check);
  t_hough_range mFrom, mTo;
};

// FIXME: may want to unify with EicBitMask at some point;
class CoordinateDescriptor {
 public:
 CoordinateDescriptor(double min, double max, double gra, unsigned shift): mMin(min), mGra(gra), mShift(shift) {
    // THINK: is this strictly true?;
    mWdim = int((max - min)/gra);

    mWidth = bits(mWdim);
    mMask  = ~(t_hough_range(0)) >> ((sizeof(t_hough_range) << 3) - mWidth);
    //printf("%2d -> %X %X\n", mWidth, mMask, wdim-1);
  };

  double GetMin()        const { return mMin; };
  double GetGra()        const { return mGra; };

  unsigned GetBitWidth() const { return mWidth; };

  // NB: assume I'm given 0-based values here (so also return 0-based one);
  t_hough_range UnpackCoord(t_hough_range value) const { return (value >> mShift) & mMask; };
  // Well, assume coordinates can be out of [0..mWdim-1] range and then need to 
  // be regularized; also assume, that 'int' width is sufficient for all practical cases;
  t_hough_range   PackCoord(int coord) const {
    if (coord < 0) 
      // Effectively pack coord=0;
      return __OUT_OF_RANGE_BIT_;
    else if (coord >= mWdim) 
      // Pack max in-range coord value; NB: '& mMask' is not really needed here and below;
      return (((t_hough_range)mWdim-1) << mShift) | __OUT_OF_RANGE_BIT_;
    else
      return  ((t_hough_range)coord)   << mShift;
  };

  t_hough_range OffsetThisValueComponent(t_hough_range value, int offset) {
    if (!offset) return value;

    t_hough_range coord = (value >> mShift) & mMask;
    int result = coord + offset;

    if (result < 0) 
      return 0x0;
    else if (result >= mWdim)
      return ((t_hough_range)(mWdim-1)) << mShift;
    else
      return ((t_hough_range)result)    << mShift;
  };

 private:
  // FIXME: unify with the same code in HoughTree.cxx;
  unsigned bits(t_hough_range value) {
    // Yes, prefer to avoid pathologic cases; at least 1 bit allocation please;
    if (value == 1) return 1;

    unsigned count = 0;

    // Subtract 1 and shift to the right till get zero value;
    for(value--; value; count++) 
      value >>= 1;
    
    return count;
  };

  // Parameters for bit-wise packing operations: field shift, width and
  // mask once shifted to 0-th position;
  unsigned mShift, mWidth;
  t_hough_range mMask;

  double mMin, mGra;
  // Basically means: expected "strip id" range is [0..mWdim-1];
  unsigned mWdim; 
};

class HoughNodeGroup {
 public:
 HoughNodeGroup(unsigned id): mId(id), mPhaseSpaceSmearing(0) {};

  void ConfigureCoordinateDescriptors(unsigned cdim, const double min[], const double max[],
				      const double gra[]) {
    // FIXME: range check here, please;
    unsigned accu = 0;

    for(unsigned iq=0; iq<cdim; iq++) {
      mCoordDescr.push_back(CoordinateDescriptor(min[iq], max[iq], gra[iq], accu));

      accu += mCoordDescr[mCoordDescr.size()-1].GetBitWidth();
    } //for iq
  };
  virtual ~HoughNodeGroup() {};

  int AddMember(const std::pair<void *, void *> &ptr, t_hough_range from, t_hough_range to) {
    // FIXME; check out-of-range condition;
#if _THINK_
    if (from > to || to > _imax) return -1;
#endif    

    mMembers.push_back(GroupMember(ptr, from, to));
    
    return 0;
  }; 

  void ResetMemberCounter()                      { mMembers.clear(); };
  unsigned GetMemberCount()                const { return mMembers.size(); };
  GroupMember *GetMember(unsigned mm)            { return (mm < mMembers.size() ? &mMembers[mm] : 0); }; 

  void SetPhaseSpaceSmearing(unsigned smearing)  { mPhaseSpaceSmearing = smearing; };
  unsigned GetPhaseSpaceSmearing()         const { return mPhaseSpaceSmearing; };

  unsigned GetGroupId()                    const { return mId; };

  // FIXME: put this XYZ stuff in some other place eventually;
  unsigned GetCoord(t_hough_range value, EicMagneticField::XYZ id) { 
    if (id >= mCoordDescr.size()) return 0;

    return mCoordDescr[id].UnpackCoord(value);
  };

  const std::vector<CoordinateDescriptor> &GetCoordDescr() { return mCoordDescr; };

  // Shift all packed components by (possibly negative) offset and pack 
  // back; NB: obey boundary conditions [0..mWdim-1];
  t_hough_range OffsetThisValue(t_hough_range value, int offset) {
    t_hough_range ret = 0x0;

    for(unsigned iq=0; iq<mCoordDescr.size(); iq++) 
      ret |= mCoordDescr[iq].OffsetThisValueComponent(value, offset);
    
    return ret;
  };

  // NB: in generic case need to confirm overlap in all dimensions; FIXME: assume, 
  // that neither of 2x2 t_hough_range values has an __OUT_OF_RANGE_BIT_ bit set;
  bool Overlap(std::pair<t_hough_range, t_hough_range> r1, 
	       std::pair<t_hough_range, t_hough_range> r2) {
    for(unsigned iq=0; iq<mCoordDescr.size(); iq++) {
      CoordinateDescriptor *descr = &mCoordDescr[iq];

      // This pair presently comes from member->From()/To(); ranges here may 
      // be swapped; THINK: may want to swap wrongly ordered components in 
      // AddMember() call?;
      t_hough_range r1l = descr->UnpackCoord(r1.first);
      t_hough_range r1r = descr->UnpackCoord(r1.second);
      if (r1l > r1r) {
	t_hough_range bff = r1l;

	r1l = r1r;
	r1r = bff;
      } //if
      //assert(r1l < r1r);

      // This pair is presently cell->From()/To(); they are guaranteed 
      // to be ordered (although can be equal: the same cell); so leave assert() here;
      t_hough_range r2l = descr->UnpackCoord(r2.first);
      t_hough_range r2r = descr->UnpackCoord(r2.second);
      // FIXME: return the strict check back!;
      //if (r2l == r2r) printf("%8X %8X\n", r2l, r2r);
      assert(r2l <= r2r);

      if (r2l > r1r || r2r < r1l) return false;
    } //for iq
    
    return true;
  };

  double GetMin(unsigned iq) const { 
    return iq < mCoordDescr.size() ? mCoordDescr[iq].GetMin() : 0.0; 
  };
  double GetGra(unsigned iq) const { 
    return iq < mCoordDescr.size() ? mCoordDescr[iq].GetGra() : 0.0; 
  };

 protected:
  // Coordinate descriptors (either X or XY or XYZ are packed into 
  // a single t_hough_range variable; pack/unpack operations are needed);
  std::vector<CoordinateDescriptor> mCoordDescr;

  t_hough_range HoughPack(const int coord[]) {
    t_hough_range value = 0x0;

    for(unsigned iq=0; iq<mCoordDescr.size(); iq++)
      value |= mCoordDescr[iq].PackCoord(coord[iq]);

    return value;
  };

 private:
  // Tree search should allow limits on hit count within certain group "types";
  unsigned mId;

  // Perhaps add +/- few {strip} units for safety when checking ranges?;
  unsigned mPhaseSpaceSmearing;

  // Array of elementary detectors to be OR'ed during calculations; these 
  // are *hits* of a given plane, in the track finder context; 
#if _LATER_
  // they can be split into a few "neighboring" entries with different weights;
#endif
  std::vector<GroupMember> mMembers;
};

#endif
