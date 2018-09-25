//
// AYK (ayk@bnl.gov), 2014/07/08
//
//  EIC reconstructed particle class; interface to eic-smear, 
//  PANDA classes (like MCTrack and PidChargedCand) is conveniently 
//  hidden from end user;
//

#include <TVector3.h>
#include <TParticlePDG.h>

#include <PndMCTrack.h>

#include <eicsmear/erhic/ParticleMC.h>

#include <TClonesArray.h>
class PndPidCandidate;
//class EicRcVertex;

#ifndef _EIC_RC_PARTICLE_
#define _EIC_RC_PARTICLE_

class EicRcCalorimeterHit: public TObject {
  friend class EicEventAssembler;
  friend class EicRcParticle;

 public: 
 EicRcCalorimeterHit(): mEnergy(0.0), mEnergySigma(0.0) {};
  ~EicRcCalorimeterHit() {};

 private:
  Double_t mEnergy;                    // reconstructed hit energy ... 
  Double_t mEnergySigma;               // ... with estimated uncertainty
  TVector3 mLocation;                  // 3D location of this hit
  // NB: cluster arrays are detector-specific (say, mBemcClusters in EventAssembler);
  //std::vector<unsigned> mCellGroupIDs; // cell group IDs of clusters which were used to estimate energy & location

  ClassDef(EicRcCalorimeterHit,2)
};

class EicRcParticle: public erhic::VirtualParticle {
  // No reason to have excessive Set...() methods;
  friend class EicEventAssembler;

 public:
 EicRcParticle(): mRcPdgCode(0), mChargeDefCL(0.0), mElectronPidCL(0.0), 
    mPndPidChargedCandIndex(-1), mPndMCTrackIndex(-1), mGeneratorEventIndex(-1),
    mRcTrackerMomentumSigma(0.0)
    { mEmCal = mHCal = 0; };
  ~EicRcParticle() {};

  // ------------- erhic::VirtualParticle (pure) virtual method implementation

  // Well, the original eic-smear writers were aware about TVector3 class 
  // existence, right?; so what was the point of introducing this stuff?;
  // tried to save some CPU time perhaps ...;
  virtual Double_t GetPx()            const { return mRcVtxMomentum.Px(); };
  virtual Double_t GetPy()            const { return mRcVtxMomentum.Py(); };
  virtual Double_t GetPz()            const { return mRcVtxMomentum.Pz(); };
  virtual Double_t GetP()             const { return mRcVtxMomentum.Mag(); };
  virtual Double_t GetPt()            const { return mRcVtxMomentum.Pt(); };
  virtual Double_t GetTheta()         const { return mRcVtxMomentum.Theta(); };
  virtual Double_t GetPhi()           const { return mRcVtxMomentum.Phi(); };
  // "True" rapidity;
  virtual Double_t GetRapidity()      const;
  // Pseudorapidity;
  virtual Double_t GetEta()           const;
  // Interaction vertex for primary particles; decay vertex for secondary ones;
  virtual TVector3 GetVertex()        const { return mRcVertex; };

  // FIXME: check with eic-smear codes later which implementation is expected;
  void SetVertex(const TVector3&) { exit(0); };

  erhic::Pid Id()                     const { 
    //printf("%d\n", mRcPdgCode); fflush(stdout);
    return ::erhic::Pid(mRcPdgCode); 
  }

  // Particle mass; remember, fRcPdg PDG code is ALWAYS available, no matter what; 
  virtual Double_t GetM()             const { return Id().Info()->Mass(); }; 

  // Yes, as trivial as that; THINK: this may all break once I start working with 
  // resonances; well, even there may decouple invariant mass value from the "true" 
  // mass of identified particle;
  virtual Double_t GetE()             const { return sqrt(pow(GetP(), 2) + pow(GetM(), 2)); };

  // (E,p) 4-vector in the lab frame; FIXME: not the most efficient call - 
  // see GetE()->GetM(); may want to optimize later;
  virtual TLorentzVector Get4Vector() const { 
    return TLorentzVector(GetPx(), GetPy(), GetPz(), GetE());
  };

  // FIXME: this call needs to be implemented properly;
  virtual UShort_t GetStatus()        const { exit(0); };
  // Dummy one; just need to compile;
  void Set4Vector(const TLorentzVector&)    {};
  // Same as in ParticleMCS, whatever it is good for ...;
  virtual UShort_t GetParentIndex()   const { return 0; }

  // ------------- various interface calls

  // Get indices in erhic::VirtualParticle, PidChargedCand and PndMCTrack arrays;
  int GetGeneratorIndex()             const { return mGeneratorEventIndex; }
  int GetPndPidChargedCandIndex()     const { return mPndPidChargedCandIndex; }
  int GetPndMCTrackIndex()            const { return mPndMCTrackIndex; }

