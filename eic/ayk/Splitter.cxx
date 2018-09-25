/* -------------------------------------------------------------------------- */
/*  Splitter.cc                                                               */ 
/*                                                                            */
/*    A simple string parser.                                                 */ 
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstring>
#include <cstdlib>

#include <Splitter.h>

/* ========================================================================== */
/*    Replaces spaces with '0' characters and stores pointers to the          */
/* corresponding substrings;                                                  */

int Splitter::splitStringCore()
{
  int len = strlen(buffer);

  // Cut out possible '\n' character; 
  if (buffer[len-1] == '\n') buffer[len-1] = 0;

  // Comments or pathological cases; is ' ' also a pathology?; 
  if (buffer[0] == '#' || buffer[0] == '\n' || !*buffer) return 0;

  // Reset substring counter;
  argn = 0;

  // Eventually disassemble string in substrings; 
  for(char *ptr = strtok(buffer, " "); ptr; ptr = strtok(NULL, " "))
  {
    if (argn == SUBSTRINGS_MAX) return -1;

    argp[argn] = ptr;
    argn++;
  } /*for*/

  return argn;
} /* Splitter::splitStringCore */

/* -------------------------------------------------------------------------- */

int Splitter::splitString(char *str)
{
  // String is too long, sorry;
  if (strlen(str)+1 > STRING_LEN_MAX) return -1;

  // Copy string over to a buffer variable (yes, I would want to modify it!);
  strcpy(buffer, str);

  return splitStringCore();
} /* splitString */

/* -------------------------------------------------------------------------- */

int Splitter::splitNextString(FILE *ff)
{              
  // Is this return code correct?;
  if (!fgets(buffer, STRING_LEN_MAX-1, ff)) return -1;

  return splitStringCore();
} /* splitNextString */

/* ========================================================================== */
