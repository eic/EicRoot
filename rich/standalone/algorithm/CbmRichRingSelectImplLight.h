/*
 * CbmRichRingSelectImplLight.h
 *
 *  Created on: 24.03.2010
 *      Author: slebedev
 */

#ifndef CBMRICHRINGSELECTIMPLLIGHT_H_
#define CBMRICHRINGSELECTIMPLLIGHT_H_

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "CbmRichRingLight.h"

class CbmRichRingSelectImplLight
{
public:
	int kMAX_NOF_HITS;

	 CbmRichRingSelectImplLight() {
		 kMAX_NOF_HITS = 100;
		 fAlpha.resize(kMAX_NOF_HITS);
		 fPhi.resize(kMAX_NOF_HITS);
	 };
	 ~CbmRichRingSelectImplLight() { };

	 int GetNofHitsOnRingCircle(CbmRichRingLight* ring);
	 float GetAngle(CbmRichRingLight* ring);
protected:
    std::vector<float> fAlpha;
    std::vector<float> fPhi;
};


inline int CbmRichRingSelectImplLight::GetNofHitsOnRingCircle(CbmRichRingLight* ring)
{
    int count = 0;
	int nHits = ring->GetNofHits();
	for (int iH = 0; iH < nHits; iH++) {
		CbmRichHitLight hitRing = ring->GetHit(iH);
		float rx = hitRing.fX - ring->GetCenterX();
		float ry = hitRing.fY - ring->GetCenterY();
		float r = sqrt(rx * rx + ry * ry) - ring->GetRadius();
		if (r < 0.35f)	count++;
	}
	return count;
}

inline float CbmRichRingSelectImplLight::GetAngle(CbmRichRingLight* ring)
{
    register int nHits = ring->GetNofHits();
	if (nHits > kMAX_NOF_HITS) return 0.2f;
	if (nHits < 4) return 999.f;

	register float Pi = 3.14159265;
	register float TwoPi = 2.*3.14159265;
    register float xRing = ring->GetCenterX();
    register float yRing = ring->GetCenterY();
    register float xHit, yHit;

    for(int iH = 0; iH < nHits; iH++){
		CbmRichHitLight hit = ring->GetHit(iH);
		xHit = hit.fX;
		yHit = hit.fY;

		if (yHit-yRing == 0 || xHit-xRing == 0) continue;

		if( xHit > xRing){
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
#endif /* CBMRICHRINGSELECTIMPLLIGHT_H_ */
