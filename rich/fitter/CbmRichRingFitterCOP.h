/**
* \file CbmRichRingFitterCOP.h
*
* \brief Here the ring is fitted with the COP algorithm from A. Ayriyan/G. Ososkov.
*
* \author Alexander Ayriyan, Gennadi Ososkov, Semen Lebedev <s.lebedev@gsi.de>
* \date 2005
**/
#ifndef CBM_RICH_RING_FITTER_COP
#define CBM_RICH_RING_FITTER_COP

#include "CbmRichRingFitterBase.h"

#include <iostream>
#include <cmath>

using std::cout;
using std::endl;

/**
* \class CbmRichRingFitterCOP
*
* \brief Here the ring is fitted with the COP algorithm from A. Ayriyan/G. Ososkov.
*
* \author Alexander Ayriyan, Gennadi Ososkov, Semen Lebedev <s.lebedev@gsi.de>
* \date 2005
**/
class CbmRichRingFitterCOP: public CbmRichRingFitterBase
{
public:
   /**
    * \brief Standard constructor.
    */
   CbmRichRingFitterCOP();

   /**
    * \brief Destructor.
    */
   ~CbmRichRingFitterCOP();

   /**
    * \brief Inherited from CbmRichRingFitterBase.
    */
   virtual void DoFit(
         CbmRichRingLight* ring);

private:
   /**
    * \brief Execute ring fitting algorithm.
    * \param[in,out] ring RICH ring to be fitted.
    */
   void FitRing(
         CbmRichRingLight* ring);
};

#endif
