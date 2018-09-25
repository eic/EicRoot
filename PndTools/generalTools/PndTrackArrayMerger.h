
#include "FairTask.h"

#include "TString.h"
#include "TClonesArray.h"
#include <vector>

class PndTrackArrayMerger : public FairTask
{

public:
  PndTrackArrayMerger();
  PndTrackArrayMerger(TString s);
  virtual ~PndTrackArrayMerger();

  virtual void SetParContainers();
  virtual InitStatus Init();
  virtual InitStatus ReInit();  
  virtual void Exec(Option_t* opt);
  virtual void FinishEvent();

  void SetPersistance(Bool_t p=kTRUE) {fPersistance = p;}
  void AddInputBranch(TString s) {fInputBranchList.push_back(s);}
  void SetOutputBranch(TString s) {fOutputBranch = s;}

private:
  Bool_t fPersistance; //! Flag if to store 
  TString fOutputBranch;
  TClonesArray* fOutputArray;
  std::vector<TString> fInputBranchList;
  std::vector<TClonesArray*> fInputArrayList;

  PndTrackArrayMerger(const  PndTrackArrayMerger& L);
  PndTrackArrayMerger& operator= (const  PndTrackArrayMerger&) {return *this;}
  
public:
  ClassDef(PndTrackArrayMerger,1);

};
