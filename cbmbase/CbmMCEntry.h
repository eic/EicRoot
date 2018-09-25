/*
 * CbmMCEntry.h
 *
 *  Created on: Dec 22, 2009
 *      Author: stockman
 */

#ifndef CBMMCENTRY_H_
#define CBMMCENTRY_H_

//#include "CbmMCList.h"
//#include "CbmDetectorList.h"
//#include "FairLink.h"
#include "FairMultiLinkedData.h"

//#include <utility>
#include <iostream>

class FairLink;

class CbmMCEntry : public FairMultiLinkedData {
public:
	CbmMCEntry();
	//CbmMCEntry(DataType type, int pos);
	CbmMCEntry(std::set<FairLink> links, Int_t source = -1, Int_t pos = -1)
		:FairMultiLinkedData(links), fSource(source), fPos(pos){}
	CbmMCEntry(FairMultiLinkedData links, Int_t source = -1, Int_t pos = -1)
		:FairMultiLinkedData(links), fSource(source), fPos(pos){}

//	CbmMCEntry(std::vector<std::pair<int, int> > links, Int_t source = -1, Int_t pos = -1)
//			:FairMultiLinkedData(links), fSource(source), fPos(pos){}

	void SetSource(Int_t source){fSource = source;}
	void SetPos(Int_t pos){fPos = pos;}

	Int_t GetSource() const {return fSource;}
	Int_t GetPos() const {return fPos;}

	virtual ~CbmMCEntry();

	virtual void Print(std::ostream& out){
		out << *this;
	}

	friend std::ostream& operator<< (std::ostream& out, const CbmMCEntry& link){
		//out << "Source: " << link.GetSource() << " Position: " << link.GetPos() << std::endl;
		((FairMultiLinkedData)link).Print(out);
		return out;
	}


private:
	Int_t fSource;
	Int_t fPos;

	ClassDef(CbmMCEntry, 1);
};

#endif
