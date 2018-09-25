//
// AYK (ayk@bnl.gov)
//
//  Resolution level description;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

#ifndef _RESOLUTION_LEVEL_
#define _RESOLUTION_LEVEL_

class HoughTree;

class ResolutionLevel 
{
 public:
  ResolutionLevel(const HoughTree *tree, const unsigned div[]);
  ~ResolutionLevel() {};

  // THINK: calls from main code only -> no range check?; 
  double GetParameterSplitFactor  (unsigned ip)  const { return mParameterSplit  [ip]; };
  double GetCellSize              (unsigned ip)  const { return mCellSize        [ip]; };

  unsigned GetTotalDivisionNumber (unsigned ip)  const { return mTotalDivisionNum[ip]; };
  unsigned GetDaughterCellNumber()               const { return mDaughterCellNum; };

  // NB: since mDim is readily available in this class, it is convenient
  // to sort of swap indices in mRemap[] 2D storage array; 
  unsigned Remap(unsigned i,       unsigned ip)  const { return mRemap[i+ip*mDaughterCellNum]; }; 

 private:
  // Divisions of individual parameters and overall dimension of 
  // the (linear, 1D) daughter cell array on this level;
  unsigned *mParameterSplit, mDaughterCellNum;

  // Accumulated number of cells on this level, including divisions
  // of each dimension separately;
  unsigned *mTotalDivisionNum, mTotalCellNum;

  // Parameter space rectangular cell size on this resolution level;
  double *mCellSize;

  unsigned *mRemap; 
};

#endif
