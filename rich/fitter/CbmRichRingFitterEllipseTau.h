/**
* \file CbmRichRingFitterEllipseTau.h
*
* \brief Here the ring is fitted with Taubin algorithm from
*  A. Ayriyan, G. Ososkov, N. Chernov
*
* \author Alexander Ayriyan and Semen Lebedev <s.lebedev@gsi.de>
* \date 2011
**/

#ifndef CBM_RICH_RING_FITTER_ELLIPSE_TAU
#define CBM_RICH_RING_FITTER_ELLIPSE_TAU

#include <vector>
#include "CbmRichRingFitterEllipseBase.h"
#include "TMath.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TMatrixDEigen.h"

#include <iostream>

using std::vector;


/**
* \class CbmRichRingFitterEllipseTau
*
* \brief Here the ring is fitted with Taubin algorithm from
*  A. Ayriyan, G. Ososkov, N. Chernov
*
* \author Alexander Ayriyan and Semen Lebedev <s.lebedev@gsi.de>
* \date 2011
**/
class CbmRichRingFitterEllipseTau : public CbmRichRingFitterEllipseBase
{
public:

   /**
    *\brief Default constructor.
    */
   CbmRichRingFitterEllipseTau();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingFitterEllipseTau();

   /**
    * \brief Inherited from CbmRichRingFitterBase.
    */
   virtual void DoFit(
         CbmRichRingLight *ring);

private:

   double fM[36];
   double fP[25];
   double fQ[25];
   double fZ[MAX_NOF_HITS_IN_RING*6];
   double fZT[MAX_NOF_HITS_IN_RING*6];
   double fAlgPar[6];

   /**
    * \brief Transform fitted curve to ellipse parameters.
    * \param[in,out] ring RICH ring.
    */
	void TransformEllipse(
	      CbmRichRingLight* ring);

	/**
	 * \brief Initialize all matrices.
	 */
	void InitMatrices(
	      CbmRichRingLight* ring);

	/**
	 * \brief Perform Taubin method.
	 */
	void Taubin();

	/**
	 * \brief Invert 5x5 matrix.
	 */
	void Inv5x5();

	/**
	 * \brief Matrices multiplication.
	 */
	void AMultB(
	      const double * const ap,
	      int na,
	      int ncolsa,
	      const double * const bp,
	      int nb,
	      int ncolsb,
	      double *cp);

	/**
	 * \brief Jacobi method.
	 */
	void Jacobi(
	      double a[5][5],
	      double d[5],
	      double v[5][5]);

	/**
	 * \brief Find eigenvalues.
	 */
	void Eigsrt(
	      double d[5],
	      double v[5][5]);
};

#endif
