//*-- AUTHOR : Ilse Koenig
//*-- Created : 10/11/2003

/////////////////////////////////////////////////////////////
// CbmGeoRich
//
// Class for geometry of RICH
//
/////////////////////////////////////////////////////////////


#include "CbmGeoRich.h"

#include "FairGeoNode.h"

ClassImp(CbmGeoRich)

// -----   Default constructor   -------------------------------------------
CbmGeoRich::CbmGeoRich() {
  // Constructor
  fName="rich";
  maxSectors=0;
  maxModules=10;
}
// -------------------------------------------------------------------------

const char* CbmGeoRich::getModuleName(Int_t m) {
  // Returns the module name of rich number m
  sprintf(modName,"rich%i",m+1);
  return modName;
}

const char* CbmGeoRich::getEleName(Int_t m) {
  // Returns the element name of rich number m
  sprintf(eleName,"r%i",m+1);
  return eleName;
}
