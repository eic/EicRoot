#ifndef PNDMCCLONER_H
#define PNDMCCLONER_H 1


#include "FairTask.h"
#include "PndMCTrack.h"

class TClonesArray;

class PndMcCloner : public FairTask
{

 public:

  /** Default constructor **/  
  PndMcCloner();


  /** Destructor **/
  ~PndMcCloner();


  /** Virtual method Init **/
  virtual InitStatus Init();


  /** Virtual method Exec **/
  virtual void Exec(Option_t* opt);

  //  PndHit* AddHit(Int_t detID, TVector3& pos, TVector3& dpos, Int_t index);

 private: 
  
  /** Input array of PndMCTrack **/
  TClonesArray* fInputArray;

  /** Output array of PndMCTrack **/
  TClonesArray* fOutputArray;  

   
  ClassDef(PndMcCloner,1);

};

#endif
