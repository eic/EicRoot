/* -------------------------------------------------------------------------- */
/*  runge-kutta.c                                                             */
/*                                                                            */
/*   My private Runge-Kutta test code.                                        */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cassert>
#include <cstdlib>

#include <htclib.h>
#include <TrKalmanFilter.h>

// 0.: no restriction per default;
double RK_cell_width_max, RK_fixed_cell_width;
double RK_small_step_limit = _RK_SMALL_STEP_DEFAULT_;

int RK_small_step_order = _RK_ORDER_4_;
t_htc_interpolation RK_htci = {_MODE_ADIM_, {2, 2, 1}};

// An ugly way to record situations of no field available;
int field_missing_flags[_FIELD_AREA_NUM_MAX_];

// Can be changed from the command line to old-style code borrowed from HERA-B;
int RK_flavor = _RK_HERMES_;

// Perhaps tune limits later (or allow certain flexibility to modify them);
// also it may be wise to have these arrays detector-specific; @@@MM@@@
//
// Ok, as of 2012/11/28 so initialize 'plen' and '_assigned' fields and make 
// both Jan and compiler happy;
static t_cmd_line_variable_array RK_array = 
  {3, 
   {{(char*)"cell-width-max=",   (char*)"cm",  &RK_cell_width_max,   2.0,   10.0, 0, 0},
    {(char*)"fixed-cell-width=", (char*)"cm",  &RK_fixed_cell_width, 2.0,   10.0, 0, 0},
    {(char*)"small-step-limit=", (char*)"cm",  &RK_small_step_limit, 0.1,  100.0, 0, 0}}};

#define _SMALL_STEP_ORDER_PREFIX_ "small-step-order="

/* ========================================================================== */

int parse_htc_interpolation_string(char *string, t_htc_interpolation *htci)
{
  int ret = 0;
    
  if (!htci)                                        return -1;

  // Duplicate sanity check, sorry;
  if (check_prefix(string, (char*)_INTERPOLATION_PREFIX_)) return -1;

  {
    char *ptr = strdup(string + strlen(_INTERPOLATION_PREFIX_));

    if (!ptr) return -1;

    // 2 predefined keys;
    if (!strcmp(ptr, "off"))
      htci->mode = _MODE_OFF_;
    else
    if (!strcmp(ptr, "hrc"))
      htci->mode = _MODE_HRC_;
    else
    {
      // Parse AxBxC string;
      int iq;
      char *qptr = ptr;

      htci->mode = _MODE_ADIM_;

      for(iq=0; iq<3; iq++)
      {
	char *x = strchr(qptr, 'x');

	if ((iq < 2 && !x) || (iq == 2 && x))
	{
	  ret = -1;
	  break;
	} /*if*/

	if (x) *x = 0;

	htci->adim[iq] = atoi(qptr);
	if (htci->adim[iq] <= 0 || htci->adim[iq] > 4)
	{
	  ret = -1;
	  break;
	} /*if*/

	if (x) qptr = x + 1;
      } /*for iq*/
    } /*if*/

    free(ptr);
  }
  
  return ret;
} /* parse_htc_interpolation_string */

/* -------------------------------------------------------------------------- */

int runge_kutta_fun(int argc, char **argv)
{
  // Well, first key is mode;
  if (!strcmp(argv[0], "mode=hermes"))
    // This is actually the default;
    RK_flavor = _RK_HERMES_;
  else
  if (!strcmp(argv[0], "mode=hera-b"))
  {
    // Yes, no modifiers for this case;
    if (argc != 1) return -1;

    RK_flavor = _RK_HERA_B_;

    return 0;
  }
  else
    return -1;

  // Parse other keys in case of HERMES mode;
  for(int ik=1; ik<argc; ik++)
  {
    char *ptr = argv[ik];

    if (!check_prefix(ptr, (char*)_INTERPOLATION_PREFIX_))
    {
      if (parse_htc_interpolation_string(ptr, &RK_htci))
	_RETURN_(-1, "'--runge-kutta': failed to parse interpolation string!\n");
      if (RK_htci.mode == _MODE_ADIM_ && RK_htci.adim[_Z_] != 1)
	_RETURN_(-1, "'--runge-kutta': only (1..4)x(1..4)x(1) modes alowed!\n");
    }
    else
      if (!check_prefix(ptr, (char*)_SMALL_STEP_ORDER_PREFIX_))
    {
      switch(atoi(ptr + strlen(_SMALL_STEP_ORDER_PREFIX_)))
      {
      case 2:
	RK_small_step_order = _RK_ORDER_2_;
	break;
      case 4:
	RK_small_step_order = _RK_ORDER_4_;
	break;
      default:
	return -1;
      } /*switch*/
    }
    else
    {
      // Eventually try to call standardized parser; yes, this all looks
      // rather messy;
      if (cmd_line_variable_parser(ptr, &RK_array))
      {
	printf("'--runge-kutta': unknown config key: '%s'!\n", ptr);
	return -1;
      } /*if*/
    } /*if*/
  } /*for ik*/

  return 0;
} /* runge_kutta_fun */

/* ========================================================================== */
