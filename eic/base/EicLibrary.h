//
// AYK (ayk@bnl.gov), 2014/09/04
//
//  Few common-use library routines with no particular class affiliation;
//

#include <TString.h>

#ifndef _EIC_LIBRARY_
#define _EIC_LIBRARY_

bool IsSortOfAbsolutePath(const char *fileName);
TString ExpandedFileName(const char *fileName);
TString ExpandedFileName(const char *prefix, const char *fileName);

#endif
