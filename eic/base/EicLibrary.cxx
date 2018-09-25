//
// AYK (ayk@bnl.gov), 2014/09/04
//
//  Few common-use library routines with no particular class affiliation;
//

#include <stdlib.h>

#include <EicLibrary.h>

// =======================================================================================

//
// Basically a signature, that file name expansion is not needed at all (absolute 
// path or a path with respect to the currect directory);
//

bool IsSortOfAbsolutePath(const char *fileName)
{
  if (!fileName) return false;
  
  // Make life easier;
  TString str(fileName);

  return (str.BeginsWith("/") || str.BeginsWith("./") || str.BeginsWith("../"));
} // IsSortOfAbsolutePath()

// ---------------------------------------------------------------------------------------

TString ExpandedFileName(const char *prefix, const char *fileName)
{
  if (!fileName) return TString("");

  // Well, I guess in this case just ignore prefix;
  if (IsSortOfAbsolutePath(fileName)) return TString(fileName);

  TString expandedFileName(fileName);
  
  // Prepend prefix if given; THINK: take care to put '/' if missing?;
  if (prefix) expandedFileName = prefix + expandedFileName;
  //expandedFileName = TString(prefix) + (prefix[strlen(prefix)-1] == '/' ? "" : "/") + 
  //  expandedFileName;

  return TString(getenv("VMCWORKDIR")) + "/" + expandedFileName;
} // ExpandedFileName()

// ---------------------------------------------------------------------------------------

TString ExpandedFileName(const char *fileName)
{
  return ExpandedFileName(0, fileName);
} // ExpandedFileName()

// =======================================================================================
