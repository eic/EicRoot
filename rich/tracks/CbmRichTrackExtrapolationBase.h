/**
* \file CbmRichTrackExtrapolationBase.h
*
* \brief This is interface for concrete extrapolation algorithms to RICH.
*
* \author Semen Lebedev
* \date 2012
**/

#ifndef CBM_RICH_TRACK_EXTRAPOLATION_BASE
#define CBM_RICH_TRACK_EXTRAPOLATION_BASE

class TClonesArray;

class CbmRichTrackExtrapolationBase
{
public:
   /**
    * \brief Default constructor.
    */
   CbmRichTrackExtrapolationBase()
   {
   }

   /**
    * \brief Distructor.
    */
   virtual ~CbmRichTrackExtrapolationBase()
   {
   }

   /**
    * \brief Initialization in case one needs to initialize some TClonearrays.
    */
   virtual void Init()
   {
   }

   /**
    * \brief Read the global track array, extrapolate track to a given z-Plane
    *  in RICH detector and fill output array with FairTrackParam.
    * \param[in] globalTracks Global tracks.
    * \param[out] extrapolatedTrackParams Output array of track parameters.
    * \param[in] z Z coordinate to which track will be extrapolated.
    * \param[in] minNofStsHits number of STS hits required for extrapolated track
    */
   virtual void DoExtrapolation(
         TClonesArray* globalTracks,
         TClonesArray* extrapolatedTrackParams,
         double z,
         int minNofStsHits) = 0;

private:
   /**
    * \brief Copy constructor.
    */
   CbmRichTrackExtrapolationBase(const CbmRichTrackExtrapolationBase&);

   /**
    * \brief Assignment operator.
    */
   CbmRichTrackExtrapolationBase& operator=(const CbmRichTrackExtrapolationBase&);
};

#endif
