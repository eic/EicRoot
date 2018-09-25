// -------------------------------------------------------------------------
// -----                     CbmRichMerger header file                 -----
// -----                  Created 01/12/05  by V. Friese               -----
// -------------------------------------------------------------------------


/** CbmRichMerger
 *@author v.friese@gsi.de
 **
 ** Abstract base class for merging of RICH rings to a global track.
 ** Derived classes must implement the method DoMerge.
 **/


#ifndef CBMRICHMERGER_H
#define CBMRICHMERGER_H 1


#include "TObject.h"


class TClonesArray;


class CbmRichMerger : public TObject
{

 public:

  /** Default constructor **/
 CbmRichMerger() : TObject(), fVerbose(0) { };


  /** Destructor **/
  virtual ~CbmRichMerger() { };


  /** Virtual method Init. If needed, to be implemented in the
   ** concrete class. Else no action.
   **/
  virtual void Init() { };


  /** Abstract method DoMerge. To be implemented in the derived classes.
   ** Task: Take arrays of GlobalTracks and RichRings and attach rings
   ** to the GlobalTrack by setting the reference index.
   **
   *@param glbTracks  Array of CbmGlobalTracks
   *@param richRings  Array of CbmRichRings
   **
   *@value Number of rings attached to the global tracks
   **/
  virtual Int_t DoMerge(TClonesArray* glbTracks,
			TClonesArray* richRings) = 0;


  /** Set verbosity 
   *@param verbose   Verbosity level
   **/
  void SetVerbose(Int_t verbose) { fVerbose = verbose; };



 private:

  Int_t fVerbose;         // Verbosity level


  ClassDef(CbmRichMerger,1);

};

#endif
