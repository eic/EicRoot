#ifndef PID_CORRELATOR_H
#define PID_CORRELATOR_H

// **************************************************************************
//  Author: Stefano Spataro e-mail: spataro@to.infn.it
//   
//  pid correlator
//
// Created: 03-07-09
// Modified:
//
// **************************************************************************

#include "TNtuple.h"
#include "TString.h"
#include "TClonesArray.h"
#include <map>

#include "FairTask.h"

#include "PndTrack.h"
#include "PndPidCandidate.h"
#include "PndGeoHandling.h"

#if _TODAY_
#include "FairField.h"
#include "FairTrackParH.h"
#include "FairGeanePro.h"
#include "FairGeane.h"

#include "PndRecoKalmanFit.h"
#include "PndPidCorrPar.h"
#include "PndEmcXClMoments.h"
#include "PndEmcGeoPar.h"
#include "PndEmcErrorMatrix.h"
#include "PndEmcClusterCalibrator.h"
#include "PndEmcErrorMatrixPar.h"
#include "PndGeoSttPar.h"
#include "PndGeoFtsPar.h"

using std::map;
using std::vector;

class TGeant3;
#endif

class PndPidCorrelator : public FairTask {

protected:
  TClonesArray* fTrack;             //! PndTrack TCA
  TClonesArray* fTrackID;           //! PndTrackID TCA
  TClonesArray* fPidChargedCand;    //! PndPidCandidate TCA for charged particles
  TClonesArray* fMcTrack;           //! PndMCTrack TCA
#if _TODAY_
  TClonesArray* fTrack2;            //! 2nd PndTrack TCA
  TClonesArray* fTrackID2;          //! 2nd PndTrackID TCA
  TClonesArray* fPidNeutralCand;    //! PndPidCandidate TCA for neutral particles
  TClonesArray* fMdtTrack;          //! PndTrack TCA for MDT refit

  TClonesArray* fMvdHitsStrip;      //! PndSdsHit TCA for strip
  TClonesArray* fMvdHitsPixel;      //! PndSdsHit TCA for pixel
  TClonesArray* fTofHit;            //! PndTofHit TCA 
  TClonesArray* fTofPoint;          //! PndTofPoint TCA 
  TClonesArray* fFtofHit;           //! PndFtofHit TCA
  TClonesArray* fFtofPoint;         //! PndFtofPoint TCA 
  TClonesArray* fEmcCluster;        //! PndEmcCluster TCA 
  TClonesArray* fEmcBump;           //! PndEmcBump TCA
  TClonesArray* fEmcDigi;           //! PndEmcDigi TCA
  TClonesArray* fMdtPoint;          //! PndMdtPoint TCA 
  TClonesArray* fMdtHit;            //! PndMdtHit TCA 
  TClonesArray* fMdtTrk;            //! PndMdtTrk TCA
  TClonesArray* fDrcPoint;          //! PndDrcBarPoint TCA
  TClonesArray* fDrcHit;            //! PndDrcHit TCA
  TClonesArray* fDskParticle;       //! PndDskParticle TCA  //need to change to PndDskHit in future
  TClonesArray* fSttHit;            //! PndSttHit TCA  
  TClonesArray* fFtsHit;            //! PndFtsHit TCA  
 
  PndPidCorrPar* fCorrPar;          //! Correlation parameters
  PndEmcGeoPar* fEmcGeoPar;         //! EMC geometry parameters
  PndEmcErrorMatrixPar* fEmcErrorMatrixPar; //! EMC error matrix parameters
  PndEmcErrorMatrix *fEmcErrorMatrix; //! EMC error matrix 
  PndGeoSttPar* fSttParameters;     //! STT geometry parameters 
  PndGeoFtsPar* fFtsParameters;     //! FTS geometry parameters
  PndEmcAbsClusterCalibrator* fEmcCalibrator;

