/**
* \file CbmRichTrackExtrapolationIdeal.h
*
* \brief This is the implementation of the TrackExtrapolation from MC points.
* It reads the STS track array, gets the corresponding MC RefPlanePoint
* and selects those to be projected to the Rich Photodetector.
*
* \author Claudia Hoehne
* \date 2006
**/

#ifndef CBM_RICH_TARCK_EXTRAPOLATION_IDEAL
#define CBM_RICH_TRACK_EXTRAPOLATION_IDEAL

#include "CbmRichTrackExtrapolationBase.h"

//class TClonesArray;

/**
* \class CbmRichTrackExtrapolationIdeal
*
* \brief "TrackExtrapolation" from MC points. It reads the PointArray with ImPlanePoints
* from MC and selects those to be projected to the Rich Photodetector.
*
* \author Claudia Hoehne
* \date 2006
**/
class CbmRichTrackExtrapolationIdeal : public CbmRichTrackExtrapolationBase
{
public:

   /**
    * \brief Default constructor.
    */
   CbmRichTrackExtrapolationIdeal();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichTrackExtrapolationIdeal();

   /**
    * \brief Inherited from CbmRichTrackExtrapolationBase.
    */
   virtual void Init();


   /**
    * \brief Inherited from CbmRichTrackExtrapolationBase.
    */
   virtual void DoExtrapolation(
         TClonesArray* globalTracks,
         TClonesArray* extrapolatedTrackParams,
         double z,
         int minNofStsHits);

private:
   TClonesArray* fRefPlanePoints;
   TClonesArray* fMcTracks;
   //TClonesArray* fStsTracks;
   //TClonesArray* fStsTrackMatches;

   /**
    * \brief Copy constructor.
    */
   CbmRichTrackExtrapolationIdeal(const CbmRichTrackExtrapolationIdeal&);

   /**
    * \brief Assignment operator.
    */
   CbmRichTrackExtrapolationIdeal& operator=(const CbmRichTrackExtrapolationIdeal&);
};

#endif
