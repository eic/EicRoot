/*
 * CbmMCStage.h
 *
 *  Created on: Dec 1, 2009
 *      Author: stockman
 */

#ifndef CBMMCSTAGE_H_
#define CBMMCSTAGE_H_

#include "CbmDetectorList.h"
#include "CbmMCObject.h"

#include<iostream>

class CbmMCStage: public CbmMCObject {
public:
	CbmMCStage();
	CbmMCStage(DataType id, std::string fileName, std::string branchName, Double_t weight = 1.0);
	CbmMCStage(const CbmMCStage& mcStage):
		CbmMCObject((CbmMCObject)mcStage),
		fBranchName(mcStage.GetBranchName()),
		fFileName(mcStage.GetFileName()),
		fWeight(mcStage.GetWeight()),
		fLoaded(mcStage.GetLoaded()),
		fFill(mcStage.GetFill())
	{}

	virtual ~CbmMCStage();

	void SetBranchName(std::string branchName)	{ fBranchName = branchName; }
	void SetFileName(std::string fileName)			{ fFileName = fileName; }
	void SetWeight(Double_t weight)				{ fWeight = weight; }
	void SetLoaded(Bool_t loaded)					{ fLoaded = loaded; }
	void SetFill(Bool_t fill)						{ fFill = fill; }

	std::string	GetBranchName(void) 	const {return fBranchName;}
	std::string GetFileName(void) 		const {return fFileName;}
	Double_t 	GetWeight(void) 		const {return fWeight;}
	Bool_t 		GetLoaded(void)		const {return fLoaded;}
	Bool_t		GetFill(void)			const {return fFill;}

	CbmMCStage& operator=(const CbmMCStage& result){
		CbmMCObject::operator=(result);
		fBranchName = result.GetBranchName();
		fFileName = result.GetFileName();
		fWeight = result.GetWeight();
		fLoaded = result.fLoaded;
                fFill = result.fFill;
                return *this;
		//		SetStage(result.GetEntryVector());
	}

	virtual void ClearEntries()
	{
		CbmMCObject::ClearEntries();
		fLoaded = kFALSE;
	}

	virtual void Print(std::ostream& out){out << *this;}

	friend std::ostream& operator<< (std::ostream& out, const CbmMCStage& stage){
		out << stage.GetStageId() << ": " << stage.GetBranchName() << " // " <<  stage.GetFileName() << std::endl; //" with weight: " << stage.GetWeight() << std::endl;
		((CbmMCObject)stage).Print(out);
		return out;
	}

private:
	std::string fBranchName;
	std::string fFileName;
	Double_t fWeight;
	Bool_t fLoaded;	///< indicates if this stage was loaded already from a Link file
	Bool_t fFill; ///< indicates if a corresponding DataFile with FairLinks exists to fill this stage


	ClassDef(CbmMCStage, 1);
};

#endif /* PNDMCSTAGE_H_ */
