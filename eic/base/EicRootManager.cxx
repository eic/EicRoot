//
// AYK (ayk@bnl.gov), 2014/07/15
//
//   Trivial input branch synchronization hack;
//

#include <assert.h>

#include <EicRootManager.h>

EicRootManager* EicRootManager::mInstance = 0;

// ---------------------------------------------------------------------------------------

EicRootManager::EicRootManager(const char *mocaInFile, const char *digiInFile, 
			       const char *recoInFile, const char *assyInFile)
{
  ResetVars();

  if (mInstance) {
    printf("EicRootManager is a singleton instance, do not try to initialize twice!\n");
    return;
  } //if

  // Well, yes: assign right away here;
  mInstance = this;

  // Initialize input files; do not mind to unify few lines here;
  if (mocaInFile) mMoCaFile = new EicRootInputFile(mocaInFile);
  if (digiInFile) mDigiFile = new EicRootInputFile(digiInFile);
  if (recoInFile) mRecoFile = new EicRootInputFile(recoInFile);
  // Typically EicEventAssembler task will be launched from reconstruction.C script;
  // allow to run it separately though;
  if (recoInFile || assyInFile) 
    mAssyFile = new EicRootInputFile(assyInFile ? assyInFile : recoInFile);

  // Initialize branches; request certain synchronization which ROOT can provide on its own;
  if (mAssyFile) {
    mEicRcTree = mAssyFile->GetTree(_EIC_RC_TREE_);
    if (!mEicRcTree) return;

    mEicRcEventBranch = SetupBranch(mAssyFile, _EIC_RC_TREE_, _EIC_RC_BRANCH_, (void **)&mEicRcEvent);
  } //if

  if (mMoCaFile) {
    mPndMcBranch = SetupBranch(mMoCaFile, _CBM_TREE_, _PND_MC_BRANCH_, (void**)&mPndMCTracks);
    
    mGeneratorBranch = SetupBranch(mMoCaFile, _EIC_GENERATOR_TREE_, _EIC_GENERATOR_EVENT_BRANCH_, 
				   (void **)&mGeneratorEvent);
  } //if

  if (mRecoFile) 
    mPndPidBranch = SetupBranch(mRecoFile, _CBM_TREE_, _PND_RC_TRACK_BRANCH_, 
				(void **)&mPndPidChargedCand);
} // EicRootManager::EicRootManager()

// ---------------------------------------------------------------------------------------

EicRootManager::EicRootManager(erhic::EventMC **generatorEventHack, TClonesArray *PndMCTracks, 
			       TClonesArray *PndPidChargedCand, EicRcEvent **EicRcEventHack)
{
  ResetVars();

  if (mInstance) {
    printf("EicRootManager is a singleton instance, do not try to initialize twice!\n");
    return;
  } //if

  // Well, yes: assign right away here;
  mInstance = this;

  mGeneratorEventHack = generatorEventHack;
  mPndMCTracks        = PndMCTracks;
  mPndPidChargedCand  = PndPidChargedCand;
  mEicRcEventHack     = EicRcEventHack;
} // EicRootManager::EicRootManager()

// ---------------------------------------------------------------------------------------

//
// FIXME: at some point figure out how to load these branches automatically;
//

void EicRootManager::SynchronizeBranch(TBranch *branch)
{
  // This check is sufficient for the same code to work in production mode (direct 
  // access to preloaded TCloneArray pointers) as well as user mode (brach 
  // synchronization needed);
  if (!branch) return;

  // Yes, assume mEicRcTree governs synchronization; FIXME: what a crap!;
  int currentEntry = mEicRcTree->GetReadEntry();

  if (mBranchStatus.at(branch) != currentEntry) {
    branch->GetEntry(currentEntry);

    mBranchStatus[branch] = currentEntry;
  } //if
} // EicRootManager::SynchronizeBranch()

// ---------------------------------------------------------------------------------------

TBranch *EicRootManager::SetupBranch(const EicRootInputFile *tfile, const char *treeName, 
					   const char *branchName, void *addr)
{
  if (!tfile) return 0;

  TBranch *branch = tfile->SetupBranch(treeName, branchName, addr);
  if (!branch) return 0;

  mBranchStatus[branch] = -1;

  // Add friend trees right here, so that at least 'rctree->GetEntry(iq)' in analysis.C
  // or similar purpose script works correctly;
  {
    TTree *ttree = tfile->GetTree(treeName);

    if (ttree != mEicRcTree) mEicRcTree->AddFriend(ttree);
  }

  return branch;
} // EicRootManager::SetupBranch()

// ---------------------------------------------------------------------------------------

const erhic::EventMC* EicRootManager::GetGenMcEvent() const
{
  if (!GetGeneratorEventPtr()) return 0;

  // THINK: really needed here?;
  Instance()->SynchronizeBranch(GetGenBranch());

  return GetGeneratorEventPtr();
} // EicRootManager::GetGenMcEvent()

// ---------------------------------------------------------------------------------------

const erhic::ParticleMC* EicRootManager::GetGenMcTrack(int genMcIndex) const
{
  if (!GetGeneratorEventPtr() || genMcIndex < 0) return 0;

  Instance()->SynchronizeBranch(GetGenBranch());

  return (genMcIndex < GetGeneratorEventPtr()->GetNTracks() ? 
	  GetGeneratorEventPtr()->GetTrack(genMcIndex) : 0);
} // EicRootManager::GetGenMcTrack()

// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

EicRootInputFile::EicRootInputFile(const char *fileName)
{
  ResetVars();

  mFileName = TString(fileName);

  mFptr = new TFile(fileName);

  if (!mFptr->IsOpen()) {
    printf("Failed to open input file '%s'!\n", fileName);
    return;
  } //if
} // EicRootInputFile::EicRootInputFile()

// ---------------------------------------------------------------------------------------

TBranch *EicRootInputFile::SetupBranch(const char *treeName, const char *branchName, void *addr) const
{
  if (!mFptr) return 0;

  TTree *tree = (TTree*)mFptr->Get(treeName); 
  if (!tree) {
    printf("Tree '%s' is missing in the input file '%s'!\n", treeName, mFileName.Data());
    return 0;
  } //if

  TBranch *branch = tree->GetBranch(branchName);

  if (!branch) return 0;

  tree->SetBranchAddress(branchName, addr);

  return branch;
} // EicRootInputFile::SetupBranch()

// ---------------------------------------------------------------------------------------

ClassImp(EicRootManager)
ClassImp(EicRootInputFile)

