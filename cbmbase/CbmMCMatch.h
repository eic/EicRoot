/*
 * CbmMCMatch.h
 *
 *  Created on: Nov 23, 2009
 *      Author: stockman
 */

#ifndef CBMMCMATCH_H_
#define CBMMCMATCH_H_


#include "CbmDetectorList.h"
#include "CbmMCStage.h"
#include "CbmMCResult.h"
#include "CbmMCEntry.h"

#include "FairLink.h"
#include "FairMultiLinkedData.h"

#include "TNamed.h"

#include <map>

class FairMultiLinkedData;

typedef std::map<DataType, CbmMCStage*>::iterator TListIterator;
typedef std::map<DataType, CbmMCStage*>::const_iterator TListIteratorConst;

class CbmMCMatch: public TNamed {
public:
	CbmMCMatch();
	CbmMCMatch(const char* name, const char* title)
	  :TNamed(name, title), fUltimateStage(kMCTrack),
	  fList(), fFinalStageML() {};
	virtual ~CbmMCMatch();

	void AddElement(DataType type, int index, FairLink link);
	void AddElement(DataType sourceType, int index, DataType targetType, int link);
	void SetElements(DataType sourceType, int index, FairMultiLinkedData* links);
	void InitStage(DataType type, std::string fileName, std::string branchName);
	void RemoveStage(DataType type);
	void LoadInMCLists(TClonesArray* myLinkArray);
	void ClearMCList();

	void SetCommonWeightStages(Float_t weight);

	CbmMCEntry GetEntry(DataType type, int index);
	CbmMCEntry GetEntry(FairLink link);

	CbmMCResult GetMCInfo(DataType start, DataType stop);
	CbmMCEntry  GetMCInfoSingle(FairLink aLink, DataType stop);

	int GetNMCStages() const {return fList.size();}

	CbmMCStage* GetMCStage(int index) const{
		TListIteratorConst iter = fList.begin();
		for (int i = 0; i < index; i++)
			iter++;
		return (iter->second);
	}

	CbmMCStage* GetMCStageType(DataType type){
		return fList[type];
	}

	void CreateArtificialStage(DataType stage, std::string fileName = "", std::string branchName = "");

	FairMultiLinkedData FindLinksToStage(DataType stage);

	bool IsTypeInList(DataType type);

	void Print(std::ostream& out = std::cout){out << *this;}

	friend std::ostream& operator<< (std::ostream& out, const CbmMCMatch& match){
		for (int i = 0; i < match.GetNMCStages(); i++){
			if (match.GetMCStage(i)->GetLoaded() == kTRUE){
				match.GetMCStage(i)->Print(out);
				out << std::endl;
			}
		}
		return out;
	}

private:
	DataType fUltimateStage; ///< last stage in link chain. Here all recursive operations must stop.
	std::map<DataType, CbmMCStage*> fList;
	FairMultiLinkedData fFinalStageML;

	void FindStagesPointingToLinks(FairMultiLinkedData links, DataType stop);
	FairMultiLinkedData FindStagesPointingToLink(FairLink link);

	CbmMCResult GetMCInfoForward(DataType start, DataType stop);
	CbmMCResult GetMCInfoBackward(DataType start, DataType stop);
	CbmMCEntry GetMCInfoForwardSingle(FairLink link, DataType stop);
	CbmMCEntry GetMCInfoBackwardSingle(FairLink link, DataType stop, Double_t weight = 1.);

	void GetNextStage(FairMultiLinkedData& startEntry, DataType stopStage);
	void AddToFinalStage(FairLink link, Float_t mult);
	void ClearFinalStage();
	ClassDef(CbmMCMatch, 1);
};

#endif /* PNDMCMATCH_H_ */
