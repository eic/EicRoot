/* -------------------------------------------------------------------------- */
/*  Keyword.cc                                                                */ 
/*                                                                            */
/*    A simple command line parser.                                           */ 
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <Keyword.h>

struct t_cmd_line_key {
  // Key offset in argv[] array; 
  int start;

  // Actual number of arguments for this keyword; 
  int arg_num;

  // Pointer to the keyword frame (if exists); 
  Keyword *keyword;
} ;

/* ========================================================================== */

Keyword::Keyword(char *_name, int _arg_num_min, int _arg_num_max, keywordfun _fun, 
		 MultipleInvocationMode _invocation_mode)
{
  memset(this, 0x00, sizeof(Keyword));

  // Allocate keyword name and parameters; 
  name = strdup(_name);
  //if (!name) return NULL;
  assert(name);

  arg_num_min                 = _arg_num_min;
  arg_num_max                 = _arg_num_max;
  fun                         = _fun;

  __multiple_invocation_mode  = _invocation_mode;
} /* Keyword::Keyword */

/* ========================================================================== */

static int isKeyword(char *argument)
{
  int len = strlen(_KEYWORD_PREFIX_);

  if (strlen(argument) < len) return 0;
  if (memcmp(argument, _KEYWORD_PREFIX_, len)) return 0;

  // Success (this argument is a keyword); 
  return 1;
} /*  isKeyword */

/* -------------------------------------------------------------------------- */
/*  After a number of cross-checks append this command line keyword to the    */
/* existing list;                                                             */

Keyword *declare_keyword(Keyword **root, char *_name, int _arg_num_min, int _arg_num_max,
			 keywordfun _fun, MultipleInvocationMode _invocation_mode)
{
  // Sanity check; 
  if (!isKeyword(_name))
  {
    printf("'%s' does not look like a keyword!\n", _name);
    return NULL;
  } /*if*/

  // Loop through the existing keywords and check for double counting; 
  while (*root)
  {
    if (!strcmp(_name, (*root)->name)) 
    {
      printf("Keyword '%s' encountered twice\n", _name);
      return NULL;
    } /*if*/

    root = &(*root)->next;
  } /*while*/

  Keyword *key = *root = new Keyword(_name, _arg_num_min, _arg_num_max, _fun, 
				     _invocation_mode);

  return key;
} /* declare_keyword */

/* ========================================================================== */

#define FALLOUT(key, why, ret) \
  {printf("\n  [PARSER]: '%s' --> %s!\n\n", (key), (why)); return (ret); }

/* -------------------------------------------------------------------------- */
/*  Well, do not care too much about performance optimization (parsing is     */
/* fast and done only once);                                                  */

int Keyword::parseCommandLine(int argc, char **argv) 
{
  int key_num = 0;
  // 'argc': certainly not more; 
  t_cmd_line_key keys[argc];

  // No arguments --> Ok; 
  if (argc == 1) return 0;

  // The very first command lie parameter must be a keyword; 
  if (!isKeyword(argv[1])) FALLOUT(argv[1], "first argument is not a keyword", -1);

  // Loop through all the command line arguments and find keywords; 
  for(int ik=1; ik<argc; ik++)
    if (isKeyword(argv[ik]))
      keys[key_num++].start = ik;

  // Prefer to check everything first and start execution in     
  // separate loop only after making sure that everything is Ok; 
  for(int ik=0; ik<key_num; ik++)
  {
    t_cmd_line_key *key = keys + ik;

    // Calculate number of arguments for this keyword; 
    key->arg_num = (ik == key_num - 1) ? argc - key->start - 1 : 
      (key+1)->start - key->start - 1; 

    // Now loop through all known keywords and find a match; 
    for(Keyword *pattern=this; pattern; pattern=pattern->next)
      if (!strcmp(argv[key->start], pattern->name))
      {
	// Means this key was already found in the command line; 
	if (pattern->activated && 
	    pattern->__multiple_invocation_mode == _MULTIPLE_INVOCATION_DISABLED_) 
	  FALLOUT(pattern->name, "key used more than once", -2);

	if (pattern->arg_num_min != -1 && key->arg_num < pattern->arg_num_min) 
	  FALLOUT(pattern->name, "wrong number of arguments", -3);
	if (pattern->arg_num_max != -1 && key->arg_num > pattern->arg_num_max) 
	  FALLOUT(pattern->name, "wrong number of arguments", -3);

	pattern->activated = 1;

	key->keyword = pattern;

	goto _next_key;
      } /*for..if*/

    // No match; 
    FALLOUT(argv[key->start], "unknown key", -4);

_next_key:;
  } /*for*/

    // Ok, now may actually call parser functions; 
  for(int ik=0; ik<key_num; ik++)
  {
    t_cmd_line_key *key = keys + ik;

    int ret = key->keyword->fun(key->arg_num, argv + key->start + 1);

    if (ret) FALLOUT(key->keyword->name, "syntax error", ret);
  } /*for*/

  return 0;
} /* Keyword::parseCommandLine */

/* ========================================================================== */
