//
// AYK (ayk@bnl.gov), 2014/07/15
//
//   For whatever reason neither TRef nor making friends work in my case, 
// where eg in cbmsim->Draw() command like
//
//   cbmsim->Draw("EicRcParticle.mc()->GetMomentum().Mag()");
//
//  EicRcParticle.mc() accesses MCTrack entry internally, but respective
//  branch does not get synchronized automatically;
//
//  NB: this stuff is to be used in user ROOT macros only, like:
//
//  TString mcInFile  = "simulation.root";
//  TString rcInFile  = "reconstruction.root";
//
//  EicRootManager *io = new EicRootManager(mcInFile, 0, rcInFile);
//  TTree *rctree = io->GetEicRcTree();
//
//    ... and now 'rctree' is the basic tree with EicRcEvent entries and 
//  all the links to other trees/branches are setup internally in a way 
//  they are accessible when needed to retrieve quantities like 
//
//    io->GetEicRcEvent()->GetTrack(0)->genmc()->GetPz()
//

#include <map>

#include <TClonesArray.h>
#include <TBranch.h>
#include <TTree.h>
#include <TFile.h>

#include <EicRcEvent.h>
#include <EicEventAssembler.h>
#include <EicEventGenerator.h>

#ifndef _EIC_ROOT_MANAGER_
#define _EIC_ROOT_MANAGER_

#define _CBM_TREE_            "cbmsim"

class EicRootInputFile: public TObject {
  friend class EicRootManager;

 public:
  EicRootInputFile() { ResetVars(); };
  EicRootInputFile(const char *fileName);
  ~EicRootInputFile() { if (mFptr) mFptr->Close(); ResetVars(); };

  void ResetVars() { 
    mFptr = 0;
  };

 private:
  TTree *GetTree(const char *treeName) const { return (TTree*)mFptr->Get(treeName); };
  // 2015/10/12: had to change 'void **addr' to 'void *addr' here and in EicRootManager::SetupBranch()
  // in order to make this stuff work again under Jul'2015 bundled ROOT 5 version;
  // otherwise errors in analysis.C like
  //
  //  Error in <TTree::SetBranchAddress>: Unable to determine the type given for the address for "EicRcEvent"
  //
  // occured; 
  TBranch *SetupBranch(const char *treeName, const char *branchName, void *addr) const;

  TString mFileName;
  TFile *mFptr;

  ClassDef(EicRootInputFile, 2);
};

class EicRootManager: public TObject {
 public:
 EicRootManager() { ResetVars(); };
 EicRootManager(const char *mocaInFile, const char *digiInFile, 
		const char *recoInFile, const char *assyInFile = 0);
 EicRootManager(erhic::EventMC **generatorEventHack, TClonesArray *PndMCTracks, 
		TClonesArray *PndPidChargedCand, EicRcEvent **EicRcEventHack);
 // FIXME: perform proper cleanup later;
 ~EicRootManager() {
   if (mMoCaFile) delete mMoCaFile;
   if (mDigiFile) delete mDigiFile;
   if (mRecoFile) delete mRecoFile;
   if (mAssyFile) delete mAssyFile;

   if (mPndMCTracks) {
     mPndMCTracks->Delete();
     delete mPndMCTracks;
   } //if
   if (mPndPidChargedCand) {
     mPndPidChargedCand->Delete();
     delete mPndPidChargedCand;
   } //if
     
   ResetVars(); 

   mInstance = 0; 
 };
  
  void ResetVars() { 
    mGeneratorEvent = 0;
    mGeneratorEventHack = 0;
    mEicRcEvent = 0;
    mEicRcTree = 0;
    mMoCaFile = mDigiFile = mRecoFile = mAssyFile = 0;

    mEicRcEventHack = 0;

    mPndMCTracks = mPndPidChargedCand = 0; 
    mGeneratorBranch = mPndPidBranch = mPndMcBranch = mEicRcEventBranch = 0;
  };

  static EicRootManager* Instance() { return mInstance; };

  void SynchronizeBranch(TBranch *branch);

  const TClonesArray* GetPndMcTracks()      const { return mPndMCTracks; };
  const TClonesArray* GetPndPidCandidates() const { return mPndPidChargedCand; };

  TBranch* GetGenBranch()                   const { return mGeneratorBranch; };
  TBranch* GetPndMcBranch()                 const { return mPndMcBranch; };
  TBranch* GetPndPidBranch()                const { return mPndPidBranch; };
  TBranch* GetEicRcEventBranch()            const { return mEicRcEventBranch; };

  const TTree *GetEicRcTree()               const { return mEicRcTree; };

  // These calls will perform synchronization if needed;
  const erhic::EventMC*    GetGenMcEvent()               const;
  const erhic::ParticleMC* GetGenMcTrack(int genMcIndex) const;

  const EicRcEvent *GetEicRcEvent()         const { 
    return mEicRcEventHack ? *mEicRcEventHack : mEicRcEvent; 
  };

  // Just two shortcuts (bypass calling GetEicRcTree() in analysis.C); return values
  // match ROOT GetEntries() and GetEntry() call types of course;
  Long64_t GetEicRcTreeEntries()            const { return mEicRcTree ? mEicRcTree->GetEntries() :  0; }
  Int_t    GetEicRcTreeEntry(unsigned ev)   const { return mEicRcTree ? mEicRcTree->GetEntry(ev) : -1; }

 private:
  static EicRootManager *mInstance;                   //!

  // FIXME: the easiest for now; if performance becomes an issue, 
  // optimize for efficiency;
  std::map<const TBranch*, int> mBranchStatus;        //!

  TClonesArray *mPndMCTracks, *mPndPidChargedCand;    //!

  TBranch *mPndPidBranch, *mPndMcBranch;              //!
  erhic::EventMC *mGeneratorEvent;                    //!
  erhic::EventMC **mGeneratorEventHack;               //!

  EicRcEvent *mEicRcEvent;                            //! reconstructed event as a whole
  EicRcEvent **mEicRcEventHack;                       //! 
  TTree *mEicRcTree;                                  //!
  TBranch *mGeneratorBranch, *mEicRcEventBranch;      //!
  
  // Assume there are at most 4 types of input files with predefined tree/branch
  // combinations which may require synchronization; 
  EicRootInputFile *mMoCaFile;                        //!
  EicRootInputFile *mDigiFile;                        //!
  EicRootInputFile *mRecoFile;                        //!
  EicRootInputFile *mAssyFile;                        //!

  TBranch *SetupBranch(const EicRootInputFile *tfile, const char *treeName, 
		       const char *branchName, void *addr);

  // These calls are just to access respective pointers;
  const erhic::EventMC *GetGeneratorEventPtr() const { 
    return mGeneratorEventHack ? *mGeneratorEventHack : mGeneratorEvent; 
  };

  ClassDef(EicRootManager, 25);
};

#endif
