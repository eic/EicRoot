//
// AYK (ayk@bnl.gov), 2015/07/15
//
//  A trivial (for now) extension of FairRunAna class;
//

#include <TRandom.h>

#include <FairRuntimeDb.h>
#include <FairParRootFileIo.h>

#include <EicRunAna.h>

EicRunAna* EicRunAna::mInstance = 0;

// ---------------------------------------------------------------------------------------

EicRunAna::EicRunAna(): mSeed(_SEED_DEFAULT_), mInitCallHappened(false) 
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
  gRandom->SetSeed(mSeed); 
 
  // Well, may want to use either {Init(), then Run()} or just {Run()}; avoid 
  // calling Init() twice;
  if (!mInitCallHappened) Init();

  FairRunAna::Run(NStart, NStop);

  // Yes, and get rid of this file as well; 
  if (!access(_GPHYSI_DAT_, W_OK)) unlink(_GPHYSI_DAT_);

  // Yes, just exit; change the default behaviour if this ever becomes a problem;
  exit(0);
} // EicRunAna::Run()

// ---------------------------------------------------------------------------------------

ClassImp(EicRunAna)
