/* -------------------------------------------------------------------------- */
/*  Splitter.h                                                                */ 
/*                                                                            */
/*    Definitions related to Splitter.cc (C string disassembler) code.        */ 
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>

#ifndef _SPLITTER_H
#define _SPLITTER_H

// Yes, do not care about dynamic allocation;
#define STRING_LEN_MAX 256
#define SUBSTRINGS_MAX 256

class Splitter {
public:
  Splitter() {argn = 0;};

  int splitString(char *str);
  int splitNextString(FILE *ff);

  // Public access to "argn" & "argp" variables;
  int getArgn() { return argn;};
  // 'const': yes, read access is assumed;
  const char *getArgp(unsigned id) { return id < argn ? argp[id] : 0;};

private:
  // Array with substring pointers;
  unsigned argn;
  char *argp[SUBSTRINGS_MAX];

  // Well, see no good reason to allow too long strings; if ever need this,
  // just malloc/free this pointer dynamically;
  char buffer[STRING_LEN_MAX];

  int splitStringCore();
} ;

#endif
