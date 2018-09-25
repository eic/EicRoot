// -------------------------------------------------------------------------
// -----                       PndStack source file                    -----
// -----             Created 10/08/04  by D. Bertini / V. Friese       -----
// -------------------------------------------------------------------------
#include "PndStack.h"

#include "FairDetector.h"
#include "FairMCPoint.h"
#include "PndMCTrack.h"
#include "FairRootManager.h"
#include "FairMCEventHeader.h"

#include "TError.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TRefArray.h"

#include <list>
#include <iostream>

using std::cout;
using std::endl;
using std::pair;


// -----   Default constructor   -------------------------------------------
PndStack::PndStack(Int_t size):
fStack(), fParticles(), fTracks(), fStoreMap(), fStoreIter(), fIndexMap(), fIndexIter(), fPointsMap(), fCurrentTrack(-1), fNPrimaries(0), fNParticles(0), fNTracks(0), fIndex(0), fStoreSecondaries(kTRUE), fMinPoints(1), fEnergyCut(0), fStoreMothers(kTRUE)
{
  fStoreMap.clear();
  fIndexMap.clear();
  fPointsMap.clear();
  fParticles = new TClonesArray("TParticle", size);
  fTracks    = new TClonesArray("PndMCTrack", size);
}

// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
PndStack::~PndStack() {
  if (fParticles) {
    fParticles->Delete();
    delete fParticles;
  }
  if (fTracks) {
    fTracks->Delete();
    delete fTracks;
  }
}
// -------------------------------------------------------------------------

  

// -----   Virtual public method PushTrack   -------------------------------
void PndStack::PushTrack(Int_t toBeDone, Int_t parentId, Int_t pdgCode,
			 Double_t px, Double_t py, Double_t pz,
			 Double_t e, Double_t vx, Double_t vy, Double_t vz, 
			 Double_t time, Double_t polx, Double_t poly,
			 Double_t polz, TMCProcess proc, Int_t& ntr, 
			 Double_t weight, Int_t is) {

	PushTrack( toBeDone, parentId, pdgCode,
				  px,  py,  pz,
				  e,  vx,  vy,  vz,
				  time,  polx,  poly,
				  polz, proc, ntr,
				  weight, is, -1);
}

// -----   Virtual public method PushTrack   -------------------------------
void PndStack::PushTrack(Int_t toBeDone, Int_t parentId, Int_t pdgCode,
			 Double_t px, Double_t py, Double_t pz,
			 Double_t e, Double_t vx, Double_t vy, Double_t vz, 
			 Double_t time, Double_t polx, Double_t poly,
			 Double_t polz, TMCProcess proc, Int_t& ntr, 
			 Double_t weight, Int_t is,Int_t secondparentID) {

  // --> Get TParticle array
  TClonesArray& partArray = *fParticles;

  // --> Create new TParticle and add it to the TParticle array
  Int_t trackId = fNParticles;
  Int_t nPoints = 0;
  Int_t daughter1Id = -1;
  Int_t daughter2Id = -1;
  TParticle* particle = 
    new(partArray[fNParticles++]) TParticle(pdgCode, trackId, parentId, 
					    nPoints, daughter1Id, 
					    daughter2Id, px, py, pz, e, 
					    vx, vy, vz, time);
  particle->SetLastMother(secondparentID);
  particle->SetPolarisation(polx, poly, polz);
  particle->SetWeight(weight);
  particle->SetUniqueID(proc);

  // --> Increment counter
  if (parentId < 0) fNPrimaries++;

  // --> Set argument variable
  ntr = trackId;

  // --> Push particle on the stack if toBeDone is set
  if (toBeDone == 1) fStack.push(particle);

}
// -------------------------------------------------------------------------

  

// -----   Virtual method PopNextTrack   -----------------------------------
TParticle* PndStack::PopNextTrack(Int_t& iTrack) {

  // If end of stack: Return empty pointer
  if (fStack.empty()) {
    iTrack = -1;
    return NULL;
  }

  // If not, get next particle from stack
  TParticle* thisParticle = fStack.top();
  fStack.pop();

  if ( !thisParticle) {
    iTrack = 0;
    return NULL;
  }

  fCurrentTrack = thisParticle->GetStatusCode();
  iTrack = fCurrentTrack;

  return thisParticle;

}
// -------------------------------------------------------------------------

  

