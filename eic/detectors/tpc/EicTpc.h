//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  EIC TPC detector definition wrapper 
//

#ifndef _EIC_TPC_
#define _EIC_TPC_

#include <EicDetector.h>

class EicTpc : public EicDetector {
public:	
  EicTpc() {}; 
 EicTpc(const char *geometryName, Bool_t Active = kTRUE): 
  // NB: want hits for every step;
  EicDetector("TPC", geometryName, qTPC, qOneStepOneHit, Active) {};
  //EicDetector("TPC", geometryName, kTRUE, qTPC, Active) {};

  virtual ~EicTpc() {};
  
  ClassDef(EicTpc,4)
};

#endif