  // Well, may want to access other variables in the related PndMCTrack
  // and PndPidCandidate arrays; EicRcParticle class itself has only a limited 
  // set of variables copied over (possibly symmetric in MC and RC); 
  const PndMCTrack* GetPndMcTrack()   const;
  // .. and respective shortcut;
  const PndMCTrack*        pndmc()    const { return GetPndMcTrack(); };
  // In particular PndPidCandidate has a plenty of various crap in;
  const PndPidCandidate* GetPndPidCandidate() const;
  // .. and respective shortcut;
  const PndPidCandidate*   pndrc()    const { return GetPndPidCandidate(); };
  // In this case just return the stored pointer;
  const erhic::ParticleMC* GetGeneratorTrack() const;
  // .. and respective shortcut;
  const erhic::ParticleMC* genmc()    const { return GetGeneratorTrack(); };

  // For completeness ...;
  const EicRcParticle*     eicrc()    const { return this; };

  //
  //  Well, I sort of want to remain sitting on two chairs: 
  //    1) want to have symmetric access to similar variables in EventMC and EicRcEvent 
  //       (so fill out pointer vectors there in the same order (with 0 pointer "gaps" 
  //       in reconstructed vector, to be specific); this is governed in part by 
  //       erhic::VirtualParticle-related methods above; 
  //    2) also want to retain symmetric access to basic simulated and reconstructed 
  //       variables (like momentum and vertex coordinates) even if erhic::VirtualParticle 
  //       entries are missing (eg Box generator used for detector studies); so maintain 
  //       a second set of methods right here which allow convenient access;
  //

  // Get MC and reconstructed 3D vertex; if performance in access to MC variables this 
  // way ever becomes a problem, use local variables and fill them out once right at a 
  // time of EicRcParticle creation; FIXME: in this case may want to use TVector3&, etc 
  // as return values;
  const TVector3 GetMcVertex()        const { 
    // FIXME: May want to cache at least this pointer;
    const PndMCTrack *mctrack = GetPndMcTrack();

    return mctrack ? mctrack->GetStartVertex() : TVector3();
  };
  const TVector3 GetRcVertex()        const { return mRcVertex; }


  // Get MC and reconstructed 3D momentum at vertex;
  const TVector3 GetMcVtxMomentum()   const { 
    const PndMCTrack *mctrack = GetPndMcTrack();

    return mctrack ? mctrack->GetMomentum() : TVector3();
  };
  const TVector3 GetRcVtxMomentum()   const { return mRcVtxMomentum; }
  double GetRcTrackerMomentumSigma()  const { return mRcTrackerMomentumSigma; }
  // A hack for test purposes;
  void SetRcVtxMomentum(TVector3 &momentum) { mRcVtxMomentum = momentum; };

  // Get MC and PID-reconstructed particle type;
  int GetMcPdgCode()                  const { 
    const PndMCTrack *mctrack = GetPndMcTrack();

    // Yes, return 0 if unknown (hmm, say reconstructed ghost track, or Rec->MC 
    // match failed or PndMCTrack entries are not available, etc);
    return mctrack ? mctrack->GetPdgCode() : 0;
  };
  int GetRcPdgCode()                  const { return mRcPdgCode; }

  double GetEmCalEnergy()             const { return mEmCal ? mEmCal->mEnergy : 0.0; };
  double GetHCalEnergy()              const { return mHCal  ? mHCal->mEnergy  : 0.0; };

 private:
  // NB: leave them signed (-1 is a "non-assigned" indicator);
  int mGeneratorEventIndex;    // index in erhic::VirtualEvent array (in case of generator input)
  int mPndMCTrackIndex;        // index in PandaRoot PndMCTrack array
  int mPndPidChargedCandIndex; // index in PandaRoot PidChargedCand array

  // FIXME: link to the EicRcVertex class should be implemented later;
  //EicRcVertex *RcVertex()     { return 0; }
  //int fVertexId;           // index in EicRcVertex clone array

 public:
  // I guess to the moment do not need more options?; extend later if needed;
  enum HadronPidType {PionKaonProton, PionKaon, KaonProton, PionProton};

