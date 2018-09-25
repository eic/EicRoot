/**
* \file CbmRichRingTrackAssignBase.h
*
* \brief Base class for RICH rings - STS tracks matching algorithms.
*
* \author Semen Lebedev
* \date 2012
**/

#ifndef CBM_RICH_RING_TRACK_ASSIGN_BASE
#define CBM_RICH_RING_TRACK_ASSIGN_BASE

class TClonesArray;

/**
* \class CbmRichRingTrackAssignBase
*
* \brief Base class for RICH rings - STS tracks matching algorithms.
*
* \author Semen Lebedev
* \date 2012
**/
class CbmRichRingTrackAssignBase
{
public:
   /**
    * brief Standard constructor.
    */
   CbmRichRingTrackAssignBase():
      fMaxDistance(100.),
      fMinNofHitsInRing(5)
   {

   }

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichRingTrackAssignBase() { }

   /**
    * \brief Initialization in case one needs to initialize some TCloneArrays.
    */
   virtual void Init() { }

   /**
    * Perform RICH rings STS tracks matching procedure.
    * It updates index of the RICH ring in Global tracks.
    * \param[in] rings Array of RICH rings.
    * \param[in] richProj Array of track projections onto the photodetector plane.
    **/
   virtual void DoAssign(
         TClonesArray* rings,
         TClonesArray* richProj) = 0;

protected:

   double fMaxDistance;  // max. distance between ring center and track extrapolation
   int fMinNofHitsInRing; // min number of hits per ring

private:

   /**
    * \brief Copy constructor.
    */
   CbmRichRingTrackAssignBase(const CbmRichRingTrackAssignBase&);

   /**
    * \brief Assignment operator.
    */
   CbmRichRingTrackAssignBase& operator=(const CbmRichRingTrackAssignBase&);
};

#endif

