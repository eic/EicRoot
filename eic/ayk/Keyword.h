/* -------------------------------------------------------------------------- */
/*  Keyword.h                                                                 */ 
/*                                                                            */
/*    Definitions related to Keyword.cc code.                                 */ 
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

//
// Prefer to change logic as less as possible in this code -> use linked
// list of keywords and a friend function to declare keywords;
//

#ifndef _KEYWORD_H
#define _KEYWORD_H

// Command line arguments starting from this prefix are 'keywords'; 
#define _KEYWORD_PREFIX_ "--"

typedef int (*keywordfun)(int argc, char **argv);

enum MultipleInvocationMode {_MULTIPLE_INVOCATION_DISABLED_, _MULTIPLE_INVOCATION_ALLOWED_};

class Keyword {
  // Yes, just a friend function;
  friend Keyword *declare_keyword(Keyword **root, char *_name, int _arg_num_min, int _arg_num_max,
				  keywordfun _fun, 
				  MultipleInvocationMode _invocation_mode /*= _MULTIPLE_INVOCATION_DISABLED_*/);
 public:
  // Constructor;
  Keyword(char *_name, int _arg_num_min, int _arg_num_max, keywordfun _fun, 
	  MultipleInvocationMode _invocation_mode);

  // The actual parser;
  int parseCommandLine(int argc, char **argv);

 private:
  // Name of the command line key (like '--geometry'); 
  char *name;
  // Expected min/max # of arguments (keyword itself excluded); 
  int arg_num_min, arg_num_max;

  // Function to be called for this keyword; 
  keywordfun fun;

  // Used to verify that the same key is not used twice in the same 
  // command line  ...;                                             
  int activated;
  // ... unless multiple invocation of this key is allowed; 
  MultipleInvocationMode __multiple_invocation_mode;

  Keyword *next;
} ;

#endif
