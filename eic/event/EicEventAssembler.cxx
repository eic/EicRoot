//
// AYK (ayk@bnl.gov), 2014/07/08
//
//   Basically a task filling out EicRcTrack, EicRcVertex, etc
//   arrays and do all dirty work to collect the necessary info;

#include <iostream>
using namespace std;

#include <PndPidCandidate.h>
#include <PndMCTrack.h>

#include <EicRootManager.h>
#include <EicRcParticle.h>
#include <EicEventAssembler.h>
#include <EicCalorimeterReconstruction.h>

// ---------------------------------------------------------------------------------------

EicEventAssembler::EicEventAssembler(): 
  mPndMCTracks(0), 
  mPndPidChargedCand(0), 
  mMappingTable(0), 
  mGeneratorEvent(0),
  mEicRcEvent(0),
  mPersistency(kTRUE),
  mEicRcEventBranch(0),
  FairTask("EIC EVENT ASSEMBLER")
{

} // EicEventAssembler::EicEventAssembler()

// ---------------------------------------------------------------------------------------

int EicEventAssembler::AddCalorimeterCore(const char *name, CalorimeterType type)
{
  // FIXME: consider to check for double-counting;
  mCalorimeters.push_back(new EicCalorimeterHub(name, type));

  return 0;
} // EicEventAssembler::AddCalorimeterCore()

// ---------------------------------------------------------------------------------------

int EicEventAssembler::AddEmCal(const char *name)
{
  return AddCalorimeterCore(name, qEmCal);
} // EicEventAssembler::AddEmCal()

// ---------------------------------------------------------------------------------------

int EicEventAssembler::AddHCal(const char *name)
{
  return AddCalorimeterCore(name, qHCal);
} // EicEventAssembler::AddHCal()

// ---------------------------------------------------------------------------------------

InitStatus EicEventAssembler::Init()
{
  // Get access to arrays with PandaRoot MC tracks and charged track candidates;
  FairRootManager* ioman = FairRootManager::Instance();

  if ( !ioman ) {
    cout << "-E- " << "EicEventAssembler::Init(): "
	 << "FairRootManager not instantiated!" << endl;
    return kFATAL;
  } //if

  mPndMCTracks = (TClonesArray*)ioman->GetObject(_PND_MC_BRANCH_);
  if ( !mPndMCTracks ) {
    std::cout << "-E-  EicSmearTask::Init: No MCTrack array found!" << std::endl;
    return kERROR;
  } //if

  mPndPidChargedCand = (TClonesArray *)ioman->GetObject(_PND_RC_TRACK_BRANCH_);

  // Synchronize EICTree (generator info) by hand (if present);
  if (ioman->GetInFile()->Get(_EIC_GENERATOR_TREE_)) {
    TChain *extraTree = new TChain(_EIC_GENERATOR_TREE_);
    extraTree->Add(ioman->GetInFile()->GetName());

    extraTree->SetBranchAddress(_EIC_GENERATOR_EVENT_BRANCH_, &mGeneratorEvent);
    extraTree->SetBranchAddress(_EIC_MAPPING_BRANCH_, &mMappingTable);
    
    ioman->GetInTree()->AddFriend(extraTree);
  } //if

#if 1
  //fRcParticleArray = new TClonesArray("EicRcParticle"); 
  //ioman->Register("EicRcParticle", "EEA", fRcParticleArray, fPersistence);

  {
    EicRootManager *io = new EicRootManager(&mGeneratorEvent, mPndMCTracks, 
					    mPndPidChargedCand, &mEicRcEvent);

    if ( !io ) {
      cout << "-E- " << "EicEventAssembler::Init(): "
	   << "EicRootManager failed to instantiate!" << endl;
      return kFATAL;
    } //if
  }
#endif

  return kSUCCESS;
} // EicEventAssembler::Init()

// ---------------------------------------------------------------------------------------

