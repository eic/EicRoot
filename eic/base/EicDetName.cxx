//
// AYK (ayk@bnl.gov), 2013/06/13
//
//  EIC detector name service class; 
//

#include <cstring>

#include "EicDetName.h"

// ---------------------------------------------------------------------------------------

EicDetName::EicDetName(const char *name)
{
  if (name)
  {
    // Upper case name (like 'TPC');
    mUname = ToUpper(name);
    
    // String name starting with capital letter (like 'Tpc');
    mCname = ToLower(name); mCname[0] = toupper(mCname[0]);
    
    // Lower case name (like 'tpc'); 
    mLname = ToLower(name);
  } //if
  //else
  //_uname = _lname = _cname = "";
} // EicDetName::EicDetName()

// ---------------------------------------------------------------------------------------

ClassImp(EicDetName)
