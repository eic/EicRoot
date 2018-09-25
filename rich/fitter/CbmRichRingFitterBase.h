/**
* \file CbmRichRingFitterBase.h
*
* \brief Abstract base class for concrete Rich Ring fitting algorithms.
*  Each derived class must implement the method DoFit.
*
* \author Semen Lebedev
* \date 2012
**/
#ifndef CBM_RICH_RING_FITTER_BASE
#define CBM_RICH_RING_FITTER_BASE

#include "CbmRichRingLight.h"
/**
* \class CbmRichRingFitterBase
*
* \brief Abstract base class for concrete Rich Ring fitting algorithms.
*  Each derived class must implement the method DoFit.
*
* \author Semen Lebedev
* \date 2012
**/
class CbmRichRingFitterBase
{
public:

	/**
	 * \brief Default constructor.
	 */
   CbmRichRingFitterBase(){ }

	/**
	 * \brief Destructor.
	 */
	virtual ~CbmRichRingFitterBase() { }

	/**
	 * \brief Abstract method DoFit. To be implemented in the concrete class.
	 * Perform a fit to the hits attached to the ring by a ring
	 * finder. Fill the ring parameters member variables.
	 * \param[in,out] ring Pointer to CbmRichRingLight
	 */
	virtual void DoFit(
	      CbmRichRingLight* ring) = 0;

protected:

	/**
	 * \brief Calculate chi2 for circle fitting algorithms.
	 * \param[in,out] ring Fitted ring.
	 */
	virtual void CalcChi2(
	      CbmRichRingLight* ring)
	{
	   int nofHits = ring->GetNofHits();
      if ( nofHits < 4 ) {
         ring->SetChi2(-1.);
         return;
      }

      float chi2 = 0.;
      float r = ring->GetRadius();
      float xc = ring->GetCenterX();
      float yc = ring->GetCenterY();

      for (int i = 0; i < nofHits; i++) {
         float xh = ring->GetHit(i).fX;
         float yh = ring->GetHit(i).fY;
         float d = r - sqrt((xc - xh)*(xc - xh) + (yc - yh)*(yc - yh));

         chi2 += d*d;
      }
      ring->SetChi2(chi2);
	}

	static const int MAX_NOF_HITS_IN_RING = 400; // maximum possible number of hits
};

#endif
