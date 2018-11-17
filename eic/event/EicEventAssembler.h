//
// AYK (ayk@bnl.gov), 2014/07/08
//
//   Basically a task filling out EicRcTrack, EicRcVertex, etc
//   arrays and do all dirty work to collect the necessary info;
//

#include <TClonesArray.h>
#include <TBranch.h>

#include <FairTask.h>

#include <EicRcEvent.h>
#include <EicDetName.h>
#include <EicEventGenerator.h>

#ifndef _EIC_EVENT_ASSEMBLER_
#define _EIC_EVENT_ASSEMBLER_

#define _PND_MC_BRANCH_       "MCTrack"
#define _PND_RC_TRACK_BRANCH_ "PidChargedCand"

#define _EIC_RC_TREE_         "EicRcTree"
#define _EIC_RC_BRANCH_       "EicRcEvent"

enum CalorimeterType {qUndefined, qEmCal, qHCal};

class EicCalorimeterHub: public TObject {
  friend class EicEventAssembler;

 public:
 EicCalorimeterHub(const char *name = 0, CalorimeterType type = qUndefined, 
		   double a = 0.0, double b = 0.0): 
  mName(0), mType(type), mClusters(0), mA(a), mB(b) {
    if (name) mName = new EicDetName(name);
  };
  ~EicCalorimeterHub() {};

  double EnergyErrorEstimate(double energy) {
    return mA/sqrt(energy) + mB;
  };

 private:
  CalorimeterType mType;    // either EmCal or HCal

  EicDetName *mName;        // detector name in all spellings

  TClonesArray *mClusters;  // input array of reconstructed calorimeter clusters

  Double_t mA;              // a/sqrt(E) term in energy resolution formula
  Double_t mB;              // constant term

  ClassDef(EicCalorimeterHub,2)
};

class EicEventAssembler: public FairTask {
 public:
  EicEventAssembler();
  ~EicEventAssembler() {};

  InitStatus Init();

  void Exec(Option_t* opt);

  void FinishTask();

  // Yes, prefer to have two separate user calls;
  int AddEmCal(const char *name);
  int AddHCal (const char *name);

 private:
  /*! input arrays of MC and PandaRoot-reconstructed tracks */
  TClonesArray *mPndMCTracks, *mPndPidChargedCand; //!

  erhic::EventMC *mGeneratorEvent;                 //!
  ParticleMappingTable *mMappingTable;             //!
   
  /*! Object persistency flag */
  Bool_t mPersistency;                             //! 

  EicRcEvent *mEicRcEvent;                         //! reconstructed event as a whole
  TBranch *mEicRcEventBranch;                      //!

  std::vector<EicCalorimeterHub*> mCalorimeters;   // vector of registered calorimeters

  int AddCalorimeterCore(const char *name, CalorimeterType type);

  void ComposeCalorimeterInformation();
  void PerformPidCalculations();
  void ReAssignMomentumValue();
  void AssignScatteredLepton();

  ClassDef(EicEventAssembler, 15);
};

#endif
