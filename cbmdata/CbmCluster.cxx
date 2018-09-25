/**
 * \file CbmCluster.cxx
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2012
 */

#include "CbmCluster.h"

CbmCluster::CbmCluster()
 : TObject(),
   fDigis(),
   fAddress(0),
   fLinks(NULL)
{

}

CbmCluster::~CbmCluster()
{

}

ClassImp(CbmCluster);
