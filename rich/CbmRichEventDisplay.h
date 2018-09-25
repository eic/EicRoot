/**
* \file CbmRichEventDisplay.h
*
* \brief Event display for the RICH detector.
*
* \author Semen Lebedev
* \date 2012
**/

#ifndef CBM_RICH_EVENT_DISPLAY
#define CBM_RICH_EVENT_DISPLAY

#include "FairTask.h"

#include <map>
#include <string>

class TClonesArray;
class CbmRichRing;

/**
* \class CbmRichEventDisplay
*
* \brief Event display for the RICH detector.
*
* \author Semen Lebedev
* \date 2012
**/
class CbmRichEventDisplay : public FairTask
{
public:

   /**
    * \brief Default constructor.
    */
   CbmRichEventDisplay();

   /**
    * \brief Destructor.
    */
   virtual ~CbmRichEventDisplay();

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

   void SetDrawRings(bool b){fDrawRings = b;}
   void SetDrawHits(bool b){fDrawHits = b;}
   void SetDrawPoints(bool b){fDrawPoints = b;}
   void SetDrawProjections(bool b){fDrawProjections = b;}

private:

   TClonesArray* fRichRings;
   TClonesArray* fRichHits;
   TClonesArray* fRichPoints;
   TClonesArray* fRichMatches;
   TClonesArray* fRichProjections;

   TClonesArray* fMcTracks;

   int fEventNum;

   bool fDrawRings;
   bool fDrawHits;
   bool fDrawPoints;
   bool fDrawProjections;

   void DrawOneEvent();

   void DrawOnePmtPlane(
         const std::string& plane);

   void DrawCircle(
         CbmRichRing* ring);


   /**
    * \brief Copy constructor.
    */
   CbmRichEventDisplay(const CbmRichEventDisplay&);

   /**
    * \brief Assignment operator.
    */
   CbmRichEventDisplay& operator=(const CbmRichEventDisplay&);

   ClassDef(CbmRichEventDisplay,1);
};

#endif