// -----   Virtual method PopPrimaryForTracking   --------------------------
TParticle* PndStack::PopPrimaryForTracking(Int_t iPrim) {

  // Get the iPrimth particle from the fStack TClonesArray. This
  // should be a primary (if the index is correct).

  // Test for index
  if (iPrim < 0 || iPrim >= fNPrimaries) {
    gLogger->Error(MESSAGE_ORIGIN, "PndStack: Primary index out of range! $i" , iPrim );
    Fatal("PndStack::PopPrimaryForTracking", "Index out of range");
  }

  // Return the iPrim-th TParticle from the fParticle array. This should be
  // a primary.
  TParticle* part = (TParticle*)fParticles->At(iPrim);
  if ( ! (part->GetMother(0) < 0) ) {
    gLogger->Error(MESSAGE_ORIGIN, "PndStack:: Not a primary track! , $i " ,iPrim);
    Fatal("PndStack::PopPrimaryForTracking", "Not a primary track");
  }

  return part;

}
// -------------------------------------------------------------------------



// -----   Virtual public method GetCurrentTrack   -------------------------
TParticle* PndStack::GetCurrentTrack() const {
  TParticle* currentPart = GetParticle(fCurrentTrack);
  if ( ! currentPart) {
    gLogger->Warning(MESSAGE_ORIGIN, "PndStack: Current track not found in stack!");
    Warning("PndStack::GetCurrentTrack", "Track not found in stack");
  }
  return currentPart;
}
// -------------------------------------------------------------------------


  
// -----   Public method AddParticle   -------------------------------------
void PndStack::AddParticle(TParticle* oldPart) {
  TClonesArray& array = *fParticles;
  TParticle* newPart = new(array[fIndex]) TParticle(*oldPart);
  newPart->SetWeight(oldPart->GetWeight());
  newPart->SetUniqueID(oldPart->GetUniqueID());
  fIndex++;
}
// -------------------------------------------------------------------------



// -----   Public method FillTrackArray   ----------------------------------
void PndStack::FillTrackArray() {

  gLogger->Debug(MESSAGE_ORIGIN,"PndStack: Filling MCTrack array...");

  // --> Reset index map and number of output tracks
  fIndexMap.clear();
  fNTracks = 0;

  // --> Check tracks for selection criteria
  SelectTracks();

  // --> Loop over fParticles array and copy selected tracks
  for (Int_t iPart=0; iPart<fNParticles; iPart++) {

    fStoreIter = fStoreMap.find(iPart);
    if (fStoreIter == fStoreMap.end() ) {
      gLogger->Error(MESSAGE_ORIGIN,"PndStack: Particle  %i  not found in storage map!" ,iPart);
      Fatal("PndStack::FillTrackArray","Particle not found in storage map.");
    }
    Bool_t store = (*fStoreIter).second;

    if (store) {
      PndMCTrack* track = 
	new( (*fTracks)[fNTracks]) PndMCTrack(GetParticle(iPart));
      fIndexMap[iPart] = fNTracks;
      // --> Set the number of points in the detectors for this track
      for (Int_t iDet=kRICH; iDet<=kHYP; iDet++) {
	pair<Int_t, Int_t> a(iPart, iDet);
	track->SetNPoints(iDet, fPointsMap[a]);
      }

	  SetGeneratorFlags(iPart);

      fNTracks++;
    
    }else{
      fIndexMap[iPart] = -2;
    }

  }

  // --> Map index for primary mothers
  fIndexMap[-1] = -1;

  // --> Screen output
  Print(0);

}
// -------------------------------------------------------------------------

void PndStack::SetGeneratorFlags(Int_t myid)
{
	if(myid<0) return;

	PndMCTrack* mytrack;
	{
		Int_t myid2=fIndexMap[myid];
		if(myid2<0){
			gLogger->Error(MESSAGE_ORIGIN,"=== This should not happen negative index in MAP!!");
			return;
		}

		mytrack = (PndMCTrack*)fTracks->At(myid2);
	}

	Int_t n;
	Int_t daughters=0, daughtersp=0;
	// fParticles; // TParticle
	n=fParticles->GetEntries();
	for (Int_t i=0; i<n; i++) {
		TParticle* part = (TParticle*)fParticles->At(i);
		Int_t m;
		m=part->GetMother(0);
		if(myid==m){
			daughters++;
		}else if(m==-1){
			m=part->GetMother(1);
			if(myid==m){
				daughtersp++;
			}
		}else if(m==-2){
			// removed should not happen before this is called
			// and anyway not on the TParticle Level
			gLogger->Error(MESSAGE_ORIGIN,"=== Problem!!! part mother -2" );
		}
	}

	Int_t mymo1=mytrack->GetMotherID();

	if( ((TParticle*)fParticles->At(myid))->GetMother(0)!=mymo1){
		gLogger->Error(MESSAGE_ORIGIN,"=== Problem: Mothers != %i ", myid);
	}
	if(mymo1==-1){
		if(daughters!=0 && daughtersp!=0){
			gLogger->Error(MESSAGE_ORIGIN,"=== Problem: particle with index %i has  daughters= %i  && daughtersp= %i ",myid,daughters,daughtersp);
		}

		mytrack->SetGeneratorCreated();
		if(daughtersp>0) mytrack->SetGeneratorDecayed();
//		cout << myid <<" ("<<mytrack->GetPdgCode()<<"): "<<daughters<<","<<daughtersp<<" ==> " <<mytrack->IsGeneratorCreated()<<" "<<mytrack->IsGeneratorDecayed()<<" "<<mytrack->IsGeneratorLast()<<endl;
	}
}


