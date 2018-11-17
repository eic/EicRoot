//
// AYK (ayk@bnl.gov), 2014/07/09
//
//  EIC code reconstructed event class; consider to reuse eic-smear codes; 
//
//  A good fraction of this code is cut'n'paste from EventSmear.h (eic-smear
//  package);

#include <eicsmear/erhic/EventMC.h>

#include <EicRcParticle.h>

#ifndef _EIC_RC_EVENT_
#define _EIC_RC_EVENT_

class EicRcEvent : public erhic::EventDis {
  friend class EicEventAssembler;

 public:

  /** Default constructor */
 EicRcEvent(): mScatteredLeptonID(-1)/*, femc(0.0), fhac(0.0)*/ {};
      
  /** Destructor */
  virtual ~EicRcEvent() { ClearParticles(); };
      
#if _LATER_
  /** Clear the particle list, sets event properties to default values. */
  virtual void Reset();
#endif
      
  /** Clears particle array, leaves event variables unchanged */
  virtual void ClearParticles();
      
  /** Returns the number of tracks in the event. */
  virtual UInt_t GetNTracks() const { return mParticles.size(); };

  /**
     Return the nth track.
     Return NULL if the track number is out of the range [0, GetNTracks()-1].
     @param [in] The track index, in the range [0, GetNTracks()-1].
  */
  virtual const EicRcParticle* GetTrack(UInt_t u) const {
    return (u < mParticles.size() ? mParticles.at(u) : NULL);
  };
  virtual       EicRcParticle* GetTrack(UInt_t u)       {
    return (u < mParticles.size() ? mParticles.at(u) : NULL);
  };

#if _LATER_    
  virtual void SetQ2(double Q2) { QSquared = Q2; }
  virtual void SetX(double xB) { x = xB; }
  virtual void SetY(double inelasticity) { y = inelasticity; }
  virtual void SetW2(double W2) { WSquared = W2; }
  virtual void SetNu(double Nu) { nu = Nu; }
#endif  
  
  // Basically wrappers for EicRootManager calls; yet it is convenient to have 
  // them here to easily access original generator event(track) classes (for 
  // instance to see which of the simulated particles have no reconstructed 
  // counterparts -> so were not detected/reconstructed);
  const erhic::EventMC*    GetGenMcEvent()                            const;
  // NB: numbering scheme here and in the below call follows ERHIC convention 
  // rather than PANDA one (so indices 0,1 correspond to beam particles); 
  const erhic::ParticleMC* GetGenMcTrack(int genMcIndex)              const;
  // Find this MC track in array of reconstructed particles; 
  EicRcParticle* GetRcParticleMatchingGenMcTrack(unsigned genMcIndex) const;

  // The only source of information for these two guys is original EventMC;
  // assume that beam lepton is #0 and beam hadron is #1 in respective 
  // EventMC internal array; 2016/11/15: use erhic::EventMC-inherited class methods instead;
  //const erhic::ParticleMC* BeamLepton()    const { return GetGenMcTrack(0); };
  const erhic::ParticleMC* BeamLepton()    const { return GetGenMcEvent()->BeamLepton(); };
  //const erhic::ParticleMC* BeamHadron()    const { return GetGenMcTrack(1); };
  const erhic::ParticleMC* BeamHadron()    const { return GetGenMcEvent()->BeamHadron(); };
  const erhic::ParticleMC* ExchangeBoson() const {return NULL; };
  
  virtual const EicRcParticle* ScatteredLepton() const {
    // Just return the stored pointer; its assignment is a different story;
    return mScatteredLeptonID == -1 ? 0 : mParticles[mScatteredLeptonID];
  };
  
  /**
     Yields all particles that belong to the hadronic final state.
     This is the same as the result of FinalState(), minus the scattered
     beam lepton.
  */
  void HadronicFinalState(ParticlePtrList&) const;


#if _LATER_
  /**
     Add a new track to the end of the track list.
     The track must be allocated via new and is subsequently owned
     by the Event.
  */
  virtual void AddLast(ParticleMCS*);
  
  
  /**
     Returns a vector of pointers to all tracks in the event.
     Note that this includes NULL pointers to tracks that were not detected.
     Do not delete the pointers.
  */
  std::vector<const erhic::VirtualParticle*> GetTracks() const;
  
  /**
     Set which particle is the scattered lepton.
  */
  virtual void SetScattered(int);
  
  /**
     Prints the attributes of this event to standard output.
     Prints event-wise kinematic values, and all tracks in the event.
  */
  virtual void Print(Option_t* = "") const;
#endif

  private:
  Int_t mScatteredLeptonID;               // scattered lepton ID in mParticles[]
  std::vector<EicRcParticle*> mParticles; ///< the reconstructed particle list
  
 public:
  // Clearly a hack for now;
  //double femc, fhac;

  ClassDef(EicRcEvent, 7);
};

#endif
