//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  Kalman filter task wrapper; 
//

#include "GFRecoHitFactory.h"

#include <FairRunAna.h>
#include <FairGeane.h>

#include <PndRecoKalmanFit.h>

#include <EicTrackingRecoHit.h>
#include <EicRecoKalmanTask.h>

#define _ITERATIONS_DEFAULT_  2
#define _BUSY_CUT_DEFAULT_   50

// ---------------------------------------------------------------------------------------

EicRecoKalmanTask::EicRecoKalmanTask(EicIdealTrackingCode *ideal) 
{
  fIdeal = ideal; 
  fTrackOutBranchName = "EicIdealGenTrack";

  // Geane stuff is needed for track propagation;
  if (FairRunAna::Instance()) FairRunAna::Instance()->AddTask(new FairGeane());

  SetBusyCut(_BUSY_CUT_DEFAULT_);
  SetNumIterations(_ITERATIONS_DEFAULT_);
} // EicRecoKalmanTask::EicRecoKalmanTask()

// ---------------------------------------------------------------------------------------

InitStatus EicRecoKalmanTask::Init() 
{
  // Do it better later; for now deal with the regular Kalman filter only;
  SetDaf(kFALSE);

  // Yes, no need to assign this in reco_complete.C;
  SetTrackInBranchName(fIdeal->fTracksArrayName);

  // Call original PandaRoot Init();
  PndRecoKalmanTask::Init();

  //Get ROOT Manager;
  FairRootManager* ioman= FairRootManager::Instance();
  if(ioman==0)
  {
    Error("EicRecoKalmanFit::Init","RootManager not instantiated!");
    return kERROR;
  }

  // Instead of hardcoding hit classes in PndRecoKalmanTask prefer to spool them from 
  // EicIdealTracker class;
  for (std::vector<EicDetectorGroup>::iterator it=fIdeal->fGroups.begin(); 
       it!=fIdeal->fGroups.end(); it++)
  { 
    //TString flname = it->dname->Name() + "DigiHit";
    TString flname = it->dname->Name() + "TrackingDigiHit";
    TClonesArray* arr=(TClonesArray*) ioman->GetObject(flname);

    if (!it->mGptr) {
      ioman->GetInFile()->GetObject(it->dname->Name() + "GeoParData", it->mGptr); 

      if (!it->mGptr) {
	std::cout << "-E- Eic"<< it->dname->Name() <<" hit producer: no map found!" << std::endl;
	return kERROR;
      } //if

      it->mGptr->InitializeLookupTables();
    } //if

    if(arr!=0)
    {
      //TClass *cl = arr->GetClass();
      //printf("%d\n", cl->InheritsFrom("EicTrackingDigiHit3D"));

      // Use custom EicRawHit class instead of a generic FairHit; and indeed EicRecoHit 
      // (base) class must be generic as well;
      //if (it->dname->Name() == "Tpc")
      // Well, I suppose this check is correct, right?; there are no other digi hit types
      // which require EicSpaceRecoHit-based factory;
      if (arr->GetClass()->InheritsFrom("EicTrackingDigiHit3D"))
	// Yes, let it be a hack for now; figure out how to pass kTPC later;
	fFitter->GetRecoHitFactory()->addProducer(FairRootManager::Instance()->GetBranchId(flname),
						  new GFRecoHitProducer<EicTrackingDigiHit,EicSpaceRecoHit>(arr), it->GetGptr());
      else
	fFitter->GetRecoHitFactory()->addProducer(FairRootManager::Instance()->GetBranchId(flname),
						  new GFRecoHitProducer<EicTrackingDigiHit,EicPlanarRecoHit>(arr), it->GetGptr());

      std::cout << "*** EicRecoKalmanFit::Init" << "\t" << flname << " array  found" << std::endl;
    }
  }
  //exit(0);

  return kSUCCESS;
} // EicRecoKalmanTask::Init()

// ---------------------------------------------------------------------------------------

ClassImp(EicRecoKalmanTask)
