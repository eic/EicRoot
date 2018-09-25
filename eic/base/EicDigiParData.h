//
// AYK (ayk@bnl.gov), 2014/10/14
//
//  Bare digi class;
//

#include <TObject.h>
#include <TString.h>

#ifndef _EIC_DIGI_PAR_DATA_
#define _EIC_DIGI_PAR_DATA_

/// Bare base class with minumum set of methods shared between various detector digi data blocks
class EicDigiParData: public TObject
{
 public:
  EicDigiParData() {};
  ~EicDigiParData() {};

  /// Dump digi data block to file
  ///
  /// @param name output file name
  int mergeIntoOutputFile(TString name);

 private:

  ClassDef(EicDigiParData,1);
};

#endif
