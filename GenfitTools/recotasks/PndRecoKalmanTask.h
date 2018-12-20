//-----------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      Kalman Filter Task
//
// Environment:
//      Software developed for the PANDA Detector at FAIR.
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//      Stefano Spataro, UNI Torino
//
//-----------------------------------------------------------

#ifndef PNDRECOKALMANTASK_HH
#define PNDRECOKALMANTASK_HH

// Base Class Headers ----------------
#include "FairTask.h"

// Collaborating Class Headers -------
#include "TString.h"
#include "PndRecoKalmanFit.h"
#include "PndRecoDafFit.h"

// Collaborating Class Declarations --
class TClonesArray;
class GFRecoHitFactory;
 

class PndRecoKalmanTask : public FairTask {
public:

  // Constructors/Destructors ---------
  PndRecoKalmanTask(const char* name = "Genfit", Int_t iVerbose = 0);
    ~PndRecoKalmanTask();

  // Operators
  

  // Accessors -----------------------
  
  // Modifiers -----------------------
  void SetTrackInBranchName(const TString& name)   { fTrackInBranchName = name;  }  
  void SetTrackInIDBranchName(const TString& name) { fTrackInIDBranchName = name;} 
  void SetTrackOutBranchName(const TString& name)  { fTrackOutBranchName = name; } 
  void SetMvdBranchName(const TString& name)       { fMvdBranchName = name;      }
  void SetCentralTrackerBranchName(const TString& name)  { fCentralTrackerBranchName = name; }
  void SetPersistence(Bool_t opt = kTRUE)          { fPersistence = opt;         }
  void SetGeane(Bool_t opt = kTRUE)                { fUseGeane = opt;            } 
  void SetIdealHyp(Bool_t opt = kTRUE)             { fIdealHyp = opt;            }
  void SetDaf(Bool_t opt = kTRUE)                  { fDaf = opt;                 }
  void SetPropagateToIP(Bool_t opt = kTRUE)        { fPropagateToIP = opt;       }
  void SetPerpPlane(Bool_t opt = kTRUE)            { fPerpPlane = opt;           }
  void SetTrackRep(Short_t num)                    { fTrackRep = num;            }
  void SetParticleHypo(TString s); 
  void SetParticleHypo(Int_t h);
  void SetBusyCut(Int_t b)                         { fBusyCut=b;                 }

  // Operations ----------------------
  virtual InitStatus Init();
  void SetParContainers();
  virtual void Exec(Option_t* opt);
  
  void StoreTrackParameterization( void )          { mStoreTrackParameterization = true; };

protected:
  
  // Private Data Members ------------
  TClonesArray* fTrackArray;      //! Input TCA for PndTrack
  TClonesArray* fTrackIDArray;    //! Input TCA for PndTrackID
  TClonesArray* fMCTrackArray;    //! Input TCA for PndMCTrack
  TClonesArray* fFitTrackArray;    //! Output TCA for track
  
  TString fTrackInBranchName;      //! Name of the input TCA 
  TString fTrackInIDBranchName;    //! Name of the input TCA
  TString fTrackOutBranchName;     //! Name of the output TCA
   
  TString fMvdBranchName;           //! Name of the TCA for MVD
  TString fCentralTrackerBranchName;//! Name of the TCA for central tracker

  PndRecoKalmanFit *fFitter; 
  PndRecoDafFit *fDafFitter;
  TDatabasePDG *pdg;             //! Particle DB

  Bool_t fPersistence;           //! Persistence

  Bool_t fUseGeane;              //! Flag to use Geane 
  Bool_t fSmoothing;             //! Flag to set on smoothing (not used) 
  Bool_t fIdealHyp;              //! Flag to use MC particle hypothesis
  Bool_t fDaf;                   //! Flag to use Deterministic Annealing
  Bool_t fPropagateToIP;         //! Flag to propagate the parameters to the interaction point (kTRUE)
  Bool_t fPerpPlane;             //! Flag to use as initial plane the one perpendicular to the track (kFALSE)
  Short_t fTrackRep;             //! (0) GeaneTrackRep, 1 RKTrackRep
  Int_t fNumIt;                  //! Number of iterations
  Int_t fPDGHyp;                 //! Hypothesis
  Int_t fBusyCut;                 //! Skip too busy events with more tracks

  // May want to instruct Kalman filter to record track parameterization (state
  // vector) at the hit locations;
  bool mStoreTrackParameterization;
  
  ClassDef(PndRecoKalmanTask,2);

};

#endif
