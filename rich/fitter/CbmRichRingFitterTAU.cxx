/**
* \file CbmRichRingFitterTAU.cxx
*
* \author Alexander Ayriyan, Gennadi Ososkov, Claudia Hoehne, Semen Lebedev <s.lebedev@gsi.de>
* \date 2012
**/
#include "CbmRichRingFitterTAU.h"

#include <vector>
#include <iostream>
#include <cmath>

using std::cout;
using std::endl;
using std::vector;
using std::pow;
using std::fabs;
using std::sqrt;

CbmRichRingFitterTAU::CbmRichRingFitterTAU():
      fRobust(0)
{

}

CbmRichRingFitterTAU::~CbmRichRingFitterTAU()
{
}

void CbmRichRingFitterTAU::DoFit(
      CbmRichRingLight* ring)
{
   int nofHits = ring->GetNofHits();

   double radius  = -1.;
   double centerX = -1.;
   double centerY = -1.;

   if ( nofHits < 3){
      ring->SetRadius(0.);
      ring->SetCenterX(0.);
      ring->SetCenterY(0.);
      return;
   }

   int iter, iterMax = 20;

   int riterMax = 4;

   double Xi,Yi,Zi;
   double M0, Mx, My, Mz, Mxy, Mxx, Myy, Mxz, Myz, Mzz, Mxz2, Myz2, Cov_xy;
   double A0,A1,A2,A22,A3,A33, epsilon=0.000000000001;
   double Dy,xnew,xold,ynew,yold=100000000000.;
   double GAM,DET;
   //double Xcenter = -1.,Ycenter = -1.,Radius = -1.;

   double sigma;
   double ctsigma;
   double dist,weight;
   const double ct = 3.;
   const double copt = 0.2;
   const double zero = 0.0001;
   const double SigmaMin = 0.18;
   double amax = -100000;
   double amin =  100000;
   double sig1 = 0.;
   double sig2 = 0.;

   vector<double> x;	// x coordinates of hits;
   vector<double> y;	// y coordinates of hits;
   vector<double> a;	// amplitudes of hits;
   vector<double> w;	// weights of points;
   vector<double> d;	// distance between points and circle;

   if(fRobust < 1 || fRobust > 2){
      riterMax = 1;
   }
   if(fRobust == 1){
      riterMax = 4;
   }
   if(fRobust == 2){
      riterMax = 4;
   }

   for(int i = 0; i < nofHits; i++) {
      x.push_back(ring->GetHit(i).fX);
      y.push_back(ring->GetHit(i).fY);
      a.push_back(1.);
   }

   for(int i = 0; i < nofHits; i++){
      if (a[i] > amax) amax = a[i];
      if (a[i] < amin) amin = a[i];
   }

   for(int i = 0; i < nofHits; i++) w.push_back(1.);

   for(int riter = 0; riter < riterMax; riter++){
      M0 = 0;
      Mx=My=0.;

      for(int i = 0; i < nofHits; i++) {
         Mx += x[i]*w[i];
         My += y[i]*w[i];
         M0 += w[i];
      }

      Mx /= M0;
      My /= M0;

      //computing moments (note: all moments are normed, i.e. divided by N)
      Mxx=Myy=Mxy=Mxz=Myz=Mzz=0.;

      for (int i = 0; i < nofHits; i++) {
         Xi = x[i] - Mx;
         Yi = y[i] - My;
         Zi = Xi*Xi + Yi*Yi;

         Mxy += Xi*Yi*w[i];
         Mxx += Xi*Xi*w[i];
         Myy += Yi*Yi*w[i];
         Mxz += Xi*Zi*w[i];
         Myz += Yi*Zi*w[i];
         Mzz += Zi*Zi*w[i];
      }
      Mxx /= M0;
      Myy /= M0;
      Mxy /= M0;
      Mxz /= M0;
      Myz /= M0;
      Mzz /= M0;

      // computing the coefficients of the characteristic polynomial
      Mz = Mxx + Myy;
      Cov_xy = Mxx*Myy - Mxy*Mxy;
      Mxz2 = Mxz*Mxz;
      Myz2 = Myz*Myz;

      A3 = 4.*Mz;
      A2 = - 3.*Mz*Mz - Mzz;
      A1 = Mzz*Mz + 4.*Cov_xy*Mz - Mxz2 - Myz2 - Mz*Mz*Mz;
      A0 = Mxz2*Myy + Myz2*Mxx - Mzz*Cov_xy - 2.*Mxz*Myz*Mxy+Mz*Mz*Cov_xy;

      A22 = A2 + A2;
      A33 = A3 + A3 + A3;
      iter = 0;
      xnew = 0.;

      // Newton's method starting at x=0
      for (iter=0; iter<iterMax; iter++) {
         ynew = A0 + xnew*(A1 + xnew*(A2 + xnew*A3));

         if (fabs(ynew)>fabs(yold)) {
            xnew = 0.;
            break;
         }

         Dy = A1 + xnew*(A22 + xnew*A33);
         xold = xnew;
         xnew = xold - ynew/Dy;

         if (fabs((xnew-xold)/xnew) < epsilon) break;
      }

      if (iter == iterMax-1) {
         //printf("Newton3 does not converge in %d iterations\n",iterMax);
         xnew = 0.;
      }
      if (xnew<0.) {
         iter=30;
         // printf("Negative root:  x=%f\n",xnew);
      }

      // computing the circle parameters
      GAM = - Mz;
      DET = xnew*xnew - xnew*Mz + Cov_xy;
      centerX = (Mxz*(Myy-xnew) - Myz*Mxy)/DET/2.;
      centerY = (Myz*(Mxx-xnew) - Mxz*Mxy)/DET/2.;
      radius = sqrt(centerX * centerX + centerY * centerY - GAM);
      centerX = (Mxz*(Myy - xnew) - Myz*Mxy)/DET/2. + Mx;
      centerY = (Myz*(Mxx - xnew) - Mxz*Mxy)/DET/2. + My;

      if(riter < riterMax - 1){
         for(int i = 0; i < nofHits; i++){
            dist = sqrt(pow((centerX - x[i]), 2) + pow((centerY - y[i]), 2)) - radius;
            dist = fabs(dist);
            d.push_back(dist);
         }

         for(int i = 0; i < d.size(); i++){
            sig1 += w[i]*d[i]*d[i];
            sig2 += w[i];
         }
         sigma = sqrt(sig1/sig2);
         if(sigma < SigmaMin) sigma = SigmaMin;
         ctsigma = ct*sigma;
         sig1 = 0.;
         sig2 = 0.;

         w.clear();
         for(int i = 0; i < d.size(); i++){
            if(fRobust == 1){	//! Tukey's weight function
               if(d[i] <= ctsigma){
                  weight = pow((1 - pow((d[i]/ctsigma), 2)), 2);
                  if(weight < zero) weight = zero;
               }else{
                  weight = zero;
               }
               w.push_back(weight);
            }
            if(fRobust == 2 || fRobust == 3){ //! Optimal weight function
               sigma *= 2;
               weight = 1 + copt;
               weight /= 1 + copt*exp(pow((d[i]/sigma), 2));
               if(weight < zero) weight = zero;
               w.push_back(weight);
            }
         }
         d.clear();
      }
   }
   x.clear();
   y.clear();
   a.clear();
   w.clear();

   ring->SetRadius(radius);
   ring->SetCenterX(centerX);
   ring->SetCenterY(centerY);

   CalcChi2(ring);
}
