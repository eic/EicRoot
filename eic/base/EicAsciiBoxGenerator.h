//
// AYK (ayk@bnl.gov), 2015/11/06
//
//  A (temporary) hack to import HIJING ASCII files with the same interface 
//  calls which are provided with the EicBox Generator;
//

#include <iostream>

#include <TString.h>

#include <EicBoxGenerator.h>

#ifndef _EIC_ASCII_BOX_GENERATOR_
#define _EIC_ASCII_BOX_GENERATOR_

class EicAsciiBoxGenerator: public EicBoxGenerator
{
 public:
  EicAsciiBoxGenerator(const char *fileName = 0);
  ~EicAsciiBoxGenerator() {};

  Bool_t ReadEvent(FairPrimaryGenerator* primGen);
  void SetPtRange(double min, double max) { mPtMin = min; mPtMax = max; };

  void SetTrackMultiplicityLimit(unsigned mult) { mMult = mult; };

  bool IsOver() { return !mFstream || mFstream->fail() || mFstream->eof(); };

 private:
  double mPtMin, mPtMax;

  std::fstream *mFstream; //!

  unsigned mRemainingTrackCounter;

  ClassDef(EicAsciiBoxGenerator,4);
};

#endif

