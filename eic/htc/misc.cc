/* -------------------------------------------------------------------------- */
/*  misc.cc                                                                   */
/*                                                                            */
/*    Some non-HERMES specific source code shared between different HTC       */
/*  programs without particular affiliation.                                  */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cassert>
#include <cstring>
#include <cstdlib>

#include <htclib.h>

// Hmm, how about offset in ASCII table? :-) 
char XYZ[3] = {'X', 'Y', 'Z'};

/* ========================================================================== */

int check_prefix(char *str, char *prefix)
{
  int len = strlen(str), plen = strlen(prefix);

  if (len >= plen && !memcmp(str, prefix, plen)) return 0;

  return -1;
} /* check_prefix */

/* -------------------------------------------------------------------------- */

int check_and_remove_suffix(char *ptr, char *suffix)
{
  int len = strlen(ptr), slen = strlen(suffix);

  // Check suffix match;
  if (len < slen || memcmp(ptr + len - slen, suffix, slen)) return -1;

  // Cut suffix out;
  ptr[len-slen] = 0;

  return 0;
} /* check_and_remove_suffix */

/* ----------------------------------------------------------------- */

int assign_dimensional_value(char *string, char *suffix, 
			     double *value, double scale)
{
  if (!string || !suffix || !value) return -1;

  {
    // Don't want to modify "string"; 
    char *ptr = strdup(string);

    if (!ptr || check_and_remove_suffix(ptr, suffix)) return -1;

    *value = scale*atof(ptr);

    free(ptr);
  }

  return 0;
} /* assign_dimensional_value */

/* ========================================================================== */

int cmd_line_variable_parser(char *str, t_cmd_line_variable_array *array)
{
  char *qstr;
  int ip, len;

  if (!str || !array || array->actual_variable_num <= 0 ||
      array->actual_variable_num >= _CMD_LINE_VARIABLE_NUM_MAX_) 
    return -1;

  // Well, don't want to modify original string;
  qstr = strdup(str); if (!qstr) return -1; len = strlen(qstr);

  // Calculate actual_variable_num, plen, slen once;
  for(ip=0; ip<array->actual_variable_num; ip++)
  {
    t_cmd_line_variable *var = array->variables + ip;

    // Yes, reset it right here;
    var->_assigned = 0;

    // Yes, something must be wrong with definitions;
    assert(var->prefix && var->addr);
    
    var->plen = strlen(var->prefix);
  } /*if .. for ip*/

  // Loop through all known tuning keys; 
  for(ip=0; ip<array->actual_variable_num; ip++)
  {
    t_cmd_line_variable *var = array->variables + ip;

    // Check prefix match;
    if (len >= var->plen && 
	!memcmp(qstr, var->prefix, var->plen))
    {
      char *ptr = qstr + var->plen;

      // If suffix present, require suffix match;
      if (var->suffix && check_and_remove_suffix(ptr, var->suffix))
      {
	printf("'%s': syntax error (%s)!\n", var->prefix, qstr);
	return -1;
      } /*if*/

      // Check for double counting;
      if (var->_assigned)
      {
	printf("'%s': key double counting (%s)!\n", var->prefix, qstr);
	return -1;
      } /*if*/
      var->_assigned = 1;
	
      // Assign key and check limits;
      *var->addr = atof(ptr);
      if (*var->addr < var->min || *var->addr > var->max)
      {
	printf("'%s': value out of range (%s)!\n", var->prefix, qstr);
	return -1;
      } /*if*/

      return 0;
    } /*if*/
  } /*for ip*/

  // If failed before, does not matter;
  free(qstr);

  // Unknown key?;
  return -1;
} /* cmd_line_variable_parser */

/* ========================================================================== */
