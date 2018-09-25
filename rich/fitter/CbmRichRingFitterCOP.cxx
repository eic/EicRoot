/**
* \file CbmRichRingFitterCOP.cxx
*
* \author Alexander Ayriyan, Gennadi Ososkov, Semen Lebedev <s.lebedev@gsi.de>
* \date 2005
**/
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingLight.h"

#include <iostream>
#include <cmath>

using namespace std;

CbmRichRingFitterCOP::CbmRichRingFitterCOP()
{

}

CbmRichRingFitterCOP::~CbmRichRingFitterCOP()
{

}

void CbmRichRingFitterCOP::DoFit(
      CbmRichRingLight *ring)
{
	FitRing(ring);
}

void CbmRichRingFitterCOP::FitRing(
      CbmRichRingLight* ring)
{
   int nofHits = ring->GetNofHits();
   if (nofHits < 3) {
      ring->SetRadius(0.);
      ring->SetCenterX(0.);
      ring->SetCenterY(0.);
      return;
   }

   if (nofHits >= MAX_NOF_HITS_IN_RING) {
		cout << "-E- CbmRichRingFitterCOP::DoFit(), too many hits in the ring:" << nofHits <<endl;
		ring->SetRadius(0.);
		ring->SetCenterX(0.);
		ring->SetCenterY(0.);
		return;
	}
	int iterMax = 4;
	float Xi, Yi, Zi;
	float M0, Mx, My, Mz, Mxy, Mxx, Myy, Mxz, Myz, Mzz, Mxz2, Myz2, Cov_xy;
	float A0, A1, A2, A22;
	float epsilon = 0.00001;
	float Dy, xnew, xold, ynew, yold = 10000000.;

	M0 = nofHits;
	Mx = My = 0.;

	// calculate center of gravity
	for (int i = 0; i < nofHits; i++) {
		Mx += ring->GetHit(i).fX;
		My += ring->GetHit(i).fY;
	}
	Mx /= M0;
	My /= M0;

	// computing moments (note: all moments are normed, i.e. divided by N)
	Mxx = Myy = Mxy = Mxz = Myz = Mzz = 0.;

	for (int i = 0; i < nofHits; i++) {
	   // transform to center of gravity coordinate system
		Xi = ring->GetHit(i).fX - Mx;
		Yi = ring->GetHit(i).fY - My;
		Zi = Xi * Xi + Yi * Yi;

		Mxy += Xi * Yi;
		Mxx += Xi * Xi;
		Myy += Yi * Yi;
		Mxz += Xi * Zi;
		Myz += Yi * Zi;
		Mzz += Zi * Zi;
	}
	Mxx /= M0;
	Myy /= M0;
	Mxy /= M0;
	Mxz /= M0;
	Myz /= M0;
	Mzz /= M0;

	//computing the coefficients of the characteristic polynomial
	Mz = Mxx + Myy;
	Cov_xy = Mxx * Myy - Mxy * Mxy;
	Mxz2 = Mxz * Mxz;
	Myz2 = Myz * Myz;

	A2 = 4. * Cov_xy - 3. * Mz * Mz - Mzz;
	A1 = Mzz * Mz + 4. * Cov_xy * Mz - Mxz2 - Myz2 - Mz * Mz * Mz;
	A0 = Mxz2 * Myy + Myz2 * Mxx - Mzz * Cov_xy - 2. * Mxz * Myz * Mxy + Mz
			* Mz * Cov_xy;

	A22 = A2 + A2;
	xnew = 0.;

	//Newton's method starting at x=0
	int iter;
	for (iter = 0; iter < iterMax; iter++) {
		ynew = A0 + xnew * (A1 + xnew * (A2 + 4. * xnew * xnew));

		if (fabs(ynew) > fabs(yold)) {
			//  printf("Newton2 goes wrong direction: ynew=%f  yold=%f\n",ynew,yold);
			xnew = 0.;
			break;
		}

		Dy = A1 + xnew * (A22 + 16. * xnew * xnew);
		xold = xnew;
		xnew = xold - ynew / Dy;
		//cout << " xnew = " << xnew ;
		if (xnew == 0 || fabs((xnew - xold) / xnew) < epsilon){
			//cout << "iter = " << iter << " N = " << fNhits << endl;
			break;
		}
	}

	//if (iter == iterMax - 1) {
		//  printf("Newton2 does not converge in %d iterations\n",iterMax);
	//	xnew = 0.;
	//}

   float radius = 0.;
   float centerX = 0.;
   float centerY = 0.;

	//computing the circle parameters
	float GAM = -Mz - xnew - xnew;
	float DET = xnew * xnew - xnew * Mz + Cov_xy;
	if (DET != 0.) {
		centerX = (Mxz * (Myy - xnew) - Myz * Mxy) / DET / 2.;
		centerY = (Myz * (Mxx - xnew) - Mxz * Mxy) / DET / 2.;
		radius = sqrt(centerX * centerX + centerY * centerY - GAM);
		centerX += Mx;
		centerY += My;
	}

	ring->SetRadius(radius);
	ring->SetCenterX(centerX);
	ring->SetCenterY(centerY);

	CalcChi2(ring);
}
