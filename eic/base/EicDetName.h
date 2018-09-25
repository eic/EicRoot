//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC detector name service class; just for convenience ...;
//

#include "TString.h"
#include "TObject.h"

#ifndef _EIC_DET_NAME_
#define _EIC_DET_NAME_

/// Detector name in all flavors like \a TPC, \a Tpc, \a tpc
class EicDetName : public TObject {
public:
  /// Default constructor
  ///
  EicDetName() {};

  /// Main constructor
  ///
  /// @param name detector name 
  EicDetName(const char *name);
  
  /// Destructor
  ///
  virtual ~EicDetName() {};

  /// upper case name, like \a TPC
  ///
  TString const &NAME() const { return mUname; }
  /// lower case name, like \a tpc
  ///
  TString const &name() const { return mLname; }
  /// mixed case name, starting with capital letter, like \a Tpc
  ///
  TString const &Name() const { return mCname; }

 private:
  // Detector name in all the variants (upper case, lower case, mixed); 
  TString mUname, mLname, mCname;

  ClassDef(EicDetName,1);
};

#endif
