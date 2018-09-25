/*
 * CbmMCObject.cpp
 *
 *  Created on: Dec 1, 2009
 *      Author: stockman
 */

#include "CbmMCObject.h"

ClassImp(CbmMCObject);

CbmMCObject::CbmMCObject() 
  : TObject(),
    fStage(),
    fStageId()
{
	//std::cout << "-I- CbmMCObject::CbmMCObject : Use of default constructor" << std::endl;
}

CbmMCObject::~CbmMCObject() {
	// TODO Auto-generated destructor stub
}


void CbmMCObject::SetEntry(CbmMCEntry entry){
	if (entry.GetPos() < 0){
		std::cout << "-E- CbmMCObject::SetEntry(CbmMCEntry): pos " << entry.GetPos() << std::endl;
		return;
	}
	AdoptSize(entry.GetPos());
	fStage[entry.GetPos()] = entry;
}

void CbmMCObject::SetEntry(std::vector<Int_t> type, std::vector<Int_t> link, int index){
	CbmMCEntry myEntry;
	myEntry.SetPos(index);
	for (int i = 0; i < type.size(); i++){
		myEntry.AddLink(FairLink(type[i],link[i]));
	}
	SetEntry(myEntry);
}

void CbmMCObject::SetEntry(FairMultiLinkedData* data, int index)
{
	AdoptSize(index);
	fStage[index].SetLinks(data->GetLinks());
}

void CbmMCObject::SetLink(FairLink link, int index)
{
	if (index < fStage.size()){
		fStage[index].Reset();
	}
	AddLink(link, index);
}


void CbmMCObject::AddLink(FairLink link, int index)
{
	AdoptSize(index);
	fStage[index].AddLink(link);
	//std::cout << "AddLink " << index << ": "<< fStageDet[index][fStageDet[index].size()-1] << " " << fStageHit[index][fStageHit[index].size()-1] << std::endl;
}

void CbmMCObject::AdoptSize(int index){
	int start = fStage.size();
	while (fStage.size() < index+1){
		CbmMCEntry myVec;
		myVec.SetPos(fStage.size());
		myVec.SetSource(GetStageId());
		//std::pair<int,int> myPair(fStageId, start);
		//myVec.AddLink(myPair);
		fStage.push_back(myVec);
		start++;
	}
}

FairMultiLinkedData CbmMCObject::PosInList(FairLink link){
	FairMultiLinkedData result;
	for (int i = 0; i < fStage.size(); i++){
		if (fStage[i].IsLinkInList(link.GetType(), link.GetIndex()))
			result.AddLink(FairLink(GetStageId(), i));
	}
	return result;
}
