//
// RMP (rpetti@bnl.gov), 09-10-2014
//
// A trivial low Q^2 tagging detector wrapper class
//
//   following the example in development of other detectors by AYK
//

#ifndef _EICLOWQ2TAGGER_
#define _EICLOWQ2TAGGER_

#include <EicDetector.h>

//! the low Q^2 tagging detector wrapper class
/***/
class EicLowQ2Tagger : public EicDetector
{

 public:
  
  //! default constructor class
  /***/
  EicLowQ2Tagger() {};

  //! standard constructor
  /***/
 EicLowQ2Tagger(const char *Name, char *geometryName, EicDetectorId dType, Bool_t Active = kTRUE):
  EicDetector(Name, geometryName, dType, qMergeStepsInOneHit, Active) {};

  //! destructor
  /***/
  virtual ~EicLowQ2Tagger() {};

  ClassDef(EicLowQ2Tagger,1);

};

#endif
