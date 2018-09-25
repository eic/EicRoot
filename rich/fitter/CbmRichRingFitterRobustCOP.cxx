/**
* \file CbmRichRingFitterRobustCOP.cxx
*
* \brief Here the ring is fitted with the RobustCOP algorithm from A. Ayriyan/ G. Ososkov.
*
* \author Alexander Ayriyan, Gennadi Ososkov, Claudia Hoehne, Semen Lebedev <s.lebedev@gsi.de>
* \date 2012
**/
#include "CbmRichRingFitterRobustCOP.h"

#include <iostream>
#include <cmath>

using std::cout;
using std::endl;
using std::sqrt;
using std::fabs;

CbmRichRingFitterRobustCOP::CbmRichRingFitterRobustCOP()
{

}

CbmRichRingFitterRobustCOP::~CbmRichRingFitterRobustCOP()
{

}

void CbmRichRingFitterRobustCOP::DoFit(
      CbmRichRingLight *ring)
{
   int nofHits = ring->GetNofHits();

   double radius = 0.;
   double centerX = 0.;
   double centerY = 0.;

   if (nofHits < 3){
      ring->SetCenterX(0.);
      ring->SetCenterY(0.);
      ring->SetRadius(0.);
      return;
   }

   int iter,iterMax=20;
   int i_iter, i_max_Robust = 4;
   const int MinNuberOfHits = 3;
   const int MaxNuberOfHits = 2000;
   double Xi,Yi,Zi;
   double M0,Mx,My,Mz,Mxy,Mxx,Myy,Mxz,Myz,Mzz,Mxz2,Myz2,Cov_xy;//,temp;
   double A0,A1,A2,A22,epsilon=0.000000000001;
   double Dy,xnew,xold,ynew,yold=100000000000.;
   double GAM,DET;
   double SumS1 = 0., SumS2 = 0.;
   double sigma;
   double ctsigma;
   double sigma_min = 0.05;
   double dx, dy;
   double d[MaxNuberOfHits];
   double w[MaxNuberOfHits];
   double ct = 7.;

   for(int i = 0; i < MaxNuberOfHits; i++) w[i] = 1.;

   Mx=My=0.;
   for(int i = 0; i < nofHits; i++) {
      Mx += ring->GetHit(i).fX;
      My += ring->GetHit(i).fY;
   }

   M0 = nofHits;
   Mx /= M0;
   My /= M0;

   for(i_iter = 0; i_iter < i_max_Robust; i_iter++){
      sigma = sigma_min;
      if(i_iter != 0){
         for(int i = 0; i < nofHits; i++){
            dx = Mx - ring->GetHit(i).fX;
            dy = My - ring->GetHit(i).fY;
            d[i] = sqrt(dx*dx + dy*dy) - radius;
            SumS1 += w[i]*d[i]*d[i];
            SumS2 += w[i];
         }
         if(SumS2 == 0.){ sigma = sigma_min; } else{ sigma = sqrt(SumS1/SumS2);}
         if(sigma < sigma_min) sigma = sigma_min;
         ctsigma = ct*sigma;
         SumS1 = 0.;
         SumS2 = 0.;
         for(int i = 0; i < nofHits; i++){
            if(d[i] <= ctsigma){
               w[i] = (1 - (d[i]/(ctsigma))*(d[i]/(ctsigma)))*(1 - (d[i]/(ctsigma))*(d[i]/(ctsigma)));
            }else{w[i] = 0.;}
         }
      }
      //computing moments (note: all moments are normed, i.e. divided by N)
      M0 = 0;
      Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;
      for (int i = 0; i < nofHits; i++) {
         if(w[i] != 0.){
            Xi = ring->GetHit(i).fX - Mx;
            Yi = ring->GetHit(i).fY - My;
            Zi = Xi*Xi + Yi*Yi;
            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
            M0++;
         }
      }
      if(M0 < MinNuberOfHits){
         M0 = 0;
         Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;
         M0 = 0;
         for (int i = 0; i < nofHits; i++) {
            Xi = ring->GetHit(i).fX - Mx;
            Yi = ring->GetHit(i).fY - My;
            Zi = Xi*Xi + Yi*Yi;
            Mxy += Xi*Yi;
            Mxx += Xi*Xi;
            Myy += Yi*Yi;
            Mxz += Xi*Zi;
            Myz += Yi*Zi;
            Mzz += Zi*Zi;
            M0++;
         }
      }
      Mxx /= M0;
      Myy /= M0;
      Mxy /= M0;
      Mxz /= M0;
      Myz /= M0;
      Mzz /= M0;

      //computing the coefficients of the characteristic polynomial
      Mz = Mxx + Myy;
      Cov_xy = Mxx*Myy - Mxy*Mxy;
      Mxz2 = Mxz*Mxz;
      Myz2 = Myz*Myz;

      A2 = 4.*Cov_xy - 3.*Mz*Mz - Mzz;
      A1 = Mzz*Mz + 4.*Cov_xy*Mz - Mxz2 - Myz2 - Mz*Mz*Mz;
      A0 = Mxz2*Myy + Myz2*Mxx - Mzz*Cov_xy - 2.*Mxz*Myz*Mxy + Mz*Mz*Cov_xy;

      A22 = A2 + A2;
      iter = 0;
      xnew = 0.;

      //Newton's method starting at x=0
      for(iter = 0; iter < iterMax; iter++) {
         ynew = A0 + xnew*(A1 + xnew*(A2 + 4.*xnew*xnew));

         if (fabs(ynew)>fabs(yold))
         {
         //  printf("Newton2 goes wrong direction: ynew=%f  yold=%f\n",ynew,yold);
            xnew = 0.;
            break;
         }

         Dy = A1 + xnew*(A22 + 16.*xnew*xnew);
         xold = xnew;
         xnew = xold - ynew/Dy;

         if (fabs((xnew-xold)/xnew) < epsilon) break;
      }

      if (iter == iterMax-1) {
         //printf("Newton2 does not converge in %d iterations\n",iterMax);
         xnew = 0.;
      }
      if (xnew<0.) {
         iter = 30;
         //printf("Negative root:  x=%f\n",xnew);
      }

      // computing the circle parameters
      GAM = - Mz - xnew - xnew;
      DET = xnew*xnew - xnew*Mz + Cov_xy;
      centerX = (Mxz*(Myy-xnew) - Myz*Mxy)/DET/2.;
      centerY = (Myz*(Mxx-xnew) - Mxz*Mxy)/DET/2.;
      radius = sqrt(centerX * centerX + centerY * centerY - GAM);
      centerX = centerX + Mx;
      centerY = centerY + My;
      Mx = centerX;
      My = centerY;

      if (DET == 0.) {
         radius = 0.;
         centerX = 0.;
         centerY = 0.;
         return;
      }
   }
   ring->SetRadius(radius);
   ring->SetCenterX(centerX);
   ring->SetCenterY(centerY);

   CalcChi2(ring);
}
