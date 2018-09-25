//
// AYK (ayk@bnl.gov), 2013/06/12
//
//  FairRoot-style EIC container factory definitions
//

#ifndef _EIC_CONT_FACT_
#define _EIC_CONT_FACT_

#include "FairContFact.h"

class FairContainer;
class EicDetector;

/// FairRoot-style EIC container factory, whatever it is good for
class EicContFact : public FairContFact {
public:
  /// Parameters are strings needed to initialize respective FairContainer 
  EicContFact(EicDetector *det, const char *_fName, const char *_fTitle, 
	      const char* name, const char* title, const char* context);
 EicContFact(): fGeoParName(0), backDoorDetPtr(0) {};
  ~EicContFact() {};
  FairParSet* createContainer(FairContainer*);

 private:
  /// \brief Service variable like \a TpcGeoPar 
  ///
  /// Let it remain char*, does not really matter
  char *fGeoParName;

  /// \brief Pointer to EicDetector frame which called this function
  ///
  /// Well, want to be able to call detector-specific allocator of EicGeoPar 
  /// derivative class in EicContFact::createContainer(); this is indeed not the best 
  /// quality C++ way of doing things, but I do not want to either cut'n'paste 
  /// createContainer() in several places or to mess up with templates; inheritance
  /// between EicDetector and EicContFact would probably help, but they both have 
  /// TObject as a base class.
  EicDetector *backDoorDetPtr;

  ClassDef(EicContFact,4) 
};

#endif  
