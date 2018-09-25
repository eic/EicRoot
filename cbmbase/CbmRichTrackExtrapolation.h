/******************************************************************************
*  $Id: CbmRichTrackExtrapolation.h,v 1.4 2006/01/30 10:59:59 hoehne Exp $
*
*  Class  : CbmRichTrackExtrapolation
*  Description : Abstract base class for concrete track extrapolation algorithm
*                of tracks to some z-Plane in RICH detector
*                Each derived class must implement the method DoExtrapolate.
*
*  Author : Claudia Hoehne
*  E-mail : c.hoehne@gsi.de
*
*******************************************************************************
*  $Log: CbmRichTrackExtrapolation.h,v $
*  Revision 1.4  2006/01/30 10:59:59  hoehne
*  RichPoint Array was not filled correctly: put TClonesArray in method
*
*  Revision 1.3  2006/01/26 09:53:21  hoehne
*  initial version for track extrapolation (base class + concrete implementations + task) to z-plane in RICH
*
*
*
*******************************************************************************/

#ifndef CBM_RICH_TRACK_EXTRAPOLATION
#define CBM_RICH_TRACK_EXTRAPOLATION 1

#include "TObject.h"

class TClonesArray;
class CbmGlobalTrack;
class FairTrackParam;


class CbmRichTrackExtrapolation : public TObject
{

 public:

  /** Default constructor **/
 CbmRichTrackExtrapolation() : TObject(), fVerbose(0) { };


  /** Destructor **/
  virtual ~CbmRichTrackExtrapolation() { };


  /** Virtual method Init. If needed, to be implemented in the
   ** concrete class. Else no action.
   **/
  virtual void Init() { };

  /** Virtual method Init. If needed, to be implemented in the
   ** concrete class. Else no action.
   **/
  virtual void Finish() { };


  /** Abstract method DoExtrapolateTrack. To be implemented in the concrete class.
   ** Task: Read the Track array and fill the TrackParam array at given z-Plane in RICH detector
   ** pointers to which are given as arguments
   **
   *@param rTrack  pointer to global track
   *@param fZ      z-position for extrapolation [cm]
   **/
   virtual Int_t DoExtrapolate(TClonesArray* gTrackArray,
		               Double_t fZ, TClonesArray *fTrackParamArray) = 0;


  /** Set verbosity 
   *@param verbose   Verbosity level
   **/
  void SetVerbose(Int_t verbose) { fVerbose = verbose; };


 private:

  Int_t fVerbose;      // Verbosity level


  ClassDef(CbmRichTrackExtrapolation,1);

};

#endif
