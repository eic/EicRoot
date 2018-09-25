/**
* \file CbmRichRingFitterQa.h
*
* \brief Test ellipse and circle fitting on toy model.
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2009
**/

#ifndef CBM_RICH_RING_FITTER_QA
#define CBM_RICH_RING_FITTER_QA

#include "TObject.h"
#include <vector>
#include "TMatrixD.h"
class TH1D;
class CbmRichRingLight;
//class TMatrixD;

using std::vector;

/**
* \class CbmRichRingFitterQa
*
* \brief Test ellipse and circle fitting on toy model.
*
* \author Semen Lebedev <s.lebedev@gsi.de>
* \date 2009
**/
class CbmRichRingFitterQa: public TObject
{
public:

   /**
    * \brief Standard constructor.
    */
 	CbmRichRingFitterQa();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingFitterQa();

 	/**
 	 * \brief Generate ellipse.
 	 */
	void GenerateEllipse();

	/**
	 * \brief Draw generated and fitted circle/ellipse.
	 */
	void Draw();

private:
	// ellipse fitting algorithm, errors
   TH1D* fhErrorA;
   TH1D* fhErrorB;
   TH1D* fhErrorX;
   TH1D* fhErrorY;
   TH1D* fhErrorPhi;
   // ellipse fitting algorithm, parameters
   TH1D* fhA;
   TH1D* fhB;
   TH1D* fhX;
   TH1D* fhY;
   TH1D* fhPhi;
   // circle fitting algorithm, errors
   TH1D* fhRadiusErr;
   TH1D* fhCircleXcErr;
   TH1D* fhCircleYcErr;
   // circle fitting algorithm, parameters
   TH1D* fhRadius;
   TH1D* fhCircleXc;
   TH1D* fhCircleYc;
   // circle fitting algorithm, pools
   TH1D* fhRadiusPool;
   TH1D* fhCircleXcPool;
   TH1D* fhCircleYcPool;

	/**
	 * \Calculate errors of the fit.
	 */
	void CalculateFitErrors(
	      CbmRichRingLight* ring,
	      Double_t sigma,
	      TMatrixD& cov);

   /**
    * \brief Copy constructor.
    */
   CbmRichRingFitterQa(const CbmRichRingFitterQa&);

   /**
    * \brief Assignment operator.
    */
   CbmRichRingFitterQa& operator=(const CbmRichRingFitterQa&);

	ClassDef(CbmRichRingFitterQa, 1);
};
#endif
