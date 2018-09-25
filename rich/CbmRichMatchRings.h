/**
* \file CbmRichMatchRings.h
*
* \brief Task class for matching a reconstructed CbmRichRings with a simulated
*  CbmMCTrack. The matching criterion is a maximal number of common
*  hits/points. The task fills the data class CbmRichRingMatch for
*  each CbmRichRing.
*
* \author Supriya Das
* \date 2006
**/

#ifndef CBM_RICH_MATCH_RINGS
#define CBM_RICH_MATCH_RINGS

#include "FairTask.h"

#include <map>

class TClonesArray;


/**
* \class CbmRichMatchRings
*
* \brief Task class for matching a reconstructed CbmRichRings with a simulated
*  CbmMCTrack. The matching criterion is a maximal number of common
*  hits/points. The task fills the data class CbmRichRingMatch for
*  each CbmRichRing.
*
* \author Supriya Das
* \date 2006
**/
class CbmRichMatchRings : public FairTask
{

public:

   /**
    * \brief Default constructor.
    */
   CbmRichMatchRings();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichMatchRings();

   /**
    * \brief Inherited from FairTask.
    */
   virtual InitStatus Init();

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Exec(
         Option_t* opt);

   /**
    * \brief Inherited from FairTask.
    */
   virtual void Finish();


private:

   TClonesArray* fRings; // Array of CbmRichRings
   TClonesArray* fPoints; // Array of FairMCPoints
   TClonesArray* fTracks; // Array of CbmMCTracks
   TClonesArray* fHits; // Array of CbmRichHits
   TClonesArray* fMatches; // Array of CbmRichRingMatch

   std::map<Int_t, Int_t> fMatchMap; // Map from MCTrackID to number of common hits
//   std::map<Int_t, Int_t> fMatchMCMap; // Map from MCTrackID to number of common hits for MC rings

   /**
    * \brief Copy constructor.
    */
   CbmRichMatchRings(const CbmRichMatchRings&);

   /**
    * \brief Assignment operator.
    */
   CbmRichMatchRings& operator=(const CbmRichMatchRings&);

   ClassDef(CbmRichMatchRings,1);
};

#endif
