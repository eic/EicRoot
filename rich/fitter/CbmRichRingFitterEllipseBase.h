/**
* \file CbmRichRingFitterEllipseBase.h
*
* \brief Base class for concrete ellipse fitting algorithms.
*  Each derived class must implement the method DoFit.
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2011
**/

#ifndef CBM_RICH_RING_FITTER_ELLIPSE_BASE
#define CBM_RICH_RING_FITTER_ELLIPSE_BASE

#include "CbmRichRingFitterBase.h"

/**
* \class CbmRichRingFitterEllipseBase
*
* \brief Base class for concrete ellipse fitting algorithms.
*  Each derived class must implement the method DoFit.
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2011
**/
class CbmRichRingFitterEllipseBase: public CbmRichRingFitterBase {
public:
   /**
    * \brief Default constructor.
    */
   CbmRichRingFitterEllipseBase(){ };

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingFitterEllipseBase(){ };

protected:

   /**
    * \brief Calculate chi2 of the ellipse fit.
    * \param[in,out] ring Fitted RICH ring with ellipse fitter.
    */
   virtual void CalcChi2(
         CbmRichRingLight* ring)
   {
      int nofHits = ring->GetNofHits();
      if (nofHits <= 5){
         ring->SetChi2(-1.);
         return;
      }

      double axisA = ring->GetAaxis();
      double axisB = ring->GetBaxis();

      if (axisA < axisB){
         ring->SetChi2(-1.);
         return;
      }

      // calculate ellipse focuses
      double xf1 = ring->GetXF1();
      double yf1 = ring->GetYF1();
      double xf2 = ring->GetXF2();
      double yf2 = ring->GetYF2();

      // calculate chi2
      double chi2 = 0.;
      for(int iHit = 0; iHit < nofHits; iHit++){
         double x = ring->GetHit(iHit).fX;
         double y = ring->GetHit(iHit).fY;

         double d1 = sqrt( (x-xf1)*(x-xf1) + (y-yf1)*(y-yf1) );
         double d2 = sqrt( (x-xf2)*(x-xf2) + (y-yf2)*(y-yf2) );

         chi2 += (d1 + d2 - 2.*axisA)*(d1 + d2 - 2.*axisA);
      }
      ring->SetChi2(chi2);
   }

   /**
    * \brief Calculate chi2 of the ellipse fitting using parameters of 2D curve.
    * \param[in] A A parameter of curve.
    * \param[in] B B parameter of curve.
    * \param[in] C C parameter of curve.
    * \param[in] D D parameter of curve.
    * \param[in] E E parameter of curve.
    * \param[in] F F parameter of curve.
    * \param[in] ring Fitted RICH ring with ellipse fitter.
    */
   virtual void CalcChi2(
         double A,
         double B,
         double C,
         double D,
         double E,
         double F,
         CbmRichRingLight* ring)
   {
      int nofHits =  ring->GetNofHits();
      if (nofHits <= 5){
         ring->SetChi2(-1.);
         return;
      }
      double chi2 = 0.;
      for(int iHit = 0; iHit < nofHits; iHit++){
         double x = ring->GetHit(iHit).fX;
         double y = ring->GetHit(iHit).fY;

         double d1 = fabs(A*x*x + B*x*y + C*y*y + D*x + E*y + F);
         double d2 = sqrt( pow(2*A*x + B*y + D, 2) + pow(B*x + 2*C*y + E, 2) );

         chi2 += (d1*d1)/(d2*d2);
      }
      ring->SetChi2(chi2);
   }

};

#endif /* CBMRICHRINGFITTERELLIPSEBASEH */
