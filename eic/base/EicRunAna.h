//
// AYK (ayk@bnl.gov), 2015/07/15
//
//  A trivial (for now) extension of FairRunAna class;
//

#include <TString.h>

#include <FairRunAna.h>

#include <EicRunSim.h>

#ifndef _EIC_RUN_ANA_
#define _EIC_RUN_ANA_

class EicRunAna : public FairRunAna
{
 public:
  EicRunAna();
  ~EicRunAna() {};

  // Singelton instance get method;
  static EicRunAna* Instance() { return mInstance; };

  void SetSeed(unsigned seed) { mSeed = seed; };

  void SetInputFile(TString fname);
  const TString &GetInputFileName() const { return mInputFileName; };

  int AddFriend(const char *fName) {
    if (!fName) return -1;

    TString strName(fName);

    mFriendFiles.push_back(strName);

    FairRunAna::AddFriend(fName);

    return 0;
  };

  void Init();
  void Run(Int_t NStart = 0, Int_t NStop = 0);

  const std::vector<TString> &GetFriendFiles() { return mFriendFiles; };

 private:
  // THINK: can probably live with dynamic cast to respective FairRunSim method?;
  static EicRunAna *mInstance;      //! singelton instance

  // It is not that easy to pull this name from FairRunAna -> just store 
  // this local copy and intercept SetInputFile() call;
  TString mInputFileName;            // input file name

  Bool_t mInitCallHappened;          //!

  // Well, there is some duplication here and in few other places between
  // EicRunSim and EicRunAna; but I have to inherit from FairRunSim and 
  // FairRunAna (so after the split) and do not want to add another base
  // class because of TObject inheritance (really a problem?);
  UInt_t mSeed;                      // random seed; default: 0x12345678

  // Want to extract digitization info from all friend files in HTC pass;
  // so store the file names and intercept AddFriend() call;
  std::vector<TString> mFriendFiles; // friend file names

  ClassDef(EicRunAna, 5)
};

#endif