// -----   Public method UpdateTrackIndex   --------------------------------
void PndStack::UpdateTrackIndex(TRefArray* detList) {

  gLogger->Debug(MESSAGE_ORIGIN, "PndStack: Updating track indizes...");
  Int_t nColl = 0;

  FairMCEventHeader* header = (FairMCEventHeader*)FairRootManager::Instance()->GetObject("MCEventHeader.");

  // First update mother ID in MCTracks
  for (Int_t i=0; i<fNTracks; i++) {
    PndMCTrack* track = (PndMCTrack*)fTracks->At(i);
    Int_t iMotherOld = track->GetMotherID();
    fIndexIter = fIndexMap.find(iMotherOld);
    if (fIndexIter == fIndexMap.end()) {
      gLogger->Error(MESSAGE_ORIGIN,"PndStack: Particle index  %i  not found in dex map! ", iMotherOld);
      Fatal("PndStack::UpdateTrackIndex","Particle index not found in map");
    }
    track->SetMotherID( (*fIndexIter).second );
    if(iMotherOld==-1){
        iMotherOld = track->GetSecondMotherID();
        fIndexIter = fIndexMap.find(iMotherOld);
        if (fIndexIter == fIndexMap.end()) {
           gLogger->Error(MESSAGE_ORIGIN,"PndStack: Particle index  %i  not found in dex map! (second mother id)", iMotherOld);
           Fatal("PndStack::UpdateTrackIndex","Particle index not found in map");
        }
        track->SetSecondMotherID( (*fIndexIter).second );
    }
  }

  // Now iterate through all active detectors
  TIterator* detIter = detList->MakeIterator();
  detIter->Reset();
  FairDetector* det = NULL;
	while ((det = (FairDetector*) detIter->Next())) {

		// --> Get hit collections from detector
		Int_t iColl = 0;
		TClonesArray* hitArray;
		while ((hitArray = det->GetCollection(iColl++))) {
			nColl++;
			Int_t nPoints = hitArray->GetEntriesFast();

			// --> Update track index for all MCPoints in the collection
			for (Int_t iPoint = 0; iPoint < nPoints; iPoint++) {
				FairMCPoint* point = (FairMCPoint*) hitArray->At(iPoint);
				Int_t iTrack = point->GetTrackID();

				fIndexIter = fIndexMap.find(iTrack);
				if (fIndexIter == fIndexMap.end()) {
					gLogger->Error(MESSAGE_ORIGIN,
							"PndStack: Particle index %i not found in index map! ",
							iTrack);
					Fatal("PndStack::UpdateTrackIndex",
							"Particle index not found in map");
				}
				point->SetTrackID((*fIndexIter).second);
//				std::cout << "Header->GetEventID() " << header->GetEventID() << std::endl;
				point->SetLink(FairLink(-1, (header->GetEventID()-1), "MCTrack", (*fIndexIter).second));
			}

		}   // Collections of this detector
	}     // List of active detectors

  gLogger->Debug(MESSAGE_ORIGIN,"...stack and %i collections updated.", nColl);
  delete detIter;
}
// -------------------------------------------------------------------------



// -----   Public method Reset   -------------------------------------------
void PndStack::Reset() {
  fIndex = 0;
  fCurrentTrack = -1;
  fNPrimaries = fNParticles = fNTracks = 0;
  while (! fStack.empty() ) fStack.pop();
  fParticles->Clear();
  fTracks->Clear();
  fPointsMap.clear();
}
// -------------------------------------------------------------------------



// -----   Public method Register   ----------------------------------------
void PndStack::Register() {
  FairRootManager::Instance()->Register("MCTrack", "Stack", fTracks,kTRUE);
}
// -------------------------------------------------------------------------



