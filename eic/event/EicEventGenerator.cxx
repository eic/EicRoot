//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Interface to eic-smear import classes;
//

#include <assert.h>
#include <iostream>

using std::cout;
using std::endl;

#include "FairPrimaryGenerator.h"
#include <FairRootManager.h>
#include <FairRunSim.h>

#include "EicEventGenerator.h"

EicEventGenerator* EicEventGenerator::mInstance = 0;

#ifdef _PROMC_
#include "ProMC.pb.h"
#include "ProMCBook.h"
#endif

#ifdef _EICMC_
#include <EicMCreader.h>
#endif

// =======================================================================================

Poacher::Poacher(const TString &fileName)
{
  // Will not hurt anyway;
  SetInputFileName(std::string(fileName.Data()));

  ResetProMC(); ResetEicMC();

  //printf("%s\n", fileName.Data());
  if (fileName.EndsWith(".promc")) {
#ifdef _PROMC_
    // Open ProMC file; get access to the header fields;
    mProMCBook = new ProMCBook(fileName.Data(), "r");
    // NB: in fact the above constructor terminates the program if file can not be opened;
    if (mProMCBook) {
      ProMCHeader header = mProMCBook->getHeader();

      mMomentumUnits = (double)(header.momentumunit());
      mLengthUnits   = (double)(header.lengthunit());
    } 
    else
      Fatal("Poacher::Poacher()", "failed to open input ProMC file.");

    // Allocate buffer variables;
    mParticle = new erhic::ParticleMC();
    mEventProMC = new EventProMC(); 
#else
    Fatal("Poacher::Poacher()", "ProMC support is not compiled in.");
#endif
  } else if (fileName.EndsWith(".eicmc")) {
#ifdef _EICMC_
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Open EicMC file; get access to the header fields;
    mReaderEicMC = new EicMC::Reader(fileName.Data());

    // Allocate buffer variables;
    mParticle = new erhic::ParticleMC();
    mEventEicMC = new EventEicMC();
#else
    Fatal("Poacher::Poacher()", "EicMC support is not compiled in.");
#endif
  } else {
    // Forester on some generator ASCII file input; open file and jump to the first event;
    // FIXME: there must be a memory leak somewhere here; works with absolute path only (?);
    OpenInput();
    
    FindFirstEvent();
  } //if
} // Poacher::Poacher()

// ---------------------------------------------------------------------------------------

erhic::VirtualEvent *Poacher::GetNextEvent() 
{ 
  if (mProMCBook) {
#ifdef _PROMC_
    // Check, that there are still events in th efile;
    if (mProMCBook->next() != 0) return 0;

    // This also resets the particle list;
    mEventProMC->Clear();

    // Get ProMC event from file, loop through all the particles 
    // and populate erhic::EventMC, which has a format, understandable to the rest of the code;  
    ProMCEvent eve = mProMCBook->get();
    ProMCEvent_Particles *pa=eve.mutable_particles();
    // Well, I have to preserve consistency in the particle array (at least not 
    // to screw up parent-daughter relationship); so put ALL the particles in the 
    // event record; EicEventGenerator::ReadEvent() will take care to select only 
    // those with code '1' for transport and to create proper mapping table;
    for (int j=0; j<pa->pdg_id_size(); j++) {
      // Assign 4-momentum; units seemingly become [GeV] here, fine;
      mParticle->Set4Vector((1.0/mMomentumUnits)*TLorentzVector(pa->px(j), pa->py(j), pa->pz(j), pa->energy(j)));

      // Assign vertex; FIXME: seemingly this way I obtain [cm]?;
      mParticle->SetVertex((0.1/mLengthUnits)*TVector3(pa->x(j), pa->y(j), pa->z(j)));
      
      // Assign status and PDG;
      mParticle->SetStatus((int)pa->status(j));
      mParticle->SetId(pa->pdg_id(j));

      // Assign parent (assume a single one, sorry); base-1 here and below, right?;
      mParticle->SetParentIndex(pa->mother1(j));
      // Assign daughter range; 
      mParticle->SetChild1Index(pa->daughter1(j));
      mParticle->SetChildNIndex(pa->daughter2(j));

      // NB: this call will dereference the pointer and create another instance, so 
      // hopefully there is no screw up with re-using the same pointer here;
      mEventProMC->AddLast(mParticle);

      //printf("%4d (%4d) (%5d) -> %8.2f %8.2f %8.2f ... %8.2f %8.2f %8.2f\n", 
      //     j, mParticle->GetStatus(), mParticle->Id().Code(), 
      //     mParticle->GetPx(), mParticle->GetPy(), mParticle->GetPz(), 
      //     mParticle->GetVertex().X(), mParticle->GetVertex().Y(), mParticle->GetVertex().Z());
    } //for j

    return mEventProMC;
#else
    Fatal("Poacher::Poacher()", "ProMC support is not compiled in."); return 0;
#endif
  } else if (mReaderEicMC) {
#ifdef _EICMC_
    // FIXME: allocate once;
    eicmc::Record record;
    
    // Get next event record; return 0 if end-of-file encountered;
    if (mReaderEicMC->GetNextEvent(record)) return 0;

    const eicmc::Record::MonteCarloEvent &mcevent = record.mcevent();

    for(int pt=0; pt<mcevent.particles_size(); pt++) {
      const eicmc::Record::MonteCarloEvent::Particle &particle = mcevent.particles(pt);

      // Assign 4-momentum; FIXME: energy component!;
      mParticle->Set4Vector(TLorentzVector(particle.px(), particle.py(), particle.pz()));

      // Assign vertex; 
      mParticle->SetVertex(TVector3(particle.vx(), particle.vy(), particle.vz()));
      
      // Assign status and PDG;
      mParticle->SetStatus((int)particle.status());
      mParticle->SetId(particle.pdg());

      // Assign parent (assume a single one, sorry); base-1 here and below, right?;
      mParticle->SetParentIndex(particle.mother1());
      // Assign daughter range; 
      mParticle->SetChild1Index(particle.daughter1());
      mParticle->SetChildNIndex(particle.daughter2());

      // NB: see the comment in ProMC section; same story here;
      mEventEicMC->AddLast(mParticle);
    } //for pt

    return mEventEicMC;
#else
    Fatal("Poacher::Poacher()", "EicMC support is not compiled in."); return 0;
#endif
  } else if (mFactory) {
    return mFactory->Create(); 
  }
  else
    return 0;
} // Poacher::GetNextEvent() 

