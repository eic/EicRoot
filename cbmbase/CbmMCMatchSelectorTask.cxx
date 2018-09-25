// -------------------------------------------------------------------------
// -----                CbmMCMatchSelectorTask source file             -----
// -----                  Created 18/07/08  by T.Stockmanns        -----
// -------------------------------------------------------------------------
#include "CbmMCMatchSelectorTask.h"

#include "CbmMCMatch.h"

// framework includes
#include "FairRootManager.h"

// libc includes
#include <iostream>
using std::cout;
using std::endl;

// -----   Default constructor   -------------------------------------------
CbmMCMatchSelectorTask::CbmMCMatchSelectorTask()
  : FairTask("Creates CbmMCMatch"), 
    fMCMatch(NULL),
    fStart(kUnknown), 
    fStop(kUnknown),
    fStageWeights(),
    fCommonWeight(0.)
{
}
// -------------------------------------------------------------------------

CbmMCMatchSelectorTask::CbmMCMatchSelectorTask(DataType start, DataType stop)
  : FairTask("Creates CbmMCMatch"), 
    fMCMatch(NULL),
    fStart(start), 
    fStop(stop),
    fStageWeights(),
    fCommonWeight(0.)
{
}

// -----   Destructor   ----------------------------------------------------
CbmMCMatchSelectorTask::~CbmMCMatchSelectorTask()
{
}

// -----   Public method Init   --------------------------------------------
InitStatus CbmMCMatchSelectorTask::Init()
{


//  fMCMatch->InitStage(kMCTrack, "", "MCTrack");



  FairRootManager* ioman = FairRootManager::Instance();
  	if (!ioman) {
  		cout << "-E- CbmMCMatchSelectorTask::Init: "
  				<< "RootManager not instantiated!" << endl;
  		return kFATAL;
  	}

  	fMCMatch = (CbmMCMatch*)ioman->GetObject("MCMatch");

	cout << "-I- CbmMCMatchSelectorTask::Init: Initialization successfull" << endl;


  return kSUCCESS;
}


// -------------------------------------------------------------------------
void CbmMCMatchSelectorTask::SetParContainers()
{
  // Get Base Container
//  FairRun* ana = FairRun::Instance();
//  FairRuntimeDb* rtdb=ana->GetRuntimeDb();

}


// -----   Public method Exec   --------------------------------------------
void CbmMCMatchSelectorTask::Exec(Option_t* opt)
{
	cout << "Output Selector: " << endl;
	SetWeights();
	cout << fMCMatch->GetMCInfo(fStart, fStop);
}

void CbmMCMatchSelectorTask::SetWeights()
{
//	cout << "SetWeights: CommonWeight " << fCommonWeight << " NStageWeights " << fStageWeights.size() << endl;
	fMCMatch->SetCommonWeightStages(fCommonWeight);
	for (int i = 0; i < fStageWeights.size();i++){
		fMCMatch->GetMCStageType(fStageWeights[i].first)->SetWeight(fStageWeights[i].second);
	}
}

void CbmMCMatchSelectorTask::Finish()
{
}


ClassImp(CbmMCMatchSelectorTask);