  Short_t fDebugMode;               // Fill ntuples for debug
  Short_t fMvdMode;                 // MVD Mode: 0 no MVD, 1 MvdPoint, (2) MvdHit
  Short_t fSttMode;                 // STT Mode: 0 no STT, 1 SttPoint, (2) SttHit
  Short_t fFtsMode;                 // FTS Mode: 0 no FTS, 1 FtsPoint, (2) FtsHit
  Short_t fTofMode;                 // TOF Mode: 0 no TOF, 1 -empty-,  (2) TofHit
  Short_t fFtofMode;                // FTOF Mode:0 no FTOF,1 -empty-,  (2) FTofHit
  Short_t fEmcMode;                 // EMC Mode: 0 no EMC, 1 -empty-,  (2) EmcCluster, 3 EmcBumps
  Short_t fMdtMode;                 // MDT Mode: 0 no MDT, 1 -empty-,  (2) MdtHit 
  Short_t fDrcMode;                 // DRC Mode: 0 no DRC, 1 -empty-,  (2) DrcHit
  Short_t fDskMode;                 // DSK Mode: 0 no DSK, 1 -empty-,  (2) DskParticle
 
  Int_t fEmcClstCount;              // Number of EMC clusters
  Int_t fFscClstCount;		    // Number of FSC clusters

  Double_t fMvdELoss;               // Energy Loss in MVD 
  Double_t fMvdPath;                // MVD path crossed by the particle
  Int_t fMvdHitCount;               // Number of mvd hits
    
  map<Int_t, vector<Int_t> >mapMdtBarrel;  // map of MDT barrel hits
  map<Int_t, vector<Int_t> >mapMdtEndcap;  // map of MDT endcap+muon filter hits
  map<Int_t, vector<Int_t> >mapMdtForward; // map of MDT forward hits
  Float_t mdtLayerPos[3][20];              // position of MDT layers
  Float_t mdtIronThickness[3][20];         // thickness of iron layers

  map<Int_t, Bool_t> fClusterList;  // List of clusters correlated to tracks
  map<Int_t, Double_t> fClusterQ;   // List of emc quaity correlated to clusters
#endif
  TString fTrackBranch;             //  options to choose PndTrack branches
  TString fTrackIDBranch;           //  options to choose PndTrackID branches
  TString fTrackOutBranch;          //  options to choose output branch
#if _TODAY_
  TString fTrackBranch2;            //  options to choose 2nd PndTrack branches
  TString fTrackIDBranch2;          //  options to choose 2nd PndTrackID branches
  Bool_t fSimulation;               // Switch simulation diagnostic
  Bool_t fMdtRefit;                 // Use MDT Kalman refit propagation
  Bool_t fIdeal;                    // Ideal Correlation
  Bool_t fMixMode;                  // Mix mode flag
  Bool_t fFast;                     // 0: normal; 1: no correlation/extrapolation/neutral
  PndRecoKalmanFit *fFitter;         // Refitter for MDT tracks
  TFile *r;                          // File for debug ntuples
  TNtuple *tofCorr;                  // Debug ntuple for tof correlation
  TNtuple *ftofCorr;                 // Debug ntuple for ftof correlation
  TNtuple *emcCorr;                  // Debug ntuple for emc correlation 
  TNtuple *fscCorr;                  // Debug ntuple for fsc correlation
  TNtuple *mdtCorr;                  // Debug ntuple for mdt correlation 
  TNtuple *drcCorr;                  // Debug ntuple for drc correlation
  TNtuple *dskCorr;                  // Debug ntuple for dsk correlation
#endif  
  Int_t  fEventCounter;             // Event number
  Int_t fPidHyp;                    // particle hypothesis for propagation
  Bool_t fIdealHyp;                 // Flag to use MC particle hypothesis
  Bool_t fGeanePro;                 // Use GEANE propagation 
  Bool_t fCorrErrorProp;            // Error propagation in correlation
  PndGeoHandling* fGeoH;             // Object to retrieve MVD geometry

  TString sDir;                      // Ntuple output directory
  TString sFile;                     // Ntuple output file

  void ConstructChargedCandidate();
  PndPidCandidate* AddChargedCandidate(PndPidCandidate* cand); 
  Bool_t GetTrackInfo(PndTrack* track, PndPidCandidate* pid); 

#if _TODAY_
  void ConstructNeutralCandidate();

  PndPidCandidate* AddNeutralCandidate(PndPidCandidate* cand); 
  PndTrack* AddMdtTrack(PndTrack* track);
 
