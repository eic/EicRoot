//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Task to select charged particles from the primary vertex (pion hypotheses)
//
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
//
//-----------------------------------------------------------

#ifndef PRIMSELECTOR_HH
#define PRIMSELECTOR_HH

// Base Class Headers ----------------
#include "FairTask.h"

// Collaborating Class Headers -------
 

// Collaborating Class Declarations --
class TClonesArray;


class PrimSelector : public FairTask {
public:

  // Constructors/Destructors ---------
  PrimSelector();
  virtual ~PrimSelector();

  
  // Accessors -----------------------


  // Modifiers -----------------------
  void SetTrackBranchName(const TString& name) {_trackBranchName=name;}
  void SetPersistence(Bool_t flag=kTRUE) {_persistence=flag;}
  void SetNExpectedTracks(unsigned int n){fNExpectedTracks=n;}

  // Operations ----------------------
  virtual InitStatus Init();
  
  virtual void Exec(Option_t* opt);

  virtual void FinishTask();

private:

  // Private Data Members ------------
  TString _trackBranchName;
  Bool_t _persistence;
  unsigned int fEventCounter;
  unsigned int fNExpectedTracks;
  unsigned int fNFullEvents;
  unsigned int fNPhysTracks;
  unsigned int fNBkgTracks;
  

  TClonesArray* _trackArray;
  TClonesArray* _pocaArray;
TClonesArray* _chargeArray;
TClonesArray* _primArray;
  // Private Methods -----------------

public:
  ClassDef(PrimSelector,1)
};

#endif

//--------------------------------------------------------------
// $Log$
//--------------------------------------------------------------
