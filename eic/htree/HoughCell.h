//
// AYK (ayk@bnl.gov)
//
//  Hough transform code elementary cell structure;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#ifndef _HOUGH_CELL_
#define _HOUGH_CELL_

class HoughTree;

class HoughCell {
 public:
  HoughCell(const HoughTree *tree);
  ~HoughCell() {
    delete [] mFrom; delete [] mTo;

    // FIXME: clean up contents as well; need to store dimension then;
    delete [] mDaughters;
  };

  bool DaughtersArrayAllocated() const { return mDaughters ? true : false; };

  void SetDaughter(unsigned iq, HoughCell *ptr) { mDaughters[iq] = ptr; };
  // FIXME: this could have been done better?;
  HoughCell **GetDaughterPtr(unsigned iq) const { return &mDaughters[iq]; };

  void UpdateRanges(const HoughTree *tree, const t_hough_range id[]);

  void AllocateDaughterCells(unsigned num) {
    mDaughters = new HoughCell*[num];
    memset(mDaughters, 0x00, num*sizeof(HoughCell*));
  };

  void ResetRanges(const HoughTree *tree, bool immunity[] = 0);

  // Not a user call -> range check not needed (?);
  t_hough_range From(unsigned gr) const { return mFrom[gr]; };
  t_hough_range To  (unsigned gr) const { return mTo  [gr]; };

  const t_hough_range* From()     const { return mFrom; }; 
  const t_hough_range* To  ()     const { return mTo; };

  private:
  // One entry per plane group;
  t_hough_range *mFrom, *mTo;

  // I guess it's fine to have it as a double pointer here rather 
  // than any STL class;
  HoughCell **mDaughters;
};

#endif