  Bool_t GetMvdInfo  (PndTrack* track, PndPidCandidate* pid); 
  Bool_t GetSttInfo  (PndTrack* track, PndPidCandidate* pid);  
  Bool_t GetFtsInfo  (PndTrack* track, PndPidCandidate* pid); 
  Bool_t GetGemInfo  (PndTrack* track, PndPidCandidate* pid);  
  Bool_t GetTofInfo  (FairTrackParH* helix, PndPidCandidate* pid);  
  Bool_t GetFtofInfo (FairTrackParH* helix, PndPidCandidate* pid); 
  Bool_t GetEmcInfo  (FairTrackParH* helix, PndPidCandidate* pid);
  Bool_t GetFscInfo (FairTrackParH* helix, PndPidCandidate* pid);
  Bool_t GetMdtInfo  (PndTrack* track, PndPidCandidate* pid);   
  Bool_t GetDrcInfo  (FairTrackParH* helix, PndPidCandidate* pid); 
  Bool_t GetDskInfo  (FairTrackParH* helix, PndPidCandidate* pid);
  Bool_t GetMdt2Info (FairTrackParH* helix, PndPidCandidate* pid); 
  Bool_t GetFMdtInfo (FairTrackParP* helix, PndPidCandidate* pid);

  Bool_t MdtMapping();  // Mapping of MDT hits
  Bool_t MdtGeometry(); // Mapping of MDT geometry
#endif
public:

  virtual void Exec(Option_t * option);
  virtual InitStatus Init();                        //
 
  void Register();
  void Reset();
#if _TODAY_ 
  void ResetEmcQ();

  PndPidCorrelator(const char *name, const char *title="Pnd Task");
#endif
  PndPidCorrelator();
  virtual ~PndPidCorrelator();

  void SetInputBranch(TString branch)     { fTrackBranch = branch; };
  void SetInputIDBranch(TString branch)   { fTrackIDBranch = branch; };	 

#if _TODAY_
  void SetOption(Option_t *option=" ")    {fOption = option;  fOption.ToLower();}
  void SetDebugMode(Bool_t debug)         { fDebugMode = debug; };
  void SetDebugFilename(TString filename) { sFile = filename; };
  void SetMdtRefit(Bool_t mdt)            { fMdtRefit = mdt; };
  void SetMixMode(Bool_t mix)             { fMixMode = mix; };
  void SetInputBranch2(TString branch)    { fTrackBranch2 = branch; };
  void SetInputIDBranch2(TString branch)  { fTrackIDBranch2 = branch; };
  void SetOutputBranch(TString branch)    { fTrackOutBranch = branch; };
  void SetSimulation(Bool_t sim)          { fSimulation = sim; };
  void SetIdeal(Bool_t id)                { fIdeal = id; }; 
  void SetFast(Bool_t fast)               { fFast = fast; };
  void SetCorrErrProp(Bool_t err)         { fCorrErrorProp = err; };
  void SetGeanePro(Bool_t gea = kTRUE)    { fGeanePro = gea; };
  void SetPidHyp(Int_t pid)               { fPidHyp = pid; };
  void SetIdealHyp(Bool_t opt = kTRUE)    { fIdealHyp = opt;            }

  void SetMvdMode(Short_t mode)	{ fMvdMode = mode; };                 // MVD Mode: 0 no MVD
  void SetSttMode(Short_t mode)	{ fSttMode = mode; };                 // STT Mode: 0 no STT 
  void SetFtsMode(Short_t mode)	{ fFtsMode = mode; };                 // FTS Mode: 0 no FTS
  void SetTofMode(Short_t mode)	{ fTofMode = mode; };                 // TOF Mode: 0 no TOF
  void SetFtofMode(Short_t mode){ fFtofMode = mode; };                // FTOF Mode:0 no FTOF
  void SetEmcMode(Short_t mode)	{ fEmcMode = mode; };                 // EMC Mode: 0 no EMC
  void SetMdtMode(Short_t mode)	{ fMdtMode = mode; };                 // MDT Mode: 0 no MDT 
  void SetDrcMode(Short_t mode)	{ fDrcMode = mode; };                 // DRC Mode: 0 no DRC
  void SetDskMode(Short_t mode)	{ fDskMode = mode; };                 // DSK Mode: 0 no DSK
#endif

  /** Get parameter containers **/
  virtual void SetParContainers();
  virtual void Finish();

ClassDef(PndPidCorrelator,2)   // PndPidCorrelator

};

#endif
