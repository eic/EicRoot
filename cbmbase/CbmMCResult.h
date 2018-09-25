/*
 * CbmMCResult.h
 *
 *  Created on: Dec 1, 2009
 *      Author: stockman
 */

#ifndef CBMMCRESULT_H_
#define CBMMCRESULT_H_

#include "CbmDetectorList.h"
#include "CbmMCObject.h"

#include<iostream>

class CbmMCResult: public CbmMCObject {
public:
	CbmMCResult();
	CbmMCResult(DataType start, DataType stop);
	CbmMCResult(const CbmMCResult& result):
		CbmMCObject(result.GetStartType(), result.GetEntryVector()),
		fStartType(result.GetStartType()),
		fStopType(result.GetStopType())
		{}

	virtual ~CbmMCResult();

	void SetStartType(DataType start){fStartType = start;}
	void SetStopType(DataType stop){fStopType = stop;}

	DataType GetStartType(void) const {return fStartType;}
	DataType GetStopType(void) const {return fStopType;}

	CbmMCResult& operator=(const CbmMCResult& result){
	  CbmMCObject::operator=(result);
	  fStartType = result.fStartType;
	  fStopType = result.fStopType;
	  return *this;
	  //		SetStage(result.GetEntryVector());
	}

	virtual void Print(std::ostream& out = std::cout){ out << *this;}

	friend std::ostream& operator<< (std::ostream& out, const CbmMCResult& result){
		out << "MC Link from: " << result.GetStartType() << " to "  << result.GetStopType() << ":" << std::endl;
		((CbmMCObject)result).Print(out);
		return out;
	}


private:
	DataType fStartType;
	DataType fStopType;

	ClassDef(CbmMCResult, 2);
};

#endif /* CbmMCResult_H_ */
