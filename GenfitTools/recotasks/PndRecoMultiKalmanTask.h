//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Kalman Filter Task for multiple particle hypothesis
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//      Stefano Spatarot, UNI Torino
//
//-----------------------------------------------------------

#ifndef PNDRECOMULTIKALMANTASK_HH
#define PNDRECOMULTIKALMANTASK_HH

// Base Class Headers ----------------
#include "FairTask.h"

// Collaborating Class Headers -------
#include "TString.h"
#include "PndRecoKalmanFit.h"
#if _EIC_OFF_
#include "PndGeoSttPar.h"
#endif
// Collaborating Class Declarations --
class TClonesArray;
class GFRecoHitFactory;
 

class PndRecoMultiKalmanTask : public FairTask {
public:

  // Constructors/Destructors ---------
  PndRecoMultiKalmanTask(const char* name = "Genfit", Int_t iVerbose = 0);
    ~PndRecoMultiKalmanTask();

  // Operators
  

  // Accessors -----------------------
  
  // Modifiers -----------------------
  void SetTrackInBranchName(const TString& name)   { fTrackInBranchName = name; } 
  void SetTrackOutBranchName(const TString& name)  { fTrackOutBranchName = name; }
  void SetMvdBranchName(const TString& name)       { fMvdBranchName = name; }
  void SetCentralTrackerBranchName(const TString& name)  { fCentralTrackerBranchName = name; }
  void SetPersistence(Bool_t opt = kTRUE)        { fPersistence = opt;      }
  void SetGeane(Bool_t opt = kTRUE)              { fUseGeane = opt;         }
  void SetNumIterations(Int_t num)               { fNumIt = num;        }
 
  // Operations ----------------------
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  
  void SetParContainers();
  
private:

  // Private Data Members ------------
  TClonesArray* fTrackArray; 
  TClonesArray* fFitTrackArrayElectron; //! Output TCA for track
  TClonesArray* fFitTrackArrayMuon;     //! Output TCA for track
  TClonesArray* fFitTrackArrayPion;     //! Output TCA for track
  TClonesArray* fFitTrackArrayKaon;     //! Output TCA for track
  TClonesArray* fFitTrackArrayProton;   //! Output TCA for track
  
  TString fTrackInBranchName;      //! Name of the input TCA
  TString fTrackOutBranchName;     //! Name of the output TCA
  
  TString fMvdBranchName;           //! Name of the TCA for MVD
  TString fCentralTrackerBranchName;//! Name of the TCA for central tracker
  
  PndRecoKalmanFit *fFitter;
  
  Bool_t fPersistence;

  Bool_t fUseGeane;              //! Flag to use Geane 
  Bool_t fSmoothing;             //! Flag to set on smoothing
  Int_t fNumIt;                  //! Number of iterations
#if _EIC_OFF_
  PndGeoSttPar *fSttParameters;  //! STT params
#endif
  ClassDef(PndRecoMultiKalmanTask,1);

};

#endif
