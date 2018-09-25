/*
 * CbmMCResult.cpp
 *
 *  Created on: Dec 1, 2009
 *      Author: stockman
 */

#include "CbmMCResult.h"

ClassImp(CbmMCResult);

CbmMCResult::CbmMCResult() 
  : CbmMCObject(),
    fStartType(kUnknown),
    fStopType(kUnknown)
{
}

CbmMCResult::~CbmMCResult() {
}

CbmMCResult::CbmMCResult(DataType start, DataType stop)
  : CbmMCObject(),
    fStartType(start), 
    fStopType(stop)
{
}

