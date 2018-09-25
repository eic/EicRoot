/*
 * CbmMCList.h
 *
 *  Created on: Dec 3, 2009
 *      Author: stockman
 */

#ifndef CBMMCLIST_H_
#define CBMMCLIST_H_

#include "TObject.h"
#include "CbmDetectorList.h"

#include <vector>



class CbmMCList : public TObject {
public:
	CbmMCList();
	CbmMCList(DataType type, Int_t entry) 
	  : TObject(),
	  fList(),
	  fEntry(entry),
	  fType(type)
	{
	  //		fType = type;
	  //fEntry = entry;
	}
	CbmMCList(DataType type, Int_t entry, std::vector<Int_t> list)
	  : TObject(),
	  fList(),
	  fEntry(entry),
	  fType(type)
	{
	  //		fType = type;
	  //	fEntry = entry;
	  //	fList = list;
	}

	virtual ~CbmMCList();

	void SetType(DataType type){ fType = type;}
	void SetEntry(Int_t entry){ fEntry = entry;}
	void AddElement(Int_t element){fList.push_back(element);}

	DataType GetType() const {return fType;}
	Int_t GetEntry() const {return fEntry;}
	Int_t GetNElements() const {return fList.size();}
	Int_t GetElement(Int_t index)const {return fList.at(index);}
	std::vector<Int_t> GetElements() const {return fList;}

	void Reset(){fList.clear();}




private:
	std::vector<Int_t> fList;
	Int_t fEntry;
	DataType fType;

	ClassDef(CbmMCList, 1);
};

#endif /* PNDMCLIST_H_ */
