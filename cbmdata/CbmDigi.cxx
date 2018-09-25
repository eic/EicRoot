/** @file CbmDigi.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 30.04.2013
 **/

#include "CbmDigi.h"

#include "FairMultiLinkedData.h"

#include <memory>

// -----   Default constructor   -------------------------------------------
CbmDigi::CbmDigi() 
 : TObject(), 
   fLinks(NULL) 
{
}
// -------------------------------------------------------------------------

CbmDigi::CbmDigi(const CbmDigi& rhs) 
 : TObject(rhs), 
   fLinks(NULL) 
{
  if (NULL != rhs.fLinks) {
     fLinks = new FairMultiLinkedData(*(rhs.fLinks));
   }
}

CbmDigi& CbmDigi::operator=(const CbmDigi& rhs) 
{

  if (this != &rhs) {
    TObject::operator=(rhs);
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





ClassImp(CbmDigi)
