// -------------------------------------------------------------------------
// -----                PndMcCloner source file                     -----
// -----                  Created 08/07/13  by  S.Spataro              -----
// -------------------------------------------------------------------------

#include "PndMcCloner.h"

#include "PndMCTrack.h"

#include "FairRootManager.h"
#include "FairDetector.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

#include "TClonesArray.h"

#include <iostream>

using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
PndMcCloner::PndMcCloner() : FairTask("Cloner of PndMCTrack") {}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
PndMcCloner::~PndMcCloner() { }
// -------------------------------------------------------------------------

// -----   Public method Init   --------------------------------------------
InitStatus PndMcCloner::Init() {
  
  cout << "-I- PndMcCloner::Init: "
       << "INITIALIZATION *********************" << endl;
  
  FairRun* sim = FairRun::Instance();
  FairRuntimeDb* rtdb=sim->GetRuntimeDb();
    
  // Get RootManager
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman ) {
    cout << "-E- PndMcCloner::Init: "
	 << "RootManager not instantiated!" << endl;
    return kFATAL;
  }
  
  // Get input array
  fInputArray = (TClonesArray*) ioman->GetObject("MCTrack");
  if ( ! fInputArray ) {
    cout << "-W- PndMcCloner::Init: "
	 << "No MCTrack array!" << endl;
    return kERROR;
  }

  // Create and register output array
  fOutputArray = new TClonesArray("PndMCTrack");
  
  ioman->Register("MCTrack","MC",fOutputArray,kTRUE);
 
  cout << "-I- PndMcCloner: Intialization successfull" << endl;
  
  return kSUCCESS;

}
// -------------------------------------------------------------------------



// -----   Public method Exec   --------------------------------------------
void PndMcCloner::Exec(Option_t* opt) {
  
  // Reset output array
  if ( ! fOutputArray ) Fatal("Exec", "No Output Array");
  
  fOutputArray->Clear();
  
  Int_t nMcTracks = fInputArray->GetEntriesFast();
  for (Int_t iMc=0; iMc<nMcTracks; iMc++) 
    {
      PndMCTrack *mctrack  = (PndMCTrack*) fInputArray->At(iMc);
      TClonesArray& clref = *fOutputArray;
      Int_t size = clref.GetEntriesFast();
      new(clref[size]) PndMCTrack(*mctrack);
      
    } // Loop over MCTracks
  
}
// -------------------------------------------------------------------------

/*
// -----   Private method AddHit   --------------------------------------------
PndHit* PndMcCloner::AddHit(Int_t detID, TVector3& pos, TVector3& dpos, Int_t index){
  // It fills the PndHit category
 
  TClonesArray& clref = *fHitArray;
  Int_t size = clref.GetEntriesFast();
  return new(clref[size]) PndHit(detID, pos, dpos, index);
}
// ----

*/
ClassImp(PndMcCloner)
