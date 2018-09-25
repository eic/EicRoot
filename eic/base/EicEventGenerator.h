//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Interface to eic-smear import classes;
//

#include <assert.h>

#include <set>

#ifndef _EIC_EVENT_GENERATOR_
#define _EIC_EVENT_GENERATOR_

#include <TChain.h>
#include <TH1D.h>

#include "FairGenerator.h"
#include "FairTask.h"

#include "eicsmear/erhic/EventFactory.h"
#include "eicsmear/erhic/Forester.h"

// Of course want to use eic-smear code conventions on the tree/branch names;
#define _EIC_GENERATOR_TREE_         "EICTree"
#define _EIC_GENERATOR_EVENT_BRANCH_ "event"

// Basically describes which entries in the original ParticleMC array made
// it into the GEANT input table for tracking;
#define _EIC_MAPPING_BRANCH_         "ParticleMap"

class FairPrimaryGenerator;
class TBranch;

class ProMCBook;

namespace EicMC {
  class Reader;
}

// erhic::EventMC has a pure virtual member -> have to create 
// inherited class in order to instantiate it; 
class EventProMC: public erhic::EventMC
{
 public:
  EventProMC() {};
  ~EventProMC() {};

  // FIXME: for now can not get these from the ProMC Pythia 8 pp-files;
  const erhic::ParticleMC* BeamLepton()      const { return 0; };
  const erhic::ParticleMC* BeamHadron()      const { return 0; };
  const erhic::ParticleMC* ExchangeBoson()   const { return 0; };
  const erhic::ParticleMC* ScatteredLepton() const { return 0; };

  // There is no parser either (but have to create implementation of this 
  // pure virtual member);
  bool Parse(const std::string&) { assert(0); };

  ClassDef(EventProMC, 1);
}; 

// Same story; THINK: do I really need these complications for EicMC input?;
class EventEicMC: public erhic::EventMC
{
 public:
  EventEicMC() {};
  ~EventEicMC() {};

  // FIXME: add proper calls here, please;
  const erhic::ParticleMC* BeamLepton()      const { return 0; };
  const erhic::ParticleMC* BeamHadron()      const { return 0; };
  const erhic::ParticleMC* ExchangeBoson()   const { return 0; };
  const erhic::ParticleMC* ScatteredLepton() const { return 0; };

  // There is no parser either (but have to create implementation of this 
  // pure virtual member);
  bool Parse(const std::string&) { assert(0); };

  ClassDef(EventEicMC, 1);
};

//
// Want to access protected variables in Forester class, but 
// can not just use it as a base class in EicEventGenerator because
// of ambiguos inheritance of TObject; so create an intermediate 
// class and be happy; actually the functionality matches the name;
//
class Poacher: public erhic::Forester
{
 public:
  Poacher() { ResetProMC(); ResetEicMC(); };
  Poacher(const TString &fileName);

  ~Poacher() {};

  erhic::VirtualEvent *GetNextEvent();

  std::string EventName() const;

 private:
  // Part related to ProMC input;
  ProMCBook *mProMCBook;               //!
  double mMomentumUnits, mLengthUnits; //!
  erhic::ParticleMC *mParticle;        //!
  EventProMC *mEventProMC;             //!

  // Part related to EicMC input;
  EventEicMC *mEventEicMC;             //!
  EicMC::Reader *mReaderEicMC;         //! 

  void ResetProMC() {
    mProMCBook = 0;
    mEventProMC = 0;
    mMomentumUnits = mLengthUnits = 0.0;
  };
  void ResetEicMC() {
    mReaderEicMC = 0;
    mEventEicMC = 0;
  };

  ClassDef(Poacher,3);
};

class ParticleMappingTable: public TObject {
  // Avoid the access complications;
  friend class EicEventGenerator;

 public:
 ParticleMappingTable() {};
  ~ParticleMappingTable() {};

  const std::vector<unsigned> &GetData() const { return mData; };

 private:
  std::vector <unsigned> mData;

  ClassDef(ParticleMappingTable,4);
};

// FIXME: create a separate EicProtoGenerator.h file;
#include <EicBoxGenerator.h>

class EicEventGenerator : public EicProtoGenerator
{
  friend class EicEventGeneratorTask;

  public:
  /** Default constructor without arguments should not be used. **/
  //EicEventGenerator(): FairGenerator() { ResetVars(); };

  /** Standard constructor.
   ** @param fileName The input file name
   **/
  EicEventGenerator(TString fileName = "");
  
  /** Destructor. **/
  virtual ~EicEventGenerator() {};
  