void EicEventAssembler::ComposeCalorimeterInformation()
{
  // Loop through all particles;
  for(unsigned pt=0; pt<mEicRcEvent->mParticles.size(); pt++) {
    EicRcParticle *particle = mEicRcEvent->mParticles[pt];

    // Loop through all the contributing hits; for now do not make any attempt to 
    // decouple shared hits; FIXME: a more intelligent algorithm is needed later;
    for(unsigned cl=0; cl<particle->mCellGroups.size(); cl++) {
      std::pair<unsigned, unsigned> value = particle->mCellGroups[cl];
      EicCalorimeterHub *chub = mCalorimeters[value.first];
      // Depending on calorimeter type populate either EmCal or HCal variables; 
      // CHECK: qUndefined can not happen, right?;
      EicRcCalorimeterHit **ptrHit = chub->mType == qEmCal ? &particle->mEmCal : &particle->mHCal;
      if (!*ptrHit) *ptrHit = new EicRcCalorimeterHit(); 
      EicRcCalorimeterHit *hit = *ptrHit;

      // Fine, now I have this pointer defined; proceed further; get cluster group instance;
      CalorimeterCellGroup *group = (CalorimeterCellGroup *)chub->mClusters->At(value.second);

      // Loop through all the members and select ones matching mPndMCTrackIndex;
      for(std::map<std::pair<UInt_t, UInt_t>, Double_t>::iterator it=group->mEnergyPerParent.begin(); 
	  it != group->mEnergyPerParent.end(); it++) {
	// Artificial match: require that "true" sub-cluster parent has the same PANDA 
	// MC index as the charged track; once at least one such sub-cluster found, break;
	if (it->first.second == particle->mPndMCTrackIndex) {
	  hit->mEnergy += it->second;

	  // Assume, that there is a generic calorimeter-specific call which gives energy 
	  // error estimate; may want to do cluster-specific call later if needed;
	  double sigma = chub->EnergyErrorEstimate(it->second);
	  hit->mEnergySigma += sigma * sigma;

	  //@@@particle->mRcEmCalHitLocation += hit->mEnergy * hit->mLocation;
	} //if
      } //for it (group->mEnergyPerParent)
    } //for cl

    // Renormalize energy resolution estimates;
    for(unsigned eh=0; eh<2; eh++) {
      EicRcCalorimeterHit *hit = eh ? particle->mHCal : particle->mEmCal;

      if (hit) {
	hit->mEnergySigma = sqrt(hit->mEnergySigma);

	//@@@particle->mRcEmCalHitLocation *= 1./particle->mRcEmCalEnergy;
      } //if
    } //for ct
  } //for pt
} // EicEventAssembler::ComposeCalorimeterInformation()

// ---------------------------------------------------------------------------------------

#define _PHOTON_PDG_CODE_      22
#define _PION_PDG_CODE_       211
#define _POSITRON_PDG_CODE_  (-11)

void EicEventAssembler::PerformPidCalculations()
{
  // For now simply loop through all particles, select the one with Generator index
  // equal 2 (or PndMCTrack index equal 0) and let it be scattered lepton;
  for(unsigned pt=0; pt<mEicRcEvent->mParticles.size(); pt++) {
    EicRcParticle *particle = mEicRcEvent->mParticles[pt];
    
    // Well, the logic is: no charged track -> let it be photon, no matter what 
    // the values of EmCal and HCal energies are;
    if (particle->mPndPidChargedCandIndex == -1)
      particle->mRcPdgCode = _PHOTON_PDG_CODE_;
    else {
      // Otherwise consider a crude e/p check for now; need to use sample 
      // lepton and hadron energy distributions in each calorimeter type 
      // to take a better decision;
      PndPidCandidate* pidcand = 
	(PndPidCandidate*)mPndPidChargedCand->At(particle->mPndPidChargedCandIndex);

      // A clear hack for now; just want to check, that DisKinmatics calculator works;
#if 1
      particle->mRcPdgCode = _POSITRON_PDG_CODE_;
#else
      if (particle->mEmCal) {
	double diff = particle->mEmCal->mEnergy - particle->mRcVtxMomentum.Mag();
	double sigma = sqrt(particle->mEmCal->mEnergySigma    * particle->mEmCal->mEnergySigma + 
			    particle->mRcTrackerMomentumSigma * particle->mRcTrackerMomentumSigma);
	// Assume any positive value is fine?;
	if (diff > 0.0 || fabs(diff/sigma) < 3.0)
	  particle->mRcPdgCode = _POSITRON_PDG_CODE_;
	else
	  particle->mRcPdgCode = _PION_PDG_CODE_;
      } 
      else
	particle->mRcPdgCode = _PION_PDG_CODE_;
#endif

      particle->mRcPdgCode *= pidcand->GetCharge();
    } //if

    //@@@particle->mRcPdgCode = _PION_PDG_CODE_ * pidcand->GetCharge();
    //@@@particle->mHadronPidCL.push_back(std::pair<EicRcParticle::HadronPidType, float>
    //@@@ (EicRcParticle::PionKaonProton, 1.0));
    //@@@
    //@@@particle->mChargeDefCL = particle->mElectronPidCL = 0.0;

    //printf("%2d -> %d %d\n", pt, particle->mRcPdgCode, particle->GetMcPdgCode());
  } //for pt
} // EicEventAssembler::PerformPidCalculations()

