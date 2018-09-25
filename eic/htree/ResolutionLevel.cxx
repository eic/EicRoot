//
// AYK (ayk@bnl.gov)
//
//  Resolution level description;
//
//  Initial port from OLYMPUS sources: Oct'2015;
//

// Yes, unfortunately I need HoughTree include here;
#include <HoughTree.h>
#include <ResolutionLevel.h>

// ---------------------------------------------------------------------------------------

ResolutionLevel::ResolutionLevel(const HoughTree *tree, const unsigned div[])
{
  unsigned ddim = tree->GetDdim(), ldim = tree->GetLdim();

  {
    mParameterSplit   = new unsigned[ddim];
    mTotalDivisionNum = new unsigned[ddim];
    mCellSize         = new double  [ddim];

    // THINK: really needed?;
    if (!div || !mParameterSplit || !mTotalDivisionNum || !mCellSize) throw;
  }

  mDaughterCellNum = 1;

  {
    for(int iq=0; iq<ddim; iq++) {
      mParameterSplit[iq] = div[iq];
      mTotalDivisionNum [iq] = (ldim == 0 ? 1 : tree->GetLevel(ldim-1)->mTotalDivisionNum[iq])*div[iq];

      // And cell size, please;
      {
	const HoughDimension *dimension = tree->GetDimension(iq);
      
	mCellSize[iq] = (dimension->GetMax() - dimension->GetMin())/mTotalDivisionNum[iq];
      } 
      
      mDaughterCellNum *= mParameterSplit[iq];
    } //for iq 
  }

  // FIXME: suitable for 2D & 3D parameter space only;
  if (tree->GetVerbosityLevel() >= 1)
    printf("lv=%2d -> accu=%5d/%5d/%5d; mCellSize[]: %8.3f %8.3f %8.3f\n", 
	   ldim, mTotalDivisionNum[0], mTotalDivisionNum[1], 
	   ddim == 2 ? 0 : mTotalDivisionNum [2],
	   mCellSize[0], mCellSize[1], ddim == 2 ? 0.0 : mCellSize[2]); 

  mTotalCellNum = (ldim == 0 ? 1 : tree->GetLevel(ldim-1)->mTotalCellNum)*mDaughterCellNum;

  mRemap = new unsigned[mDaughterCellNum*ddim];
  // THINK: really needed?;
  if (!mRemap) throw;

  {
    // In 4D case id(a,b,c,d) = a*d2*d3*d4 + b*d3*d4 + c*d4 + d; sdim[] is just 
    // array of coefficients by {a,b,c,d};
    unsigned sdim[ddim];
  
    for(int iq=ddim-1; iq>=0; iq--)
      sdim[iq] = iq == ddim-1 ? 1 : mParameterSplit[iq+1]*sdim[iq+1];
    
    // Loop through all the entries and fill the array out;
    for(unsigned ip=0; ip<mDaughterCellNum; ip++) {
      // Figure out 1D-indices;
      unsigned curr = ip;
      
      // NB: indices in mRemap[] array are sort of swapped for convenience;
      for(int iq=0; iq<ddim; iq++) {
	mRemap[ip+iq*mDaughterCellNum] = curr / sdim[iq];
	curr -= mRemap[ip+iq*mDaughterCellNum]*sdim[iq]; 
      } //for iq
    } //for ip
  }
} // ResolutionLevel::ResolutionLevel()

// ---------------------------------------------------------------------------------------
