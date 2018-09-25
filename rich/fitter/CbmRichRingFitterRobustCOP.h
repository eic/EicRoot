/**
* \file CbmRichRingFitterRobustCOP.h
*
* \brief Here the ring is fitted with the RobustCOP algorithm from A. Ayriyan/G. Ososkov.
*
* \author Alexander Ayriyan, Gennadi Ososkov, Claudia Hoehne, Semen Lebedev <s.lebedev@gsi.de>
* \date 2012
**/
#ifndef CBM_RICH_RING_FITTER_ROBUST_COP
#define CBM_RICH_RING_FITTER_ROBUST_COP

#include "CbmRichRingFitterBase.h"

/**
* \class CbmRichRingFitterRobustCOP
*
* \brief Here the ring is fitted with the RobustCOP algorithm from A. Ayriyan/G. Ososkov.
*
* \author Alexander Ayriyan, Gennadi Ososkov, Claudia Hoehne, Semen Lebedev <s.lebedev@gsi.de>
* \date 2012
**/
class CbmRichRingFitterRobustCOP : public CbmRichRingFitterBase
{
public:

   /**
    * \brief Default constructor.
    */
   CbmRichRingFitterRobustCOP();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingFitterRobustCOP();

   /**
    * \brief Inherited from CbmRichRingFitterBase.
    */
   virtual void DoFit(
         CbmRichRingLight* ring);

};

#endif
