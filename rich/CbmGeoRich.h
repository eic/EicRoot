#ifndef CBMGEORICH_H
#define CBMGEORICH_H

#include "FairGeoSet.h"

class  CbmGeoRich : public FairGeoSet {
protected:
  char modName[20];  // name of module
  char eleName[20];  // substring for elements in module
public:
  CbmGeoRich();
  ~CbmGeoRich() {}
  const char* getModuleName(Int_t);
  const char* getEleName(Int_t);
  inline Int_t getModNumInMod(const TString&);
  ClassDef(CbmGeoRich,0) // Class for Rich
};

inline Int_t CbmGeoRich::getModNumInMod(const TString& name) {
  // returns the module index from module name
  return (Int_t)(name[4]-'0')-1;
}

#endif  /* !CBMGEORICH_H */



