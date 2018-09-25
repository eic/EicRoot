//-----------------------------------------------------------
// Task which coverts TClonesArray of GFTrack to TClonesArray of PndTrack 
//-----------------------------------------------------------

#ifndef PNDGFTRACKTOPNDTRACKCONVERTORTASK_HH
#define PNDGFTRACKTOPNDTRACKCONVERTORTASK_HH

// Base Class Headers ----------------
#include "FairTask.h"

// Collaborating Class Headers -------
#include "TString.h"

// Collaborating Class Declarations --
class TClonesArray;
 
class PndGFTrackToPndTrackConvertorTask : public FairTask {
public:

  // Constructors/Destructors ---------
  PndGFTrackToPndTrackConvertorTask(const char* name = "Track Convertor", Int_t iVerbose = 0);
    ~PndGFTrackToPndTrackConvertorTask();

  // Operators
  

  // Accessors -----------------------
  
  // Modifiers -----------------------
  void SetTrackInBranchName(const TString& name)   { fTrackInBranchName = name;  } 
  void SetTrackOutBranchName(const TString& name)  { fTrackOutBranchName = name; } 

  // Operations ----------------------
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  
private:
  
  // Private Data Members ------------
  TClonesArray* fInTrackArray; 
  TClonesArray* fOutTrackArray;    //! Output TCA for track
  
  TString fTrackInBranchName;      //! Name of the input TCA
  TString fTrackOutBranchName;     //! Name of the output TCA
   
  ClassDef(PndGFTrackToPndTrackConvertorTask,1);

};

#endif
