//
// AYK (ayk@bnl.gov), 2015/11/06
//
//  A (temporary) hack to import HIJING ASCII files with the same interface 
//  calls which are provided with the EicBox Generator;
//

#include <assert.h>
#include <stdlib.h>

#include <TRandom.h>
#include <TMath.h>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>

#include <EicAsciiBoxGenerator.h>

// ---------------------------------------------------------------------------------------

EicAsciiBoxGenerator::EicAsciiBoxGenerator(const char *fileName): 
  mFstream(0), mPtMin(0.0), mPtMax(0.0), mRemainingTrackCounter(0)
{
  if (fileName) {
    char title[1024];

    // Open input stream, check title and skip few lines; FIXME: more checks needed!;
    mFstream = new std::fstream(fileName);
    mFstream->getline(title, 1024-1);
    if (strcmp(title, "  HIJING EVENT FILE")) {
      printf("no HIJING file header line found!\n");
      mFstream->close();
      mFstream = 0;
    } //if
    //printf("%s\n", buffer); exit(0);
  } //if
} // EicAsciiBoxGenerator::EicAsciiBoxGenerator()

// ---------------------------------------------------------------------------------------

Bool_t EicAsciiBoxGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  //printf("Entering EicAsciiBoxGenerator::ReadEvent() ...\n");

  // The return value logic is to fail in case of no input sream at all, but keep
  // returning kTRUE with no tracks in case input file is over;
  if (!mFstream) return kFALSE;

  // Vertex; ignore the stuff in HIJING file for now; FIXME: make this 
  // behaviour configurable;
  double vtx[3];
  //for(unsigned iq=0; iq<3; iq++) 
  //vtx[iq] = mCoord[iq] + (mCoordSigma[iq] ? gRandom->Gaus(0.0, mCoordSigma[iq]) : 0.0);
  //printf("%f %f\n", vtx[0], vtx[1]);
  // FIXME: should unify with EicBoxGenerator call;
  for(unsigned iq=0; iq<3; iq++) {
    vtx[iq] = mCoord[iq] +
      (mGaussianCoordinateSmearing ?
       (mCoordSigma[iq] ? gRandom->Gaus(0.0, mCoordSigma[iq]) : 0.0) :
       (mCoordRange[iq] ? gRandom->Uniform(-mCoordRange[iq]/2, mCoordRange[iq]/2) : 0.0));
  } //for iq

  unsigned thisEventTrackCounter = 0;

  // FIXME: need to break if event list is over;
  for( ; ; ) {
    //printf("   Entering inf.loop with %5d remaining tracks ...\n", mRemainingTrackCounter);
    if (!mRemainingTrackCounter) {
      // Skip few lines;
      char buffer[1024];
      for(unsigned iq=0; iq<5; iq++) 
	mFstream->getline(buffer, 1024-1);
      if (IsOver()) return kTRUE;

      // Get track count; arrange the loop; FIXME: the code is very unsafe;
      {
	double d0;
	unsigned i0, i1, i2;

	*mFstream >> i0 >> d0 >> i1 >> i2 >> mRemainingTrackCounter;
	if (IsOver()) return kTRUE;
      }
      //printf("        Reading event header: %5d new tracks ...\n", mRemainingTrackCounter);

      //printf("%d\n", mRemainingTrackCounter); 
      // Skip this current line and the next one;
      {
	char buffer[1024];
	
	for(unsigned iq=0; iq<2; iq++) 
	  mFstream->getline(buffer, 1024-1);
	if (IsOver()) return kTRUE;
      }
    } //if

    // Loop though all tracks (or as many as requested);
    for(unsigned tr=0; tr<mRemainingTrackCounter; tr++) {
      int pdg;
      double px, py, pz;
      {
	unsigned i0, i1, i2, i3, i4, i5;
	double d0, d1, d2, d3, d4;
	*mFstream >> i0 >> pdg >> i1 >> i2 >> i3 >> i4 >> i5 >> px >> py >> pz >> d0 >> d1 >> d2 >> d3 >> d4;
	if (IsOver()) return kTRUE;
      }
      //printf("%4d -> %5d; %8.3f %8.3f %8.3f\n", tr, pdg, px, py, pz);

      // A hack to skip beam particle instance(s) which are a plenty in the
      // file I have in hands;
      if (!px && !py) continue;

      // Well, I guess for now I do not need neutrals?;
      TDatabasePDG *pdgTable = TDatabasePDG::Instance(); assert(pdgTable);
      TParticlePDG *particle = pdgTable->GetParticle(pdg);
      if (!particle->Charge()) continue;
      //printf("%4d -> %f\n", pdg, particle->Charge());

      // And the perform acceptance checks if EicBoxGenerator was configured so;
      TVector3 p(px, py, pz);
      // FIXME: do it better later (unify)?;
      if (mPtMin || mPtMax) {
	if (p.Pt() < mPtMin || p.Pt() > mPtMax) continue;
      } //if
      if (mPmin || mPmax) {
	if (p.Mag() < mPmin || p.Mag() > mPmax) continue;
      } //if
      if (mThetaMin || mThetaMax) {
	double thetaDeg = p.Theta()*180.0/TMath::Pi();
	if (thetaDeg < mThetaMin || thetaDeg > mThetaMax) continue;
      } //if
      if (mPhiMin || mPhiMax) {
	double phiDeg = p.Phi()*180.0/TMath::Pi();
	if (phiDeg < mPhiMin || phiDeg > mPhiMax) continue;
      } //if

      {
	// FIXME: unify all these pieces of code in all generators;
	TVector3 pvect = GetModifiedTrack(TVector3(px, py, pz));
	//primGen->AddTrack(pdg, px, py, pz, vtx[0], vtx[1], vtx[2]);
	primGen->AddTrack(pdg, pvect[0], pvect[1], pvect[2], vtx[0], vtx[1], vtx[2]);
      }

      thisEventTrackCounter++;
      //printf(" (tr=%5d): this event tr.counter --> %5d\n", tr, thisEventTrackCounter);
      if (mMult && thisEventTrackCounter == mMult) {
	mRemainingTrackCounter -= (tr+1);
	//printf("Exiting track loop; %5d tracks remaining ...\n", mRemainingTrackCounter);
	break;
      } //if
    } //for tr

    // Help the default logic (when all tracks in a given Hijing event are read in); 
    if (!mMult || (mMult && thisEventTrackCounter != mMult)) mRemainingTrackCounter = 0;

    // Get ready for the next event;
    if (!mRemainingTrackCounter)
    {
      char buffer[1024];

      //printf("No more tracks, getting ready to the next event ...\n");
      mFstream->getline(buffer, 1024-1);
      if (IsOver()) return kTRUE;
    } //if

    // In case of either default behaviour or mMult tracks spooled out:
    // break out of the infinite loop;
    if (!mMult || (mMult && thisEventTrackCounter == mMult)) break;
  } //for inf

  return kTRUE;
} // EicAsciiBoxGenerator::ReadEvent()

// ---------------------------------------------------------------------------------------

ClassImp(EicAsciiBoxGenerator)
