/**
* \file CbmRichRingSelectImpl.h
*
* \brief
*
* \author Semen Lebedev
* \date 2010
**/

#ifndef CBM_RICH_RING_SELECT_IMPL
#define CBM_RICH_RING_SELECT_IMPL

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "CbmRichRingLight.h"

/**
* \class CbmRichRingSelectImpl
*
* \brief
*
* \author Semen Lebedev
* \date 2010
**/
class CbmRichRingSelectImpl
{
private:

   static const int kMAX_NOF_HITS = 100; // Maximum number of hits in ring

public:
   /**
    * \brief Standard constructor.
    */
	CbmRichRingSelectImpl():
	   fAlpha(),
	   fPhi()
	{
		fAlpha.resize(kMAX_NOF_HITS);
		fPhi.resize(kMAX_NOF_HITS);
	};

	/**
	 * \brief Destructor.
	 */
	~CbmRichRingSelectImpl() { };

   /**
    * Calculates number of hits on a ring.
    * \param[in] ring Found and fitted ring.
    */
	int GetNofHitsOnRingCircle(
	      CbmRichRingLight* ring)
	{
	   int count = 0;
	   int nHits = ring->GetNofHits();
	   for (int iH = 0; iH < nHits; iH++) {
	      CbmRichHitLight hitRing = ring->GetHit(iH);
	      float rx = hitRing.fX - ring->GetCenterX();
	      float ry = hitRing.fY - ring->GetCenterY();
	      float r = sqrt(rx * rx + ry * ry) - ring->GetRadius();
	      if (r < 0.35f) count++;
	   }
	   return count;
	}

	/**
	 * Calculates the sum of 3 biggest angles between neighboring hits.
	 * \param[in] ring Found and fitted ring.
	 */
	float GetAngle(
	      CbmRichRingLight* ring)
	{
	   int nHits = ring->GetNofHits();
	   if (nHits > kMAX_NOF_HITS) return 0.2f;
      if (nHits < 4) return 999.f;

      float Pi = 3.14159265;
      float TwoPi = 2.*3.14159265;
      float xRing = ring->GetCenterX();
      float yRing = ring->GetCenterY();
      float xHit, yHit;

      for(int iH = 0; iH < nHits; iH++){
         CbmRichHitLight hit = ring->GetHit(iH);
         xHit = hit.fX;
         yHit = hit.fY;

         if (yHit-yRing == 0 || xHit-xRing == 0) continue;

         if(xHit > xRing){
            if (yHit > yRing){
               fAlpha[iH] = atan(fabs((yHit-yRing)/(xHit-xRing)));
            } else{
               fAlpha[iH] = TwoPi - atan(fabs((yHit-yRing)/(xHit-xRing)));
            }
         }else {
            if (yHit > yRing){
               fAlpha[iH] = Pi - atan(fabs((yHit-yRing)/(xHit-xRing)));
            }else {
               fAlpha[iH] = Pi + atan(fabs((yHit-yRing)/(xHit-xRing)));
            }
         }
      }

      sort(fAlpha.begin(),fAlpha.begin()+nHits);

      for(int i = 0; i < nHits-1; i++) fPhi[i] = fAlpha[i+1] - fAlpha[i];
      fPhi[nHits-1] = TwoPi - fAlpha[nHits-1] + fAlpha[0];
      sort(fPhi.begin(),fPhi.begin()+nHits);

      float angle = fPhi[nHits-1]+fPhi[nHits-2]+fPhi[nHits-3];

      return angle;
	}

protected:
    std::vector<float> fAlpha;
    std::vector<float> fPhi;
};


#endif
