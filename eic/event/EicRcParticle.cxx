//
// AYK (ayk@bnl.gov), 2014/07/08
//
//  EIC reconstructed particle class; interface to eic-smear, 
//  PANDA classes (like MCTrack and PidChargedCand) is conveniently 
//  hidden from end user;
//

#include <TClonesArray.h>

#include <EicEventAssembler.h>
#include <EicRootManager.h>
#include <EicRcParticle.h>

// ---------------------------------------------------------------------------------------

//
//  Follow the logic of ParticleMCS::GetEta() & ParticleMCS::GetRapidity(); in particular
//  use 19. as a default value in both cases;
//
 
#define _DEFAULT_RAPIDITY_ (19.)

Double_t EicRcParticle::GetEta() const 
{
  // Well, this can not fail for a valid 3D TVector3, or?;
  double theta = GetTheta();

  return (theta > 0.0 && theta < TMath::Pi() ? -log(tan(theta/2.)) : _DEFAULT_RAPIDITY_);
} // EicRcParticle::GetEta()

Double_t EicRcParticle::GetRapidity() const 
{
  // Yes, prefer to hide behind these methods, whatever GetE() is;
  double E = GetE(), Pz = GetPz();

  // In the present scheme "E>Pz" is always true; well, keep the check anyway;
  return (E > Pz ? 0.5 * log((E + Pz) / (E - Pz)) : _DEFAULT_RAPIDITY_);
} // EicRcParticle::GetRapidity()

// ---------------------------------------------------------------------------------------

const erhic::ParticleMC* EicRcParticle::GetGeneratorTrack() const
{
  EicRootManager *io = EicRootManager::Instance();

  return io ? io->GetGenMcTrack(mGeneratorEventIndex) : 0;
} // EicRcParticle::GetGeneratorTrack()

// ---------------------------------------------------------------------------------------

const PndMCTrack* EicRcParticle::GetPndMcTrack() const
{
  EicRootManager *io = EicRootManager::Instance();
  if ( !io || !io->GetPndMcTracks() || mPndMCTrackIndex == -1) return 0;

  io->SynchronizeBranch(io->GetPndMcBranch());

  return mPndMCTrackIndex < io->GetPndMcTracks()->GetEntriesFast() ? 
    (PndMCTrack*)io->GetPndMcTracks()->At(mPndMCTrackIndex) : 0;
} // EicRcParticle::GetPndMCTrack()

// ---------------------------------------------------------------------------------------

const PndPidCandidate* EicRcParticle::GetPndPidCandidate() const
{
  EicRootManager *io = EicRootManager::Instance();
  if ( !io || !io->GetPndPidCandidates() || mPndPidChargedCandIndex == -1) return 0;

  io->SynchronizeBranch(io->GetPndPidBranch());

  return mPndPidChargedCandIndex < io->GetPndPidCandidates()->GetEntriesFast() ? 
    (PndPidCandidate*)io->GetPndPidCandidates()->At(mPndPidChargedCandIndex) : 0;
} // EicRcParticle::GetPndPidCandidate()

// ---------------------------------------------------------------------------------------

ClassImp(EicRcParticle)
ClassImp(EicRcCalorimeterHit)