// ---------------------------------------------------------------------------------------

std::string Poacher::EventName() const 
{ 
  if (mProMCBook) {
    return EventProMC::Class()->GetName();
  } else if (mReaderEicMC) {
    return EventEicMC::Class()->GetName();
  } else if (mFactory) {
    return mFactory->EventName(); 
  } else {
    assert(0); //return "";
  } //if
} // Poacher::EventName()

// =======================================================================================

//EicEventGenerator::EicEventGenerator(TString fileName): FairGenerator()
EicEventGenerator::EicEventGenerator(TString fileName): EicProtoGenerator("EicEventGenerator")
{
  //if (mInstance) {
  //Fatal("EicEventGenerator::EicEventGenerator()", "Singleton instance already exists.");
  //return;
  //} //if
  mInstance = this;

  cout << "-I EicEventGenerator: Using input file(s) " << fileName << endl;

  // Reset private variables to 0; 
  ResetVars();

  // This basically will be a default constructor;
  if (fileName.IsNull()) return;

  // Yes, either ".root" suffix (ROOT file(s)) or any other extension 
  // and then it is some generator output ASCII file;
  if (fileName.EndsWith(".root"))
  {
    mInputTree = new TChain(_EIC_GENERATOR_TREE_);
    // NB: this will work on either single file or a mask like "dir/*.root";
    mInputTree->Add(fileName);
    
    mInputTree->SetBranchAddress(_EIC_GENERATOR_EVENT_BRANCH_, &mGeneratorEvent);
  }
  else
    // If anything goes wrong here, Forester will throw an exception; 
    mPoacher = new Poacher(fileName);

  mMappingTable = new ParticleMappingTable();
} // EicEventGenerator::EicEventGenerator()

// ---------------------------------------------------------------------------------------

//
// Well, the implementation is such, that this call can happen at any time 
// and more than once; not really needed, but do not want to keep track of >1
// calls either; let user be responsible;
//

#include <eicsmear/erhic/EventDpmjet.h>

int EicEventGenerator::SkipFewEvents(unsigned eventsToSkip)
{
  if (mPoacher) {
    // Have to loop through the ASCII file;
    for(unsigned ev=0; ev<eventsToSkip; ev++) 
      if (!mPoacher->GetNextEvent())
	return -1;
  }
  else {
    // In case of ROOT tree input just increment next event counter;
    mInputTreeNextEventId += eventsToSkip;
    //printf(" --> will start from %d\n", mInputTreeNextEventId); exit(0);
    // Well, allow some control over what's going on;
    if (!mInputTree->GetEntry(mInputTreeNextEventId)) return -1;
  } //if

  return 0;
} // EicEventGenerator::SkipFewEvents()

