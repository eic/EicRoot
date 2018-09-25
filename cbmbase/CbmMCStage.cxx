/*
 * CbmMCStage.cpp
 *
 *  Created on: Dec 1, 2009
 *      Author: stockman
 */

#include "CbmMCStage.h"

ClassImp(CbmMCStage);

CbmMCStage::CbmMCStage()
  : CbmMCObject(),
    fBranchName(),
    fFileName(),
    fWeight(1.0), 
    fLoaded(kFALSE), 
    fFill(kFALSE) 
{
}

CbmMCStage::~CbmMCStage() {
}

CbmMCStage::CbmMCStage(DataType id, std::string fileName, std::string branchName, Double_t weight)
  : CbmMCObject(id), 
    fBranchName(branchName), 
    fFileName(fileName), 
    fWeight(weight), 
    fLoaded(kFALSE), 
    fFill(kFALSE)
{
}