  // Singelton instance get method;
  static EicEventGenerator* Instance() { return mInstance; };

  // Well, may want to use the same input file to be processed in chunks 
  // in a batch mode; FairRunSim class does not seem to provide functionality 
  // to skip events, so implement this feature here;
  int SkipFewEvents(unsigned eventsToSkip);
  void RestrictEventChunkSize(unsigned nEvents) { mEventChunkSize = nEvents; };

  // May want to select only few particle types; can call more than once
  // (say enable only electrons and pi-minus tracks);
  void SelectPdgCode(int pdg)                   { mSelectedPdgCodes.insert(pdg); };
  void SelectLeadingParticle(double pmin = 0.0) { 
    mSelectLeadingParticle       = true; 
    mSelectedLeadingParticlePmin = pmin;
  };

  void BuildEnergyFlowPlot(int etaBinNum, double etaMin, double etaMax) { 
    mEnergyFlowHist    = new TH1D("EnergyFlowPlot",    "Energy Flow Plot",    etaBinNum, etaMin, etaMax);
    mEnergyFlowHist->GetXaxis()->SetTitle("Pseudorapidity");
    mEnergyFlowHist->GetYaxis()->SetTitle("Energy flow per rapidity unit per event, [GeV]");

    mParticleCountHist = new TH1D("ParticleCountPlot", "Particle Count Plot", etaBinNum, etaMin, etaMax);
    mParticleCountHist->GetXaxis()->SetTitle("Pseudorapidity");
    mParticleCountHist->GetYaxis()->SetTitle("Particle count per rapidity unit per event");

    // FIXME: some sanity check needed here;
    mEtaBinWidth = (etaMax - etaMin)/etaBinNum;
  };
  void AddEnergyFlowPdgCode(int pdg)            { mEnergyFlowPdgCodes.insert(pdg); };

  /** Reads on event from the input file and pushes the tracks onto
   ** the stack. Abstract method in base class.
   ** @param primGen  pointer to the FairPrimaryGenerator
   **/
  virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);

  const TChain *GetInputTree() const { return mInputTree; };

 private:
  static EicEventGenerator *mInstance;    //! singelton instance

  Poacher *mPoacher;                      //!
  TChain *mInputTree;                     //!
  unsigned mInputTreeNextEventId;         //!
  erhic::VirtualEvent *mGeneratorEvent;   //!
  
  // Do not want to rely on "vp->GetStatus() == 1" check when passing data 
  // over to the GEANT engine; just record the exact list of particles which 
  // will enter PndMCTrack array and import it back during the reconstruction;
  ParticleMappingTable *mMappingTable;    //!

  Bool_t mSelectLeadingParticle;          // may want to select leading particles only ...
  Double_t mSelectedLeadingParticlePmin;  // ... and with a momentum cut-off
  std::set<Int_t> mSelectedPdgCodes;      // if empty, all particles; otherwise selected only

  // May want to carve a huge input file into several chunks to be processed by different
  // batch jobs; SkipFewEvents() helps to arrange a jump, fine; however, since ReadEvent() may 
  // want to skip several events (because there are no particles left after SelectPdgCode() 
  // call(s)), one needs to restrict max event chunk to consider in order not to grab events 
  // from the "next chunk";
  UInt_t mEventChunkSize;                 // consider at most that number of event candidates
  UInt_t mEventCounter;                   // internal counter which helps to obey mEventChunkSize limit

  double mEtaBinWidth;                    //!
  TH1D *mEnergyFlowHist;                  //!
  TH1D *mParticleCountHist;               //!
  std::set<Int_t> mEnergyFlowPdgCodes;    //!

  void ResetVars() { 
    mPoacher = 0; 
    mGeneratorEvent = 0; 
    mInputTree = 0; mInputTreeNextEventId = 0; mMappingTable = 0; 

    mSelectLeadingParticle = false; mSelectedLeadingParticlePmin = 0.0;
    mEventChunkSize = mEventCounter = 0;

    mEtaBinWidth = 0.0;
    mEnergyFlowHist = mParticleCountHist = 0;
  };
  
  ClassDef(EicEventGenerator,22);
};

class EicEventGeneratorTask: public FairTask {
 public:
 EicEventGeneratorTask(TTree* tree = 0): mOutputTree(tree) {};
  ~EicEventGeneratorTask() {};
  
  // The only method I need here;
  void FinishTask();
  
 private:
  TTree* mOutputTree; //!

  ClassDef(EicEventGeneratorTask,4);
};

#endif
