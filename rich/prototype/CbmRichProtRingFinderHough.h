/**
* \file CbmRichProtRingFinderHough.h
*
* \brief Main class for ring finder based on Hough Transform implementation.
*
* \author Semen Lebedev
* \date 2008
**/

#ifndef CBM_RICH_PROT_RING_FINDER_HOUGH_H_
#define CBM_RICH_PROT_RING_FINDER_HOUGH_H_

#include "CbmRichRingFinder.h"
#include <vector>

class CbmRichProtRingFinderHoughImpl;
class CbmRichRingFinderHoughSimd;
class CbmRichRing;
class CbmRichRingLight;

#define HOUGH_SERIAL
//#define HOUGH_SIMD

using std::vector;

/**
* \class CbmRichProtRingFinderHough
*
* \brief Main class for ring finder based on Hough Transform implementation.
*
* \author Semen Lebedev
* \date 2008
**/
class CbmRichProtRingFinderHough : public CbmRichRingFinder
{
protected:
	Int_t fNEvent; // event number
	Int_t fRingCount; // number of found rings

// choose between serial and SIMD implementation of the ring finder
#ifdef HOUGH_SERIAL
	CbmRichProtRingFinderHoughImpl *fHTImpl;
#endif

#ifdef HOUGH_SIMD
	CbmRichRingFinderHoughSimd *fHTImpl;
#endif

public:

	/**
    * \brief Standard constructor.
    */
  	CbmRichProtRingFinderHough ();

   /**
    * \brief Destructor.
    */
	virtual ~CbmRichProtRingFinderHough();

	/**
	 * \brief Inherited from CbmRichRingFinder.
	 */
	virtual void Init();

	/**
	 * \brief Inherited from CbmRichRingFinder.
	 */
	virtual Int_t DoFind(
	      TClonesArray* rHitArray,
	 		TClonesArray* rProjArray,
		   TClonesArray* rRingArray);

private:
	/**
	 * \brief Add found rings to the output TClonesArray.
	 * \param[out] rRingArray Output array of CbmRichRing.
	 * \param[in] rings Found rings.
	 */
	void AddRingsToOutputArray(
	      TClonesArray *rRingArray,
	      const vector<CbmRichRingLight*>& rings);

	/**
	 * \brief Copy constructor.
	 */
	CbmRichProtRingFinderHough(const CbmRichProtRingFinderHough&);

   /**
    * \brief Assignment operator.
    */
	CbmRichProtRingFinderHough& operator=(const CbmRichProtRingFinderHough&);
};

#endif