// -----   Public method Print  --------------------------------------------
void PndStack::Print(Int_t iVerbose) const {
  gLogger->Debug(MESSAGE_ORIGIN,"  PndStack: Number of primaries  = ",fNPrimaries);
  gLogger->Debug(MESSAGE_ORIGIN,"  Total number of particles  = ", fNParticles);
  gLogger->Debug(MESSAGE_ORIGIN,"  Number of tracks in output = ", fNTracks);
 
  if (iVerbose) {
    for (Int_t iTrack=0; iTrack<fNTracks; iTrack++) 
      ((PndMCTrack*) fTracks->At(iTrack))->Print(iTrack);
  }
}
// -------------------------------------------------------------------------



// -----   Public method AddPoint (for current track)   --------------------
void PndStack::AddPoint(DetectorId detId) {
  Int_t iDet = detId;
  pair<Int_t, Int_t> a(fCurrentTrack, iDet);
  if ( fPointsMap.find(a) == fPointsMap.end() ) fPointsMap[a] = 1;
  else fPointsMap[a]++;
}
// -------------------------------------------------------------------------



// -----   Public method AddPoint (for arbitrary track)  -------------------
void PndStack::AddPoint(DetectorId detId, Int_t iTrack) {
  if ( iTrack < 0 ) return;
  Int_t iDet = detId;
  pair<Int_t, Int_t> a(iTrack, iDet);
  if ( fPointsMap.find(a) == fPointsMap.end() ) fPointsMap[a] = 1;
  else fPointsMap[a]++;
}
// -------------------------------------------------------------------------




// -----   Virtual method GetCurrentParentTrackNumber   --------------------
Int_t PndStack::GetCurrentParentTrackNumber() const {
  TParticle* currentPart = GetCurrentTrack();
  if ( currentPart ) return currentPart->GetFirstMother();
  else               return -1;
}
// -------------------------------------------------------------------------



// -----   Public method GetParticle   -------------------------------------
TParticle* PndStack::GetParticle(Int_t trackID) const {
  if (trackID < 0 || trackID >= fNParticles) {
    gLogger->Error(MESSAGE_ORIGIN,"PndStack: Particle index out of range.", trackID);
    Fatal("PndStack::GetParticle", "Index out of range");
  }
  return (TParticle*)fParticles->At(trackID);
}
// -------------------------------------------------------------------------



// -----   Private method SelectTracks   -----------------------------------
void PndStack::SelectTracks() {

  // --> Clear storage map
  fStoreMap.clear();

  // --> Check particles in the fParticle array
  for (Int_t i=0; i<fNParticles; i++) {

    TParticle* thisPart = GetParticle(i);
    Bool_t store = kTRUE;

    // --> Get track parameters
    Int_t iMother   = thisPart->GetMother(0);
    TLorentzVector p;
    thisPart->Momentum(p);
    Double_t energy = p.E();
    Double_t mass   = p.M();
//    Double_t mass   = thisPart->GetMass();// Why?? Mass (given by generator) is inside by Lorentzvector!!! I dont care about PSG mass!
    Double_t eKin = energy - mass;
    if(eKin < 0.0) eKin=0.0; // sometimes due to different PDG masses between ROOT and G4!!!!!!
    // --> Calculate number of points
    Int_t nPoints = 0;
    for (Int_t iDet=kRICH; iDet<=kHYP; iDet++) {
      pair<Int_t, Int_t> a(i, iDet);
      if ( fPointsMap.find(a) != fPointsMap.end() )
	nPoints += fPointsMap[a];
    }

    // --> Check for cuts (store primaries in any case)
    if (iMother < 0)            store = kTRUE;
    else {
      if (!fStoreSecondaries)   store = kFALSE;
      if (nPoints < fMinPoints) store = kFALSE;
      if (eKin < fEnergyCut)    store = kFALSE;
    }

    // --> Set storage flag
    fStoreMap[i] = store;

  }

  // --> If flag is set, flag recursively mothers of selected tracks
  if (fStoreMothers) {
    for (Int_t i=0; i<fNParticles; i++) {
      if (fStoreMap[i]) {
	Int_t iMother = GetParticle(i)->GetMother(0);
	while(iMother >= 0) {
	  fStoreMap[iMother] = kTRUE;
	  iMother = GetParticle(iMother)->GetMother(0);
	}
      }
    }
  }

}
// -------------------------------------------------------------------------



ClassImp(PndStack)
