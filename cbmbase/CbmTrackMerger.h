// -------------------------------------------------------------------------
// -----                     CbmTrackMerger header file                -----
// -----                  Created 01/12/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmTrackMerger
 *@author v.friese@gsi.de
 **
 ** Abstract base class for merging of local tracks in STS and TRD to
 ** a global track. Derived classes must implement the method DoMerge.
 **/


#ifndef CBMTRACKMERGER_H
#define CBMTRACKMERGER_H 1


#include "TObject.h"


class TClonesArray;


class CbmTrackMerger : public TObject 
{

 public:

  /** Default constructor **/
 CbmTrackMerger() : TObject(), fVerbose(0) { };


  /** Destructor **/
  virtual ~CbmTrackMerger() { };


  /** Virtual method Init. If needed, to be implemented in the
   ** concrete class. Else no action.
   **/
  virtual void Init() { };


  /** Abstract method DoMerge. To be implemented in the derived classes.
   ** Task: Take arrays of StsTracks and TrdTracks and merge them to 
   ** GlobalTracks. Fill the TClonesArray with CbmGlobalTracks.
   **
   *@param stsTracks   Array of CbmStsTrack (input)
   *@param trdTracks   Array of CbmTrdTrack (input)
   *@param glbTracks   Array of CbmGlobalTracks (output)
   **
   *@value Number of merged STS+TRD tracks
   **/
  virtual Int_t DoMerge(TClonesArray* stsTracks,
			TClonesArray* trdTracks,
			TClonesArray* glbTracks) = 0;


  /** Set verbosity 
   *@param verbose   Verbosity level
   **/
  void SetVerbose(Int_t verbose) { fVerbose = verbose; };



 protected:

  Int_t fVerbose;         // Verbosity level


  ClassDef(CbmTrackMerger,1);

};

#endif

