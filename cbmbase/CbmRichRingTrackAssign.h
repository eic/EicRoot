/******************************************************************************
*  $Id: CbmRichRingTrackAssign.h,v 1.1 2006/01/26 09:54:27 hoehne Exp $
*
*  Class  : CbmRichRingTrackAssign
*  Description : Abstract base class for concrete RICH ring - track assignement algorithm.
*                Each derived class must implement the method DoAssign.
*
*  Author : Claudia Hoehne
*  E-mail : c.hoehne@gsi.de
*
*******************************************************************************
*  $Log: CbmRichRingTrackAssign.h,v $
*  Revision 1.1  2006/01/26 09:54:27  hoehne
*  initial version: assignement of Rich rings and extrapolated tracks (base class, concrete implementation, Task)
*
*
*
*******************************************************************************/

#ifndef CBM_RICH_RING_TRACK_ASSIGN
#define CBM_RICH_RING_TRACK_ASSIGN 1

#include "TObject.h"

class TClonesArray;
class CbmRichRing;


class CbmRichRingTrackAssign : public TObject
{

 public:

  /** Default constructor **/
 CbmRichRingTrackAssign() : TObject(), fVerbose(0) { };


  /** Destructor **/
  virtual ~CbmRichRingTrackAssign() { };


  /** Virtual method Init. If needed, to be implemented in the
   ** concrete class. Else no action.
   **/
  virtual void Init() { };


  /** Abstract method DoAssign. To be implemented in the concrete class.
   ** Task: read ring array of found rings and of extrapolated tracks
   **       andd assign ring and track
   **
   *@param pTrack    pointer to extrapolated track
   *@param pRing     pointer to found ring
   **/
  virtual void DoAssign(TClonesArray* pRing,
		        TClonesArray* pTrack) = 0;


  /** Set verbosity 
   *@param verbose   Verbosity level
   **/
  void SetVerbose(Int_t verbose) { fVerbose = verbose; };


 private:

  Int_t fVerbose;      // Verbosity level


  ClassDef(CbmRichRingTrackAssign,1);

};

#endif
