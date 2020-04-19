//
// AYK (ayk@bnl.gov), 2015/07/15
//
//  A trivial (for now) extension of FairRunAna class;
//

#include <TRandom.h>

#include <FairRuntimeDb.h>
#include <FairParRootFileIo.h>

#include <FairEventHeader.h>
#include <FairMCEventHeader.h>
#include <FairTrajFilter.h>

#include <EicRunAna.h>

EicRunAna* EicRunAna::mInstance = 0;

// ---------------------------------------------------------------------------------------

EicRunAna::EicRunAna(): mSeed(_SEED_DEFAULT_), mInitCallHappened(false), mEvStart(0), mEvEnd(0)
			//, mEvCurrent(-1)
{
  if (mInstance) {    
    Fatal("EicRunAna::EicRunAna()", "Singleton instance already exists.");
    return;
  } //if

  mInstance = this;
} // EicRunAna::EicRunAna()

// ---------------------------------------------------------------------------------------

void EicRunAna::SetInputFile(TString fname)
{
  mInputFileName = fname;

  FairRunAna::SetInputFile(fname);
} // EicRunAna::SetInputFile()

// ---------------------------------------------------------------------------------------

void EicRunAna::Init()
{
  FairRuntimeDb* rtdb = GetRuntimeDb();
  FairParRootFileIo* parInput1 = new FairParRootFileIo();
  parInput1->open(mInputFileName);
        
  rtdb->setFirstInput(parInput1);

  FairRunAna::Init();

  mInitCallHappened = true;
} // EicRunAna::Init()

// ---------------------------------------------------------------------------------------

void EicRunAna::Run(Int_t NStart, Int_t NStop)
{
  // This is indeed a clear hack;
  if (mJanaPluginMode) return;

  //gRandom->SetSeed(mSeed); 
 
  // Well, may want to use either {Init(), then Run()} or just {Run()}; avoid 
  // calling Init() twice;
  //if (!mInitCallHappened) Init();

  //FairRunAna::Run(NStart, NStop);
  RunCoreStart(NStart, NStop);
  while (RunCoreImportNextEvent()) RunCoreProcessNextEvent();
  RunCoreFinish();

  // Yes, and get rid of this file as well; 
  //if (!access(_GPHYSI_DAT_, W_OK)) unlink(_GPHYSI_DAT_);

  // Yes, just exit; change the default behaviour if this ever becomes a problem;
  // NB: can reach this point in a "standalone" mode only (not as a plugin);
  // so terminate and exit ROOT session, for user convenience;
  exit(0);
} // EicRunAna::Run()

// ---------------------------------------------------------------------------------------

void EicRunAna::RunCoreStart(Int_t Ev_start, Int_t Ev_end)
{
  gRandom->SetSeed(mSeed); 
 
  // Well, may want to use either {Init(), then Run()} or just {Run()}; avoid 
  // calling Init() twice;
  if (!mInitCallHappened) Init();

  assert(!fProofAnalysis && !fTimeStamps && !fMixedInput &&fInFileIsOpen);

  //if ( fProofAnalysis ) {
  //RunOnProof(Ev_start,Ev_end);
  //return;
  //}

  //if (fTimeStamps) {
  //RunTSBuffers();
  //} else if (fMixedInput) {
  //RunMixed(Ev_start,Ev_end);
  //} else {
  {
    //UInt_t tmpId =0;
    //  if (fInputFile==0) {
    //if (!fInFileIsOpen) {
    //DummyRun(Ev_start,Ev_end);
    //return;
    //}
    if (Ev_end==0) {
      if (Ev_start==0) {
        Ev_end=Int_t((fRootManager->GetInChain())->GetEntries());
      } else {
        Ev_end =  Ev_start;
        if ( Ev_end > ((fRootManager->GetInChain())->GetEntries()) ) {
          Ev_end = (Int_t) (fRootManager->GetInChain())->GetEntries();
        }
        Ev_start=0;
      }
    } else {
      Int_t fileEnd=(fRootManager->GetInChain())->GetEntries();
      if (Ev_end > fileEnd) {
        //cout << "-------------------Warning---------------------------" << endl;
        //cout << " -W FairRunAna : File has less events than requested!!" << endl;
        //cout << " File contains : " << fileEnd  << " Events" << endl;
        //cout << " Requested number of events = " <<  Ev_end <<  " Events"<< endl;
        //cout << " The number of events is set to " << fileEnd << " Events"<< endl;
        //cout << "-----------------------------------------------------" << endl;
        Ev_end = fileEnd;
      }
    }

    // Assign class instance variables;
    mEvStart = Ev_start; mEvEnd = Ev_end; mEvCurrent = mEvStart-1;

    fRunInfo.Reset();
  }
} // EicRunAna::RunCoreStart()

bool EicRunAna::RunCoreImportNextEvent( void )
{
  if (++mEvCurrent == mEvEnd) return false;

  //for (int i=mEvStart; i< mEvEnd; i++) {
  fRootManager->ReadEvent(mEvCurrent);
  return true;
} // EicRunAna::RunCoreImportNextEvent()

void EicRunAna::RunCoreProcessNextEvent( void )
{
  {
    UInt_t tmpId =0;

    /**
     * if we have simulation files then they have MC Event Header and the Run Id is in it, any way it
     * would be better to make FairMCEventHeader a subclass of FairEvtHeader.
     */
    if (fRootManager->IsEvtHeaderNew()) {
      tmpId = fMCHeader->GetRunID();
    } else {
      tmpId = fEvtHeader->GetRunId();
    }
    if ( tmpId != fRunId ) {
      fRunId = tmpId;
      if ( !fStatic ) {
	Reinit( fRunId );
	fTask->ReInitTask();
      }
    }
      //FairMCEventHeader* header = (FairMCEventHeader*)fRootManager->GetObject("MCEventHeader.");
      //std::cout << "WriteoutBufferData with time: " << fRootManager->GetEventTime();
      fRootManager->StoreWriteoutBufferData(fRootManager->GetEventTime());
      fTask->ExecuteTask("");
      fRootManager->Fill();
      fRootManager->DeleteOldWriteoutBufferData();
      fTask->FinishEvent();

      fRunInfo.StoreInfo();
      if (NULL !=  FairTrajFilter::Instance()) {
        FairTrajFilter::Instance()->Reset();
      }
      //}
  }

  //mEvCurrent++;
  //return true;
} // EicRunAna::RunCoreProcessNextEvent()

void EicRunAna::RunCoreFinish( void )
{
  {
    fRootManager->StoreAllWriteoutBufferData();
    fTask->FinishTask();
    if (fWriteRunInfo) {
      fRunInfo.WriteInfo();
    }
    fRootManager->LastFill();
    fRootManager->Write();
  }

  // Yes, and get rid of this file as well; 
  if (!access(_GPHYSI_DAT_, W_OK)) unlink(_GPHYSI_DAT_);
}

// ---------------------------------------------------------------------------------------

ClassImp(EicRunAna)
