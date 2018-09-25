/**
 * \file CbmBaseHit.cxx
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 **/
#include "CbmBaseHit.h"

#include "FairMultiLinkedData.h"

#include <memory>

CbmBaseHit::CbmBaseHit():
    TObject(),
	fType(kHIT),
	fAddress(-1),
	fZ(0.),
	fDz(0.),
	fRefId(-1),
	fLinks(NULL)
{
}

CbmBaseHit::CbmBaseHit(const CbmBaseHit& rhs)
 : TObject(rhs),
   fType(rhs.fType),
   fZ(rhs.fZ),
   fDz(rhs.fDz),
   fRefId(rhs.fRefId),
   fAddress(rhs.fAddress),
   fLinks(NULL)
{
   if (NULL != rhs.fLinks) {
     fLinks = new FairMultiLinkedData(*(rhs.fLinks));
   }
}

CbmBaseHit& CbmBaseHit::operator=(const CbmBaseHit& rhs)
{

  if (this != &rhs) {

    TObject::operator=(rhs);
    fType = rhs.fType;
    fZ = rhs.fZ;
    fDz = rhs.fDz;
    fRefId = rhs.fRefId;
    fAddress = rhs.fAddress;

    if (NULL != rhs.fLinks) {
      std::auto_ptr<FairMultiLinkedData> tmp(new FairMultiLinkedData(*rhs.fLinks));
      delete fLinks;
      fLinks = tmp.release();
    } else {
      fLinks = NULL;
    }
  }
  return *this;
}


CbmBaseHit::~CbmBaseHit()
{
}

ClassImp(CbmBaseHit);
