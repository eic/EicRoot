// *************************************************************************
// Author: Ralf Kliemt ralf.kliemt(at)hiskp(dot)uni-bonn(dot)de
//   
// ideal tracker using MC id & track info
// gaussian momentum smearing & fake efficiency included
// z-momentum & z-coordinate of vertex is smeared with doubled sigma (just a guess)
//
// Created: 28.01.2011
//
// Modified for EicRoot: AYK, 2013
//
// *************************************************************************

#ifndef _EIC_IDEAL_TRACKING_CODE_
#define _EIC_IDEAL_TRACKING_CODE_

#include "TVector3.h"
#include "TDatabasePDG.h"
#include "FairTask.h"
#include <cmath>

class EicGeoParData;
class EicTrackingDigiHitProducer;
class SensitiveVolume;
class LogicalVolumeLookupTableEntry;

#include <EicDetName.h>
#include <EicGeoParData.h>
#include <EicTrackingDigiHit.h>

class EicDetectorGroup {
  // FIXME: do it better later;
  friend class EicIdealTrackingCode;
  friend class EicHtcTask;
  friend class EicRecoKalmanTask;
  friend class FwdTrackFinder;
  friend class FwdHoughTree;

 public: 
  //@@@ EicDetectorGroup();
 EicDetectorGroup(char *name = 0): svCounter(0), _fMCPoints(0), _fHits(0),
    mGptr(0), mDigi(0) { dname = name ? new EicDetName(name) : 0; };
  ~EicDetectorGroup() {}; 

  EicDetName *dname;

  SensitiveVolume* GetSensitiveVolume(const EicTrackingDigiHit *hit) const {
    if (!hit) return 0;
    
    // FIXME: may want to simplify (and optimize) this stuff at some point;
    ULogicalIndex_t id = mGptr->GeantMultiToLogicalIndex(hit->GetMultiIndex());
    LogicalVolumeLookupTableEntry *lNode = mGptr->GetLookupTableNode(id);

    if (mSensitiveVolumes.find(lNode) == mSensitiveVolumes.end()) return 0;

    return mSensitiveVolumes.at(lNode);
  };

  EicGeoParData *GetGptr() const { return mGptr; };

 private:
  TClonesArray*  _fMCPoints;     //! Array of event's points
  TClonesArray*  _fHits;         //! Array of event's hits
  Int_t          _fBranchID;     //! Branch ID

  unsigned svCounter;
  EicGeoParData *mGptr;
  EicTrackingDigiHitProducer *mDigi;

  std::map<LogicalVolumeLookupTableEntry*, SensitiveVolume*> mSensitiveVolumes;
};

class EicIdealTrackingCode : public FairTask {
  // FIXME: do it better later;
  friend class EicRecoKalmanTask;
  friend class EicHtcTask;
  friend class FwdTrackFinder;
  
public:
  
  EicIdealTrackingCode();
  virtual ~EicIdealTrackingCode();  
  
  int AddDetectorGroup(char *name);

  virtual void Exec(Option_t * option);
  virtual InitStatus Init();              
  virtual void Finish(); 
  
  void Reset();                    
  virtual void Register();                 
  
  void SetTrackOutBranchName(TString name) { fTracksArrayName = name; };
  
  void SetMomentumSmearing(Double_t sigmax, Double_t sigmay, Double_t sigmaz) 
    { fMomSigma.SetXYZ(fabs(sigmax),fabs(sigmay),fabs(sigmaz)); fRelative=kFALSE; }; // in GeV
  void SetRelativeMomentumSmearing(Double_t dpop) { fDPoP=fabs(dpop); fRelative=kTRUE;}; // in GeV
  void SetVertexSmearing(Double_t sigmax, Double_t sigmay, Double_t sigmaz) 
    { fVtxSigma.SetXYZ(fabs(sigmax),fabs(sigmay),fabs(sigmaz)); }; // in cm
  void SetTrackingEfficiency(Double_t eff) { fEfficiency=eff; };
  
protected:
  
  void SmearFWD(TVector3 &vec, const TVector3 &sigma); // smearing with doubled sigma in z direction
  
  TClonesArray*  fMCTracks;         //! Array of PndMCTrack

  std::vector<EicDetectorGroup> fGroups;
  
  TClonesArray  *fTrackCands;   //! Array of found track candidates
  TClonesArray  *fTracks;       //! Array of found tracks
  TClonesArray  *fTrackIds;     //! Array of track IDs (Links)

  // Parameters for fake tracking;
  TVector3 fMomSigma;          // Momentum smearing sigma [GeV]
  Double_t fDPoP;              // Relative momentum Smearing
  Bool_t fRelative;            // falg
  TVector3 fVtxSigma;          // Vertex smearing sigma [cm]
  Double_t fEfficiency;        // Tracking efficiency - if (0 <= e < 1), some tracks will be discarded
  
  TString fTracksArrayName;     // Branch name where to store the Track candidates
  TDatabasePDG *pdg;            //! Particle DB
  
  EicIdealTrackingCode(const  EicIdealTrackingCode& L);
  EicIdealTrackingCode& operator= (const  EicIdealTrackingCode&) {return *this;}
  
  ClassDef(EicIdealTrackingCode,1);
};

#endif
