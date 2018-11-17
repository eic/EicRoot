//
// AYK (ayk@bnl.gov), 2014/07/09
//
//  EIC code reconstructed event class; consider to reuse eic-smear codes; 
//

#include <EicRootManager.h>
#include <EicRcEvent.h>

// ---------------------------------------------------------------------------------------

const erhic::EventMC* EicRcEvent::GetGenMcEvent() const
{
  EicRootManager *io = EicRootManager::Instance();

  return io ? io->GetGenMcEvent() : 0;
} // EicRcEvent::GetGenMcEvent()

// ---------------------------------------------------------------------------------------

const erhic::ParticleMC* EicRcEvent::GetGenMcTrack(int genMcIndex) const
{
  EicRootManager *io = EicRootManager::Instance();

  return io ? io->GetGenMcTrack(genMcIndex) : 0;
} // EicRcEvent::GetGenMcTrack()

// ---------------------------------------------------------------------------------------

void EicRcEvent::ClearParticles()
{
  for(unsigned i(0); i < mParticles.size(); ++i) {
    EicRcParticle *particle = GetTrack(i);
    
    if(particle) delete particle;
  } //for i
} // EicRcEvent::ClearParticles()

// ---------------------------------------------------------------------------------------

EicRcParticle* EicRcEvent::GetRcParticleMatchingGenMcTrack(unsigned genMcIndex) const
{
  // Sanity check: skip beam particles;
  if (genMcIndex <= 1) return 0;

  // FIXME: this call is not too much efficient, sorry; need to create a separate 
  // mapping set perhaps;
  for(unsigned iq=0; iq<mParticles.size(); iq++) {
    EicRcParticle *particle = mParticles.at(iq);

    // Has never been checked;
    if (particle->GetGeneratorIndex()  == genMcIndex || 
	particle->GetPndMCTrackIndex() == genMcIndex-2) 
      return particle;
  } //for iq

  return 0;
} // EicRcEvent::GetRcParticleMatchingGenMcTrack()

// ---------------------------------------------------------------------------------------

void EicRcEvent::HadronicFinalState(ParticlePtrList& final) const 
{
#if _OLD_
  // Skip the first two entries, as these are the incident beams;
  for(unsigned i(2); i < GetNTracks(); ++i) {
#endif
  for(unsigned i(0); i < GetNTracks(); ++i) {
    const EicRcParticle *particle = GetTrack(i);

    if (particle && i != mScatteredLeptonID) final.push_back(particle);
  } //for i
} // EicRcEvent::HadronicFinalState()

// ---------------------------------------------------------------------------------------

ClassImp(EicRcEvent)
