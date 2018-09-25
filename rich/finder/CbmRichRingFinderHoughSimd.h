/**
* \file CbmRichRingFinderHoughSimd.h
*
* \brief SIMDized ring finder based on Hough Transform method.
*
* \author Semen Lebedev
* \date 2009
**/

#ifndef CBM_RICH_RING_FINDER_HOUGH_SIMD
#define CBM_RICH_RING_FINDER_HOUGH_SIMD

#include "CbmRichRingFinderHoughImpl.h"
//#include "../L1/L1Algo/L1Types.h"
//#include "../L1/L1Algo/vectors/P4_F32vec4.h"

class CbmRichHoughHitVec {
public:
	fvec fX;
	fvec fY;
	fvec fX2plusY2;
	//unsigned short int fId;
	//Bool_t fIsUsed;
}  _fvecalignment;

/**
* \class CbmRichRingFinderHoughSimd
*
* \brief SIMDized ring finder based on Hough Transform method.
*
* \author Semen Lebedev
* \date 2009
**/
class CbmRichRingFinderHoughSimd : public CbmRichRingFinderHoughImpl
{

public:
  	CbmRichRingFinderHoughSimd ();

	~CbmRichRingFinderHoughSimd(){;}

	virtual void HoughTransformReconstruction();

	virtual void HoughTransformGroup(
	      unsigned short int indmin,
			unsigned short int indmax,
			Int_t iPart);

	std::vector<CbmRichHoughHitVec> fDataV;

};
#endif
