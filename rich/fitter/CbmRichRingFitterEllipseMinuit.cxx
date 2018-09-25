/**
* \file CbmRichRingFitterEllipseMinuit.cxx
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2011
**/

#include "CbmRichRingFitterEllipseMinuit.h"

using std::endl;
using std::cout;

CbmRichRingFitterEllipseMinuit::CbmRichRingFitterEllipseMinuit()
{

}

CbmRichRingFitterEllipseMinuit::~CbmRichRingFitterEllipseMinuit()
{

}

void CbmRichRingFitterEllipseMinuit::DoFit(
      CbmRichRingLight *ring)
{
   int nofHits = ring->GetNofHits();

   if (nofHits <= 5) {
      ring->SetXYABP(-1.,-1.,-1., -1., -1.);
      ring->SetRadius(-1.);
      return;
   }

   vector<double> hitX;
   vector<double> hitY;
   hitX.clear();
   hitY.clear();
   for(int i = 0; i < nofHits; i++) {
      hitX.push_back(ring->GetHit(i).fX);
      hitY.push_back(ring->GetHit(i).fY);
   }

   vector<double> fpar = DoFit(hitX, hitY);

   TransformToRichRing(ring, fpar);

   CalcChi2(ring);
}

void CbmRichRingFitterEllipseMinuit::TransformToRichRing(
      CbmRichRingLight* ring,
      const vector<double>& fpar)
{
   // calculate center of the ellipse
   double xc = (fpar[0] + fpar[2])/2.;
   double yc = (fpar[1] + fpar[3])/2.;
   // calculate length of b axes
   double p1 = (fpar[0] - fpar[2])*(fpar[0] - fpar[2]) + (fpar[1] - fpar[3])*(fpar[1] - fpar[3]);
   if (p1 < 0.){
      ring->SetXYABP(-1.,-1.,-1., -1., -1.);
      ring->SetRadius(-1.);
      return;
   }

   double c = sqrt ( p1 ) / 2.;
   double p2 = fpar[4]*fpar[4] - c*c;
   if (p2 < 0.){
      ring->SetXYABP(-1.,-1.,-1., -1., -1.);
      ring->SetRadius(-1.);
      return;
   }
   double b = sqrt ( p2 );
   // calculate angle
   double p3 = fpar[2] - fpar[0];
   if (p3 == 0.){
      ring->SetXYABP(-1.,-1.,-1., -1., -1.);
      ring->SetRadius(-1.);
      return;
   }
   double k = (fpar[3] - fpar[1]) / (fpar[2] - fpar[0]);
   double ang = atan(k);

   ring->SetXYABP(xc, yc, fpar[4], b, ang);
   ring->SetRadius((b + fpar[4])/2.);
}

vector<double> CbmRichRingFitterEllipseMinuit::DoFit(
      const vector<double>& x,
      const vector<double>& y)
{
   FCNEllipse *theFCN = new FCNEllipse(x, y);

   // create initial starting values for parameters
   double xf1 = 0.;
   double yf1 = 0.;
   for(int i = 0; i < x.size(); i++) {
      xf1 += x[i];
      yf1 += y[i];
   }
   double a = 5.;
   xf1 = xf1 / x.size() - a;
   yf1 = yf1 / x.size();
   double xf2 = xf1 + a;
   double yf2 = yf1;

   TFitterMinuit theMinuit;
   theMinuit.SetPrintLevel(-1);
   theMinuit.SetMinuitFCN(theFCN);
   theMinuit.SetParameter(0, "xf1", xf1, 0.1, 1., -1.);
   theMinuit.SetParameter(1, "yf1", yf1, 0.1, 1., -1.);
   theMinuit.SetParameter(2, "xf2", xf2, 0.1, 1., -1.);
   theMinuit.SetParameter(3, "yf2", yf2, 0.1, 1., -1.);
   theMinuit.SetParameter(4, "a", a, 0.1, 1., -1.);
   theMinuit.CreateMinimizer();
   theMinuit.Minimize();
   vector<double> fpar;
   fpar.clear();
   for (int i = 0; i < 5; i++){
      fpar.push_back(theMinuit.GetParameter(i));
   }
   return fpar;
}
