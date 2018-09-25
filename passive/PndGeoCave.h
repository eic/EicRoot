#ifndef PNDGEOCAVE_H
#define PNDGEOCAVE_H

#include "FairGeoSet.h"
#include "TString.h"
#include "FairGeoMedia.h"
#include <fstream>
class  PndGeoCave : public FairGeoSet {
protected:
  TString name;
public:
  PndGeoCave();
  ~PndGeoCave() {}
  const char* getModuleName(Int_t) {return name.Data();}  
  Bool_t read(std::fstream&,FairGeoMedia*);
  void addRefNodes();
  void write(std::fstream&);
  void print();
  ClassDef(PndGeoCave,0) // Class for the geometry of CAVE
};

#endif  /* !PNDGEOCAVE_H */