// ---------------------------------------------------------------------------------------

void EicEventAssembler::ReAssignMomentumValue()
{
  //
  // FIXME: there is an implicit interplay between PerformPidCalculations() and 
  // ReAssignMomentumValue() in a sense they use the same input information to 
  // make logical assignments; perhaps makes sense to unify these two calls;
  //

  for(unsigned pt=0; pt<mEicRcEvent->mParticles.size(); pt++) {
    EicRcParticle *particle = mEicRcEvent->mParticles[pt];   

    // Trivial case: photon; this would mean no charged track and EmCal information 
    // available; just assign 3D momentum based on e/m cluster energy and location;
    if (         particle->mRcPdgCode  == _PHOTON_PDG_CODE_) {
      // FIXME: think, what to do if no EmCal info (and no track!) available;
      assert(particle->mEmCal);

      particle->mRcVtxMomentum = particle->mEmCal->mEnergy * particle->mEmCal->mLocation.Unit();
    }
    // Electron: assume tracking coverage is sufficient to claim, that in case no 
    // charged track seen in the detector, particle could not have been electron (and thus 
    // would be photon); if EmCal info available (and most likely it is and was used in 
    // e/p decision), use weighted method (mix tracking and e/m calorimeter info);
    else if (abs(particle->mRcPdgCode) == _POSITRON_PDG_CODE_) {
      // Otherwise do not do anything (particle->mRcVtxMomentum will remain as assigned by tracker);
      if (particle->mEmCal) {
	double wtE = 1./(particle->mEmCal->mEnergySigma*particle->mEmCal->mEnergySigma);
	double wtP = 1./(particle->mRcTrackerMomentumSigma*particle->mRcTrackerMomentumSigma);
	// Ignore electron mass, sorry;
	double E = (wtE*particle->mEmCal->mEnergy + wtP*particle->mRcVtxMomentum.Mag())/(wtE+wtP);
	// NB: direction still take from tracking;
	particle->mRcVtxMomentum = E * particle->mRcVtxMomentum.Unit();
      } //if
    } 
    // For pions (no RICH and other hadron PID devices as of yet) assume charged track 
    // was always avaliable; yet try to account HCal+EmCal energy;
    else if (abs(particle->mRcPdgCode) == _PION_PDG_CODE_) {
      // Otherwise will remain as assigned by tracking;
      if (particle->mHCal) {
	// NB: this will also work if either one of the calorimeter parts (EmCal or HCal) was missing;
	double fullCalorimeterEnergy = 0.0, fullCalorimeterEnergySigma = 0.0;
	for(unsigned eh=0; eh<2; eh++) {
	  EicRcCalorimeterHit *hit = eh ? particle->mHCal : particle->mEmCal;
	  
	  fullCalorimeterEnergy      += hit->mEnergy;
	  fullCalorimeterEnergySigma += hit->mEnergySigma;
	} //for ct
	fullCalorimeterEnergySigma = sqrt(fullCalorimeterEnergySigma);
	
	// FIXME: unify this part with electron case (see above);
	double wtE = 1./(fullCalorimeterEnergySigma*fullCalorimeterEnergySigma);
	double wtP = 1./(particle->mRcTrackerMomentumSigma*particle->mRcTrackerMomentumSigma);
	// FIXME: ignore pion mass, sorry;
	double E = (wtE*fullCalorimeterEnergy + wtP*particle->mRcVtxMomentum.Mag())/(wtE+wtP);
	particle->mRcVtxMomentum = E * particle->mRcVtxMomentum.Unit();
      } //if
    } //if
  } //for pt
} // EicEventAssembler::ReAssignMomentumValue()

// ---------------------------------------------------------------------------------------