// ---------------------------------------------------------------------------------------

Bool_t EicEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  // Clear re-mapping vector;
  mMappingTable->mData.clear();

  // Fill out the next event record; NB: if PDG selection is made, it can happen, that 
  // there are no primary particles selected; therefore loop until find an event with 
  // at least one particle;
  for( ; ; ) {
    // Check, that event chunk size was not exceeded;
    if (mEventChunkSize && mEventCounter == mEventChunkSize) {
      cout << "-W- EicEventGenerator(): event chunk size limit reached!" << endl;
      
      return kFALSE;
    } //if

    if (mPoacher)
      mGeneratorEvent = mPoacher->GetNextEvent();
    else {
      //printf("%6d ...\n", mInputTreeNextEventId);
      if (!mInputTree->GetEntry(mInputTreeNextEventId++)) mGeneratorEvent = 0;
    } //if

    if (!mGeneratorEvent) {
      cout << "-E- EicEventGenerator(): event list exhausted!" << endl;
      
      return kFALSE;
    } //if
 
    // NB: this counter does not care about event selections, it just counts 
    // all considered input event candidates;
    mEventCounter++;

    // If only leading particle is requested, have to arrange a separate loop;
    int leadingParticleId = -1;
    double leadingParticleMomentum;
    if (mSelectLeadingParticle) {
      for(unsigned iq=0; iq<mGeneratorEvent->GetNTracks(); iq++) { 
	erhic::VirtualParticle *vp = mGeneratorEvent->GetTrack(iq);
      
	// May want to do it better later; suffices for now;
	if (vp->GetStatus() != 1) continue;

	// Well, if PDG selection was applied, check that this particle matches;
	if (mSelectedPdgCodes.size() && mSelectedPdgCodes.find(vp->Id()) == mSelectedPdgCodes.end())
	  continue;

	if (leadingParticleId == -1 || vp->GetP() > leadingParticleMomentum) {
	  leadingParticleId       = iq;
	  leadingParticleMomentum = vp->GetP();
	} //if
      } //for iq

      // Well, if was not able to select a proper leading particle, skip to next event;
      if (leadingParticleId == -1 || 
	  (mSelectedLeadingParticlePmin && leadingParticleMomentum < mSelectedLeadingParticlePmin))
	continue;
    } //if

    {
      //erhic::EventDpmjet *evt = dynamic_cast<erhic::EventDpmjet*>(mGeneratorEvent);
      //assert(evt);
      //printf(" @@@ -> %d %d\n", evt->process1, evt->process2);

    } 

    // Will be the same for all tracks;
    TVector3 vtx = GetSimulatedPrimaryVertex();

    //printf("%d\n", mGeneratorEvent->GetNTracks()); fflush(stdout);
    // Loop through all the particles and feed them to the FairRoot primary generator;
    for(unsigned iq=0; iq<mGeneratorEvent->GetNTracks(); iq++) { 
      erhic::VirtualParticle *vp = mGeneratorEvent->GetTrack(iq);
      
      // May want to do it better later; suffices for now;
      if (vp->GetStatus() != 1) continue;
      
      // Well, if PDG selection was applied, check that this particle matches;
      if (mSelectedPdgCodes.size() && mSelectedPdgCodes.find(vp->Id()) == mSelectedPdgCodes.end())
	continue;
      
      // If leading particle was selected, check exact match;
      if (leadingParticleId != -1 && leadingParticleId != iq) continue;



      //if (vp->GetParentIndex() != 6) continue;



      // Record explicitely all entries in the original erhic::ParticleMC* array
      // which made it into the GEANT tracing engine; if direct encoding proves to 
      // be inconvenient, create inverse table or perhaps a 0/1 map;
      mMappingTable->mData.push_back(iq);

      // Populate energy flow plot if requested;
      if (mEnergyFlowPdgCodes.find(vp->Id()) != mEnergyFlowPdgCodes.end()) {
	//printf(" --> %7.2f; %7.2f %7.2f -> %7.2f\n", vp->GetEta(), 
	//     1000*vp->GetE(), 1000*vp->GetM(), 1000*(vp->GetE() - vp->GetM()));
	if (mEnergyFlowHist)    mEnergyFlowHist   ->Fill(vp->GetEta(), vp->GetE() - vp->GetM());
	if (mParticleCountHist) mParticleCountHist->Fill(vp->GetEta());
      } //if

      // Indeed some of the tracks may originate from a secondary vertex -> calculate a sum;
      TVector3 pvtx = vtx + vp->GetVertex();
      
      // FIXME: at some point check that vtx units match;
      {
	TVector3 pvect = GetModifiedTrack(TVector3(vp->GetPx(), vp->GetPy(), vp->GetPz()));//vp->GetP());
	//primGen->AddTrack(vp->Id(), vp->GetPx(), vp->GetPy(), vp->GetPz(), vtx[0], vtx[1], vtx[2]);
	printf("%7.3f %7.3f %7.3f ... %4d  %2d\n", pvect[0], pvect[1], pvect[2], vp->GetParentIndex(), vp->GetStatus());
	//printf("%7.3f %7.3f %7.3f\n", vp->GetPx(), vp->GetPy(), vp->GetPz());//pvect[0], pvect[1], pvect[2]);
	primGen->AddTrack(vp->Id(), pvect[0], pvect[1], pvect[2], pvtx[0], pvtx[1], pvtx[2]);
      }
    } /*for iq*/
    //printf("stat: %3d -> %3d\n", mGeneratorEvent->GetNTracks(), mMappingTable->mData.size());

    // There are primary tracks selected in this event -> break;
    if (mMappingTable->mData.size()) break;
  } //for inf

  //
  // THINK: for now the idea is that in case input file had no events, output tree/branch(es)
  //        are not created at all; re-shuffle "outputBranch" assignment, etc if this 
  //        ever becomes a problem;
  //

  // It looks like the easiest way is to always copy over the original tree into the same
  // simulation.root output file; it is indeed a duplication in case of .root input -> FIXME; 
  // NB: just to mention it: output simulation.root file can be re-used as ROOT EICTree input file;
  {
    static TBranch *outputEventBranch, *outputMappingBranch;

    if (!outputEventBranch) {
      TTree *outputTree = new TTree(_EIC_GENERATOR_TREE_, "A tree of original Monte Carlo events");
      //outputTree->SetCurrentFile("simulation.root");

      // The easiest: do not switch pointers all the time; THINK: multi-threading?; 
      outputEventBranch = outputTree->Branch(_EIC_GENERATOR_EVENT_BRANCH_, 
					     // Take real class name (like erhic::EventMilou) either 
					     // from poacher (ASCII input) 
					     // or from the input branch directly (.root input); 
					     mPoacher ? mPoacher->EventName().c_str() :
					     mInputTree->GetBranch(_EIC_GENERATOR_EVENT_BRANCH_)->GetClassName(), 
					     &mGeneratorEvent, 32000, 99);

      outputMappingBranch = outputTree->Branch(_EIC_MAPPING_BRANCH_, "ParticleMappingTable", 
					       &mMappingTable, 32000, 99);

      // Arrange a separate FairTask with the only purpose to call Write() at the end of 
      // simulation run; FairRunSim::Instance() pointer should indeed be always available, or?;
      FairRunSim::Instance()->AddTask(new EicEventGeneratorTask(outputTree));
    } //if

    outputEventBranch->GetTree()->Fill();
  }

  return kTRUE;
} // EicEventGenerator::ReadEvent()

// ---------------------------------------------------------------------------------------

void EicEventGeneratorTask::FinishTask()
{
  if (mOutputTree) {
    //printf(" --> writing to %s ...\n", mOutputTree->GetCurrentFile()->GetName());
    mOutputTree->GetCurrentFile()->cd();
    mOutputTree->Write();

    EicEventGenerator *evtGen = EicEventGenerator::Instance();
    if (evtGen && evtGen->mEventCounter) {
      TH1D *hists[2] = {evtGen->mParticleCountHist, evtGen->mEnergyFlowHist};

      for(unsigned ih=0; ih<2; ih++) {
	TH1D *hist = hists[ih];

	if (hist) {
	  hist->Scale(1.0/(evtGen->mEtaBinWidth*evtGen->mEventCounter));
	  hist->Write();
	} //if
      } //for ih
    } //if
  } //if

  FairTask::FinishTask();
} // EicEventGeneratorTask::FinishTask()

// ---------------------------------------------------------------------------------------

ClassImp(EicEventGenerator)
ClassImp(ParticleMappingTable)
ClassImp(EicEventGeneratorTask)

ClassImp(EventProMC)
ClassImp(EventEicMC)

