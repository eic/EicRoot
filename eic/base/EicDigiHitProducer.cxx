//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC digitized hit producer class; 
//

#include <assert.h>
#include <iostream>
#include <cmath>

#include "TGeoManager.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TRandom.h"

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairGeoNode.h"
#include "FairGeoTransform.h"
#include "FairGeoRotation.h"
#include "FairGeoVector.h"

#include <PndGeoHandling.h>

#include "EicDigiHitProducer.h"

using namespace std;

// ---------------------------------------------------------------------------------------

EicDigiHitProducer::EicDigiHitProducer(const char *name):
  mEventCounter(0), mPersistence(kTRUE), mDigiHitArray(0)
{ 
  ResetVars();

  mDetName = new EicDetName(name);

  // Yes, after EicDetName(name); or could it yet be placed in the header section?;
  FairTask("EIC " + mDetName->NAME() + " Hit Producer");
} // EicDigiHitProducer::EicDigiHitProducer()

// ---------------------------------------------------------------------------------------

InitStatus EicDigiHitProducer::Init() 
{
  std::cout<<"#########################################################"<<std::endl;
  std::cout<<mDetName->Name()<<"DigiHitProducer: Init()#######"<<std::endl;
  std::cout<<"#########################################################"<<std::endl;

  // Get RootManager;
  FairRootManager* ioman = FairRootManager::Instance();
  if ( ! ioman ) {
    cout << "-E- "<<mDetName->Name()<<"DigiHitProducer::Init: "
	 << "RootManager not instantiated!" << endl;
    return kFATAL;
  }
  
  // Get input Monte-Carlo point array;
  mMoCaPointArray = (TClonesArray*) ioman->GetObject(mDetName->Name() + "MoCaPoint");
  if ( ! mMoCaPointArray ) {
    cout << "-W- "<<mDetName->Name()<<"DigiHitProducer::Init: "
	 << "No "<<mDetName->Name()<<"Point array!" << endl;
    return kERROR;
  } //if

  {
    ioman->GetInFile()->GetObject(mDetName->Name() + "GeoParData", mGptr);

    // Well, missing mapping table is a critical failure;
    if (!mGptr) {
      std::cout << "-E- Eic"<< mDetName->Name() <<" hit producer: no map found!" << std::endl;
      return kERROR;
    } //if

    mGptr->InitializeLookupTables();

    // Loop through all the maps and set sensitive flags for those, whose 
    // top-level volume names match the requested ones;
    for(int iq=0; iq<mGptr->GetMapNum(); iq++) {
      EicGeoMap *fmap = mGptr->GetMapPtrViaMapID(iq);

      const std::pair<TString, double> *entry = 
	/*mDigi->*/mSensitiveVolumes.AnyMatch(*fmap->GetInnermostVolumeName());
      
      // Well, I guess this makes sense: make all maps sensitive unless a fraction
      // of them activated manually; FIXME: handling of Birk's correction constant here 
      // is indeed a crappy coding style;
      if (entry || mSensitiveVolumes.IsEmpty()) fmap->SetSensitivityFlag(entry ? entry->second : 0.0);
    } //for iq
  }

  // At the very least these routines are supposed to allocate and register 'mDigiHitArray';
  if (ExtraInit() != kSUCCESS) return kERROR;
  //if (mDigiHitArray)
  //ioman->Register(mDetName->Name() + "DigiHit", mDetName->NAME(), mDigiHitArray, mPersistence);

  cout << "-I- "<<mDetName->Name()<<"DigiHitProducer: INITIALIZATION SUCCESSFUL" << endl;

  return kSUCCESS;
} // EicDigiHitProducer::Init()

// ---------------------------------------------------------------------------------------

void EicDigiHitProducer::SetParContainers() 
{

  FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();
  //fFtsParameters = (PndGeoFtsPar*) rtdb->getContainer("PndGeoFtsPar");
  //fFtsParameters = (PndGeoFtsPar*) rtdb->getContainer("TrsGeoPar");
} // EicDigiHitProducer::SetParContainers() 

// ---------------------------------------------------------------------------------------

void EicDigiHitProducer::Exec(Option_t* opt) 
{
  //printf("Here!\n");
  if ((fVerbose && mEventCounter%50 == 0) || fVerbose >= 3) 
    cout << "Event Number " << mEventCounter << endl;

  mEventCounter++;
  
  // Reset output array (if declared at all);
  if ( !mDigiHitArray ) return;
  
  mDigiHitArray->Clear();
 
  // Possible detector-specific event-level initialization;
  PreExec();

  // Loop over MC points;
  Int_t nPoints = mMoCaPointArray->GetEntriesFast();
  //printf("%d\n", nPoints);

  for (Int_t iPoint = 0; iPoint < nPoints; iPoint++) {
    EicMoCaPoint* point = (EicMoCaPoint*) mMoCaPointArray->At(iPoint);
    if (point == NULL) continue;

    // Save back-door variable;
    point->SetPointID(iPoint);

    HandleHit(point);
  } //for iPoint

  // Possible detector-specific event-level post-handling;
  PostExec();

  // Event summary; THINK: can be misleading for inherited classes?;
  printf("-I- %sDigiHitProducer (ev#%5d): %6d point(s); %4d hit(s) created\n",
	 mDetName->Name().Data(), mEventCounter, nPoints, mDigiHitArray->GetEntriesFast());
} // EicDigiHitProducer::Exec()

// ---------------------------------------------------------------------------------------

ClassImp(EicDigiHitProducer)