void EicEventAssembler::AssignScatteredLepton()
{
  // Get beam lepton charge (sign); FIXME: may want to assign once?;
  const erhic::ParticleMC* beamLepton = mEicRcEvent->BeamLepton();
  //printf("%p\n", beamLepton); assert(0);
  if (!beamLepton) return;

  Int_t beamLeptonPDG = beamLepton->GetPdgCode().Info()->PdgCode();
  //printf("beam lepton PDG: %d\n", beamLeptonPDG);
  //printf("old sc.lepton ID: %d\n", mEicRcEvent->mScatteredLeptonID);

  mEicRcEvent->mScatteredLeptonID = -1;
  for(unsigned pt=0; pt<mEicRcEvent->mParticles.size(); pt++) {
    EicRcParticle *particle = mEicRcEvent->mParticles[pt]; 
    //printf("Beam PDG: %d; my PDG: %d\n", beamLeptonPDG, particle->mRcPdgCode);
    if (particle->mRcPdgCode == beamLeptonPDG && 
#if 1
	// Yes, for now want ti fake, sorry; no real PID and comparing energies
	// only (since all charged particles are given +/-11 PDG code) is just misleading;
	particle->GetMcPdgCode() == beamLeptonPDG &&
#endif
	(mEicRcEvent->mScatteredLeptonID == -1 || 
	 particle->GetE() > mEicRcEvent->mParticles[mEicRcEvent->mScatteredLeptonID]->GetE()))

      mEicRcEvent->mScatteredLeptonID = pt;
  } //for pt

  printf("scattered lepton: %2d\n", mEicRcEvent->mScatteredLeptonID);
} // EicEventAssembler::AssignScatteredLepton()

// ---------------------------------------------------------------------------------------

