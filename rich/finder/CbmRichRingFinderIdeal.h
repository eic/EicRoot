/**
* \file CbmRichRingFinderIdeal.h
*
* \brief Ideal ring finder in the RICH detector. It uses MC information
* to attach RICH hits to rings.
*
* \author Supriya Das
* \date 2006
**/

#ifndef CBM_RICH_RING_FINDER_IDEAL
#define CBM_RICH_RING_FINDER_IDEAL

#include "CbmRichRingFinder.h"

class TClonesArray;

class CbmRichRingFinderIdeal : public CbmRichRingFinder
{
private:
   TClonesArray* fRichPoints; // CbmRichPoint array
   TClonesArray* fMcTracks; // CbmMCTrackArray

public:
   /**
    * \brief Default constructor.
    */
   CbmRichRingFinderIdeal();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingFinderIdeal();

   /**
    * \brief Inherited from CbmRichRingFinder.
    */
   virtual void Init();

  /**
   * Inherited from CbmRichRingFinder.
   */
   virtual int DoFind(
       TClonesArray* hitArray,
       TClonesArray* projArray,
		 TClonesArray* ringArray);

private:
   /**
    * \brief Copy constructor.
    */
   CbmRichRingFinderIdeal(const CbmRichRingFinderIdeal&);

   /**
    * \brief Assignment operator.
    */
   CbmRichRingFinderIdeal& operator=(const CbmRichRingFinderIdeal&);

};

#endif
