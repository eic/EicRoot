/**
* \file CbmRichRingFitterCircle.cxx
*
* \author Supriya Das
* \date 2006
**/

#include "CbmRichRingFitterCircle.h"
#include "CbmRichRingLight.h"

CbmRichRingFitterCircle::CbmRichRingFitterCircle()
{

}

CbmRichRingFitterCircle::~CbmRichRingFitterCircle()
{

}

void CbmRichRingFitterCircle::DoFit(
      CbmRichRingLight* ring)
{
   int nofHits = ring->GetNofHits();
   if (nofHits < 3) return;

   float c[3], a[3][3];

   float b1 = 0.f;
   float b2 = 0.f;
   float b3 = 0.f;

   float b12 = 0.f;
   float b22 = 0.f;
   float b32 = nofHits;

   float a11 = 0.f;
   float a12 = 0.f;
   float a13 = 0.f;
   float a21 = 0.f;
   float a22 = 0.f;
   float a23 = 0.f;
   float a31 = 0.f;
   float a32 = 0.f;
   float a33 = 0.f;

   float meanX = 0.f;
   float meanY = 0.f;

   for (int iHit = 0; iHit < nofHits; iHit++) {
      float hx = ring->GetHit(iHit).fX;
      float hy = ring->GetHit(iHit).fY;

      b1 += (hx*hx + hy*hy) * hx;
      b2 += (hx*hx + hy*hy) * hy;
      b3 += (hx*hx + hy*hy);

      b12 += hx;
      b22 += hy;

      a11 += 2*hx*hx;
      a12 += 2*hx*hy;
      a22 += 2*hy*hy;

      meanX += hx;
      meanY += hy;
   }

   if (nofHits != 0) {
      meanX = meanX/(float)(nofHits);
      meanY = meanY/(float)(nofHits);
   }

   a21 = a12;

   a13 = b12;
   a23 = b22;

   a31 = 2*b12;
   a32 = 2*b22;
   a33 = b32;

   c[0] = b1*b22 - b2*b12;
   c[1] = b1*b32 - b3*b12;
   c[2] = b2*b32 - b3*b22;

   a[0][0] = a11*b22 - a21*b12;
   a[1][0] = a12*b22 - a22*b12;
   a[2][0] = a13*b22 - a23*b12;

   a[0][1] = a11*b32 - a31*b12;
   a[1][1] = a12*b32 - a32*b12;
   a[2][1] = a13*b32 - a33*b12;

   a[0][2] = a21*b32-a31*b22;
   a[1][2] = a22*b32-a32*b22;
   a[2][2] = a23*b32-a33*b22;

   float det1 = a[0][0]*a[1][1] - a[0][1]*a[1][0];

   float x11 = (c[0]*a[1][1] - c[1]*a[1][0]) / det1;
   float x21 = (a[0][0]*c[1] - a[0][1]*c[0]) / det1;

//   Float_t det2 = a[0][1]*a[1][2] - a[0][2]*a[1][1];
//   Float_t det3 = a[0][0]*a[1][2] - a[0][2]*a[1][0];
//   Float_t x12 = (c[1]*a[1][2] - c[2]*a[1][1])/det2;
//   Float_t x22 = (a[0][1]*c[2] - a[0][2]*c[1])/det2;

   float radius = sqrt((b3 + b32*(x11*x11 + x21*x21) - a31*x11 - a32*x21)/a33);
   float centerX = x11;
   float centerY = x21;

   ring->SetRadius(radius);
   ring->SetCenterX(centerX);
   ring->SetCenterY(centerY);

   //if (TMath::IsNaN(radius) == 1) ring->SetRadius(0.);
   //if (TMath::IsNaN(centerX) == 1) ring->SetCenterX(0.);
   //if (TMath::IsNaN(centerY) == 1) ring->SetCenterY(0.);

   CalcChi2(ring);
}
