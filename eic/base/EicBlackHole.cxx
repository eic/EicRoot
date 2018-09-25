//
// AYK (ayk@bnl.gov), 2015/08/06;
//
//  Parent-hierarchy-related class; see the codes below for more details;
//

#include <TParticle.h>
#include <TVirtualMC.h>

#include <PndStack.h>

#include <EicBlackHole.h>

std::set<unsigned> *EicBlackHole::mTracks = new std::set<unsigned>();

// ---------------------------------------------------------------------------------------

std::pair<int, int> EicBlackHole::GetParentIDs()
{
  // First track in the predecessor chain which entered any of 
  // the "black hole" volumes; undefined per default;
  int lowestBlackVolumeTrackID = -1;

  // This call is cheap; do not mind to perform twice (see ProcessHits());
  int trackID = gMC->GetStack()->GetCurrentTrackNumber();

  //printf("  investigating track #%5d ...\n", trackID);
  // Assume can not get stuck in an infinite loop;
  for( ; ; )
  {
    TParticle *particle = ((PndStack*)gMC->GetStack())->GetParticle(trackID);

    if (EicBlackHole::IsInTrackList(trackID)) lowestBlackVolumeTrackID = trackID;

    //printf("     %5d (%d); %d\n", trackID, particle->IsPrimary(), EicBlackHole::IsInTrackList(trackID));

    // Reached a true primary particle in the parent list; return this ID unless
    // there was a track entering "black hole" volume in the chain;
    if (particle->IsPrimary()) {
      //printf(" ret: %5d %5d ... %5d\n", trackID, lowestBlackVolumeTrackID == -1 ? 
      //     trackID : lowestBlackVolumeTrackID, lowestBlackVolumeTrackID);
      return std::pair<int,int>(trackID, lowestBlackVolumeTrackID == -1 ? 
				trackID : lowestBlackVolumeTrackID);
    } //if

    trackID = particle->GetFirstMother();
    particle = ((PndStack*)gMC->GetStack())->GetParticle(trackID);
  } //for inf
} // EicBlackHole::GetParentIDs()

// ---------------------------------------------------------------------------------------