 private:
  //
  // PID procedure - PDG definition logic - (to be) implemented:
  //
  //   - PDG code is ALWAYS assigned to some valid non-zero number; the meaning and confidence
  //     level are moderated by accompanying variables;
  //   
  //   - assume, that if track is available, charge can ALWAYS be determined (at least 
  //     with a 50% probability if a straight track in no field :-);
  //
  //   - tracking (TR) used for momentum and charge sign determination;
  //   - {TR,ECAL} and/or {ECAL,HCAL} used for electron/hadron separation (EPID);
  //   - RICH used for pion/kaon/proton separation (HPID); THINK: well, in fact can be used 
  //     for electron ID as well?;
  //
  //     OPTIONS:
  //     ========
  //
  //   A) charged track is registered -> fChargeDefCL in the range [0.5 .. 1.0]:
  //   -------------------------------------------------------------------------
  //     - A1: EPID & HPID Ok -> PID-based value: +/-(11/211/321/2212); 
  //                                                             fElectronPidCL assigned; fHadronPidCL assigned;
  //     - A2: no HPID info   -> if electron    : +/-11;         fElectronPidCL assigned; fHadronPidCL empty;
  //                          -> if hadron      : +/-211 (pion); fElectronPidCL assigned; fHadronPidCL empty;
  //     - A3: no EPID info   -> if pion        : +/-211;        fElectronPidCL=0;        fHadronPidCL assigned;
  //                          -> if kaon/proton : +/-(321/2212); fElectronPidCL assigned based on either PionKaon 
  //                                                             or PionProton;           fHadronPidCL assigned;
  //     - A3: no EPID & HPID ->                : +/-211 (pion); fElectronPidCL=0;        fHadronPidCL empty;
  //
  //   B) no charged track in tracker acceptance -> fChargeDefCL = 1.0 (or whatever tracking inefficiency):
  //   ----------------------------------------------------------------------------------------------------
  //     - B1: EPID available ->                : EPID-based, either  22 (gamma) or 111 (pi^0); yes, assume pi^0;
  //                                                             fElectronPidCL assigned; fHadronPidCL empty;
  //     - B1: no EPID        ->                : 111 (pi^0); yes, assume pi^0;
  //                                                             fElectronPidCL=0;        fHadronPidCL empty;
  //
  //   C) track would be out of tracker acceptance (say, high |eta|) -> fChargeDefCL = 0.0:
  //   ------------------------------------------------------------------------------------
  //     - C1: EPID available ->                : EPID-based, either  22 (gamma) or 111 (pi^0); 
  //                                                             fElectronPidCL assigned; fHadronPidCL empty;
  //     - C1: no EPID        ->                : 111 (pi^0); yes, assume pi^0;
  //                                                             fElectronPidCL=0;        fHadronPidCL empty;
  //

  int mRcPdgCode;           // PID-based (or wildly guessed) PDG code (see above)
  // Well, something like expected fraction of 1/p gaussian tail extending to opposite charge;
  float mChargeDefCL;       // charge definition confidence level in the range [0.5 .. 1.0]
  float mElectronPidCL;     // electron/hadron separation confidence level in PDG assignment
  // Looks too much tricky, but leave as it is for now; typically will be just 
  // one pair here I guess; leave some freedom for more complicated cases though;
  // access through GetHadronPidCL() shoud hide the details; 
  std::vector< std::pair<HadronPidType, float> > mHadronPidCL; // available hadron separation CLs

  //
  // These variables I want to have separately instead of extracting them on-the-fly
  // because the assignment differs between charged particles, gammas, decay particles, etc;
  // 
  
  // Taken either from tracking (if track is available) or remains (0,0,0) if only calorimeter 
  // cluster information is available; 
  TVector3 mRcVertex;       // reconstructed track 3D vertex

  //
  //  Well, consider a bit naive logic here:
  //
  //   - assume, that this variable contains both 1D energy and 3D momentum 
  //     information properly weighted to pack best knowledge particle kinematics; 
  //   - assume PID info is ALWAYS available (see fRcPdg above and comments there), 
  //     so use that PDG mass in the calculation;
  //   - GetE() in this approach just trivially returns sqrt(p^2+m^2), NOT 
  //     the calorimeter energy; 
  //
  // FIXME: later there should be easy way to access original 3D (tracker) momentum
  //        and original calorimeter energy variables;
  //
  // NB: since Kinematics.cxx routines use momentum if it is available, 
  //     this approach should work there (or be a good start, at least); 
  //
  // NB: Kalman filter is assumed to be re-run with anticipated PID hypothesis
  //     (well, FIXME: so far pion hypothesis is always used), so everything should 
  //     be sort of consistent, right?;
  TVector3 mRcVtxMomentum;          // reconstructed track 3D momentum at the vertex
  Double_t mRcTrackerMomentumSigma; // diagonal error on momentum magnitude

#if _OLD_
  // Well, the idea is that no matter how track-to-cluster and cluster-to-cluster 
  // (in different calorimeters) association is made, at the end EicRcParticle class
  // instance contains links to calorimeter cluster groups in some of the calorimeters, 
  // which was sufficient to calculate energy deposit fraction and 3D location of hit 
  // related to this particle; the higher level code may the proceed to 1) combine EmCal
  // information into the final estimate of e/m energy (indeed eg FEMC and CEMC may 
  // have energy deposits from the same track), 2) calculate weighted mean momentum&energy
  // 3D kinematics at the IP, 3) perform necessary PID assignments based on e/p and hadronic
  // calorimeter signal; FIXME: perhaps vectors and enum stuff would be better?;
#endif
  std::vector<std::pair<unsigned, unsigned> > mCellGroups; // cell group IDs of clusters which were used to estimate energy & location
  EicRcCalorimeterHit *mEmCal, *mHCal;

  ClassDef(EicRcParticle, 16);
};

#endif