void EicEventAssembler::Exec(Option_t* opt)
{
  //printf("  EicEventAssembler::Exec() %3d ...\n", mPndPidChargedCand->GetEntriesFast());

  //return;



  //static unsigned evcounter;
  //evcounter++;

  if (!mEicRcEventBranch) {
    TTree *outputTree = new TTree(_EIC_RC_TREE_, "A tree of reconstructed events");

    // The easiest: do not switch pointers all the time; THINK: multi-threading?; 
    mEicRcEventBranch = outputTree->Branch(_EIC_RC_BRANCH_, _EIC_RC_BRANCH_,
					   &mEicRcEvent, 32000, 99);
    //printf(" --> will be writing to %s ...\n", outputTree->GetCurrentFile()->GetName()); exit(0);

    mEicRcEvent = new EicRcEvent();
   
    {
      FairRootManager* ioman = FairRootManager::Instance(); assert(ioman);
      
      for(unsigned ct=0; ct<mCalorimeters.size(); ct++) {
	EicCalorimeterHub *chub = mCalorimeters[ct];

	chub->mClusters = (TClonesArray *)ioman->GetObject(chub->mName->Name() + "ClusterGroup");
      } //for ct (mCalorimeters)
    } 
  } //if

  //rcEvent->ClearParticles();
  mEicRcEvent->mParticles.clear();

  //fRcParticleArray->Clear();
  //if (mtable) cout << "@@@ -> " << mtable->GetData().size() << endl;

  // First, loop through the reconstructed charged tracks and (no matter how they were 
  // reconstructed and whether mc-to-rc correspondence was established by hand)
  // allocate one EicRcParticle instance per track;
  if (mPndPidChargedCand)
    for(unsigned ch=0; ch<mPndPidChargedCand->GetEntriesFast(); ch++) {
      PndPidCandidate* pidcand = (PndPidCandidate*)mPndPidChargedCand->At(ch);

      // Allocate new reconstructed track;
      EicRcParticle *particle = new EicRcParticle();
      //new((*fRcParticleArray)[fRcParticleArray->GetEntriesFast()]) EicRcParticle();
      
      // Assign PidChargedCand and (may be) MCTrack array indices;
      particle->mPndPidChargedCandIndex = ch;
      
      // Pull out few most needed variables from PndMCTrack class;
      int mcId = pidcand->GetMcIndex();
      if (mcId >= 0) {
	particle->mPndMCTrackIndex = mcId;
	
	// Well, it looks like no range checks, etc are needed here?;
	if (mMappingTable) particle->mGeneratorEventIndex = mMappingTable->GetData()[mcId];
	
	// FIXME: may want to return back at least 'mctrack' assignment;
	//PndMCTrack *mctrack = (PndMCTrack*)mPndMCTracks->At(mcId);
      } //if
      
      // Pull out few most needed variables from PndPidCandidate class;
      particle->mRcVertex               = pidcand->GetPosition();
      particle->mRcVtxMomentum          = pidcand->GetMomentum();
      particle->mRcTrackerMomentumSigma = 
	sqrt(pidcand->GetCov()[0])*pow(particle->mRcVtxMomentum.Mag(), 2);//*particle->mRcVtxMomentum;
      //printf("%7.1f +/- %8.4f\n", particle->mRcVtxMomentum.Mag(), particle->mRcTrackerMomentumSigma);

      printf("%3d MC PDG: %5d (E=%7.3f) --> %3d\n", ch, particle->GetMcPdgCode(),
	     particle->GetE(), particle->mPndMCTrackIndex);

      mEicRcEvent->mParticles.push_back(particle);
    } //if..for ch

  // Then loop through all calorimeter clusters (cell groups); at this stage want to 
  // relate track to *all* clusters which can potentially belong to it (even partly); 
  // this procedure can be either based on MC track and cluster "parent" IDs (as now)
  // or done based on a match in space (later);
#if _LATER_
  for(unsigned ct=0; ct<mCalorimeters.size(); ct++) {
    EicCalorimeterHub *chub = mCalorimeters[ct];
    
    if (!chub->mClusters) continue;

    printf("Here: %d \n", chub->mClusters->GetEntriesFast());

    for(unsigned cl=0; cl<chub->mClusters->GetEntriesFast(); cl++) {
      CalorimeterCellGroup *group = (CalorimeterCellGroup *)chub->mClusters->At(cl);

      bool found = false;

      // Carelessly loop through all the EicRcParticle instances and try to establish
      // either track-to-cluster or cluster-to-cluster relation;
      for(unsigned pt=0; pt<mEicRcEvent->mParticles.size(); pt++) {
	EicRcParticle *particle = mEicRcEvent->mParticles[pt];   

	for(std::map<std::pair<UInt_t, UInt_t>, Double_t>::iterator it=group->mEnergyPerParent.begin(); 
	    it != group->mEnergyPerParent.end(); it++) {
	  // Artificial match: require that "true" sub-cluster parent has the same PANDA 
	  // MC index as the charged track; once at least one such sub-cluster found, break;
	  if (it->first.second == particle->mPndMCTrackIndex) {
	    particle->mCellGroups.push_back(std::pair<unsigned, unsigned>(ct, cl));
	    found = true;

	    // Yes, fall just into 'pt' loop and continue (there can be more than one 
	    // charged particle track associated with this cluster group);
	    break;
	  } //if

	  //printf("%3d %3d -> %f\n", it->first.first, it->first.second, it->second);
	} //for it
      _next_particle:
	continue;
      } //for pt
      
      // Unless at least one EicRcParticle instance match found, create a new one for every 
      // parent; yes, these instances will only contain this calorimeter info (yet mPndMCTrackIndex
      // will be assigned);
      if (!found) {
	// Keep track on registered parents;
	std::set<unsigned> registered;

	for(std::map<std::pair<UInt_t, UInt_t>, Double_t>::iterator it=group->mEnergyPerParent.begin(); 
	    it != group->mEnergyPerParent.end(); it++) 
	  if (registered.find(it->first.second) == registered.end()) {
	    EicRcParticle *particle = new EicRcParticle();

	    particle->mPndMCTrackIndex = it->first.second;
	    particle->mCellGroups.push_back(std::pair<unsigned, unsigned>(ct, cl));
	    mEicRcEvent->mParticles.push_back(particle);

	    registered.insert(it->first.second);
	  } //for it (group->mEnergyPerParent)..if
      } //if
    } //for cl
  } //for ct (mCalorimeters)

  // Combine and unify information from all calorimeters;
  ComposeCalorimeterInformation();
#endif

  // Pretty naive (ideal) for now;
  PerformPidCalculations();

  // Based on PID assignments (re)calculate mRcVtxMomentum;
#if _LATER_
  ReAssignMomentumValue();
#endif

  // Basically check whether at least one electron (positron) with the same
  // charge as beam lepton exist and select the one with the highest momentum;
  AssignScatteredLepton();

  mEicRcEventBranch->GetTree()->Fill();
} // EicEventAssembler::Exec()

// ---------------------------------------------------------------------------------------

void EicEventAssembler::FinishTask()
{
  if (mPersistency && mEicRcEventBranch) {
    //@@@mEicRcEventBranch->GetTree()->GetCurrentFile()->cd();
    mEicRcEventBranch->GetTree()->Write();
  } //if

#if _OFF_
  outFile->cd();
  smearedTree->Write();
#endif

  FairTask::FinishTask();
} // EicEventAssembler::FinishTask()

// ---------------------------------------------------------------------------------------

ClassImp(EicCalorimeterHub)
ClassImp(EicEventAssembler)
