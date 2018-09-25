/* ----------------------------------------------------------------- */
/*  htclib.h                                                         */ 
/*                                                                   */
/*   HTC library code definitions.                                   */
/*                                                                   */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                         */
/*    e-mail: kisselev@hermes.desy.de                                */
/* ----------------------------------------------------------------- */

// Unify later!!!!!!!!!!!;
#define _SX_ 2
#define _SY_ 3
#define _QP_ 4

// Comment this out if want to use usual ~(1. + 0.038*log(X/X0)) Coulomb formula; 
#define _USE_GEANT3_MOLIERE_CHC_

#include <ayk.h>
#include <Mgrid.h>

#define f2cFortran
#define CFSUBASFUN
#include <cfortran.h>

#ifndef _HTC_LIB_
#define _HTC_LIB_

// Positive/negative charges;
#define _POSITIVE_  0
#define _NEGATIVE_  1

extern char XYZ[3];

#define _RETURN_(ret, message) { if (message) printf(message); return(ret); }
#define _TERMINATE_(ret, message) { printf(message); exit  (ret); }

//#ifdef _USE_CERNLIB_
//#ifdef CFSUBASFUN
PROTOCCALLSFFUN4(INT,DSINV,dsinv,INT,DOUBLEV,INT,PINT)
PROTOCCALLSFFUN5(INT,DINV, dinv, INT, DOUBLEV, INT, INTV, PINT)
//#endif

#define DSINV(N,A,IDIM,IFAIL) \
  CCALLSFSUB4(DSINV,dsinv,INT,DOUBLEV,INT,PINT,N,A,IDIM,IFAIL)
#define DINV(N, A, IDIM, IR, IFAIL) \
  CCALLSFSUB5(DINV, dinv, INT, DOUBLEV, INT, INTV, PINT, N, A, IDIM, IR, IFAIL)

PROTOCCALLSFFUN2(FLOAT,PROB,prob,FLOAT,INT)
#define PROB(X,N) CCALLSFFUN2(PROB,prob_htc,FLOAT,INT,X,N)
//#endif

// Increase and recompile if ever not enough;
#define _CMD_LINE_VARIABLE_NUM_MAX_ 10

typedef struct {
  // Pattern names; prefer to have suffices for sompleteness as well;
  char *prefix, *suffix;

  // Address of respective variable; 
  double *addr;

  // Some more or less sane limits; let them be double?;
  double min, max;

  // Pattern length; calculate once;
  int plen;

  // Well, one change allowed, naturally; keep track on that;
  int _assigned;

  //
  // Do not change parameter order above this line;
  //
} t_cmd_line_variable;

typedef struct {
  // Actual number of variables (<= _CMD_LINE_VARIABLE_NUM_MAX_);
  int actual_variable_num;

  // Array of keys;
  t_cmd_line_variable variables[_CMD_LINE_VARIABLE_NUM_MAX_];


  // Do not change parameter order above this line;


} t_cmd_line_variable_array;

#ifdef  __cplusplus
extern "C" {
#endif
  int check_prefix(char *str, char *prefix);
  int check_and_remove_suffix(char *ptr, char *suffix);
  int assign_dimensional_value(char *string, char *suffix, 
			       double *value, double scale);
  int cmd_line_variable_parser(char *str, t_cmd_line_variable_array *array);

  int redirect_fortran_output(char *std_out, char *std_err);
#ifdef  __cplusplus
}
#endif

#define G3ZEBRA(P1) CCALLSFSUB1(G3ZEBRA, g3zebra, INT, P1)
PROTOCCALLSFFUN1(INT,G3ZEBRA, g3zebra, INT)

#define G3INIT() CCALLSFSUB0(G3INIT, g3init) 
PROTOCCALLSFFUN0(INT,G3INIT, g3init)
#define G3ZINIT() CCALLSFSUB0(G3ZINIT, g3zinit)
PROTOCCALLSFFUN0(INT,G3ZINIT, g3zinit)
#define G3PHYSI() CCALLSFSUB0(G3PHYSI, g3physi) 
PROTOCCALLSFFUN0(INT,G3PHYSI, g3physi)

#define G3DRELX(A,Z,DENS,T,HMASS,DEDX)					\
  CCALLSFSUB6(G3DRELX,g3drelx,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT,A,Z,DENS,T,HMASS,DEDX)
PROTOCCALLSFFUN6(INT,G3DRELX,g3drelx,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT)

#define G3PROBI(NLMAT,WMAT,AAA,ZZZ,DENS,POTL,FAC,C,X0,X1,AA) \
  CCALLSFSUB11(G3PROBI,g3probi,INT,FLOATV,FLOATV,FLOATV,FLOAT, \
    PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,NLMAT,WMAT,AAA,ZZZ,DENS,POTL,FAC,C,X0,X1,AA)
PROTOCCALLSFFUN11(INT,G3PROBI,g3probi,INT,FLOATV,FLOATV,FLOATV,FLOAT,	\
		  PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE)

#define G3MOLI(P1, P2, P3, P4, P5, P6, P7) \
  CCALLSFSUB7(G3MOLI, g3moli, FLOATV, FLOATV, FLOATV, INT, FLOAT, PFLOAT, PFLOAT, \
	      P1, P2, P3, P4, P5, P6, P7)
PROTOCCALLSFFUN7(INT,G3MOLI, g3moli, FLOATV, FLOATV, FLOATV, INT, FLOAT, PFLOAT, PFLOAT)

#if 0
#define GZEBRA(P1) CCALLSFSUB1(GZEBRA, gzebra, INT, P1)
#define GINIT() CCALLSFSUB0(GINIT, ginit) 
#define GZINIT() CCALLSFSUB0(GZINIT, gzinit)
#define GPART() CCALLSFSUB0(GPART, gpart) 
#define GPHYSI() CCALLSFSUB0(GPHYSI, gphysi)
     
#define GMOLI(P1, P2, P3, P4, P5, P6, P7) \
  CCALLSFSUB7(GMOLI, gmoli, FLOATV, FLOATV, FLOATV, INT, FLOAT, PFLOAT, PFLOAT, \
	      P1, P2, P3, P4, P5, P6, P7)

#define GDRELX(A,Z,DENS,T,HMASS,DEDX)					\
  CCALLSFSUB6(GDRELX,gdrelx,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT,A,Z,DENS,T,HMASS,DEDX)
#define G3DRELX(A,Z,DENS,T,HMASS,DEDX)					\
  CCALLSFSUB6(G3DRELX,g3drelx,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT,A,Z,DENS,T,HMASS,DEDX)
#define GDRELP(A,Z,DENS,T,DEDX) \
  CCALLSFSUB5(GDRELP,gdrelp,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT,A,Z,DENS,T,DEDX)

#if 1
// These routines have a different interface than original CERNLIB ones,
// just because I had to decouple them from the source common blocks;
#define GDRELE(EEL,CHARGE,POTL,FAC,C,X0,X1,AA,DEDX) \
  CCALLSFSUB9(GDRELE,gdrele,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,PDOUBLE, \
    EEL,CHARGE,POTL,FAC,C,X0,X1,AA,DEDX)
#define GPROBI(NLMAT,WMAT,AAA,ZZZ,DENS,POTL,FAC,C,X0,X1,AA) \
  CCALLSFSUB11(GPROBI,gprobi,INT,FLOATV,FLOATV,FLOATV,FLOAT, \
    PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,NLMAT,WMAT,AAA,ZZZ,DENS,POTL,FAC,C,X0,X1,AA)

#define GBRELA(NLMAT,WMAT,AAA,ZZZ,AAVG,ZAVG,DENS,EEL,CHARGE,DEDX) \
  CCALLSFSUB10(GBRELA,gbrela,INT,FLOATV,FLOATV,FLOATV,FLOAT,FLOAT,FLOAT,DOUBLE,DOUBLE,PDOUBLE, \
    NLMAT,WMAT,AAA,ZZZ,AAVG,ZAVG,DENS,EEL,CHARGE,DEDX)
#endif

#ifdef CFSUBASFUN
PROTOCCALLSFFUN7(INT,GMOLI, gmoli, FLOATV, FLOATV, FLOATV, INT, FLOAT, PFLOAT, PFLOAT)
PROTOCCALLSFFUN11(INT,GPROBI,gprobi,INT,FLOATV,FLOATV,FLOATV,FLOAT,	\
		  PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE,PDOUBLE)
PROTOCCALLSFFUN6(INT,GDRELX,gdrelx,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT)
PROTOCCALLSFFUN6(INT,G3DRELX,g3drelx,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,PFLOAT)
PROTOCCALLSFFUN9(INT,GDRELE,gdrele,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,PDOUBLE)
PROTOCCALLSFFUN10(INT,GBRELA,gbrela,INT,FLOATV,FLOATV,FLOATV,FLOAT,FLOAT,FLOAT,DOUBLE,DOUBLE,PDOUBLE)
PROTOCCALLSFFUN0(INT,GPHYSI, gphysi)
PROTOCCALLSFFUN0(INT,GPART, gpart) 
PROTOCCALLSFFUN0(INT,GINIT, ginit)
PROTOCCALLSFFUN0(INT,GZINIT, gzinit)
PROTOCCALLSFFUN1(INT,GZEBRA, gzebra, INT)
#endif
#endif

typedef struct {
  char name[5];

  // Byte ordering will be different on different platforms       
  // therefore do not want to create union; well, in fact         
  // I'm not going to port it to any other platform than Linux:-) 
  int id;
} t_geant_name;

// Prefer to specify dE/dx losses model explicitely;
#define _DEDX_HADRON_   0
#define _DEDX_ELECTRON_ 1

typedef struct {
  // Some name like "Pi+"; 
  char *name;

  // GEANT3 & GEANT4 identifiers; 
  int geant3, geant4;
} t_particle;

struct t_particle_group {
  // Group name (like "pion" for both Pi+ & Pi-); Kalman filter 
  // requires particle hypothesis, for instance, and it is natural 
  // to give just "pion" from the command line assuming that a proper
  // charge sign will be picked up;
  char *grname;

  // Yes, assume mass is the same; actually GEANT4 identifiers are 
  // also the same up to a sign;
  double mass;

  // See 2 possible options above;
  int dE_dx_model;

  // +/- charges; yes, no neutral particles assumed for now;
  t_particle members[2];
};

extern t_particle_group particle_groups[];
extern int particle_group_num;

#ifdef  __cplusplus
extern "C" {
#endif
  t_particle *get_particle_by_name(const char *name);
  t_particle_group *get_particle_group_by_name(const char *name);
#ifdef  __cplusplus
}
#endif

// Different head node initialization modes;
#define _USE_XF_ 0
#define _USE_XS_ 1
#define _USE_00_ 2

// Never change these numbers if use them as array indices!;
#define _RK_ORDER_2_ 0
#define _RK_ORDER_4_ 1
#define _RK_ORDER_5_ 2

extern double _drv_steps[4];

// If this bit is not set, only gaps between nodes with non-linear 
// transport will be filled by intermediate ones; 
//#define _FILL_ALL_GAPS_   0x0001

// HERMES: TARGET/FRINGE/SPEC, right?; if ever becomes too small,
// modify and recompile;
#define _FIELD_AREA_NUM_MAX_   3

extern int field_missing_flags[_FIELD_AREA_NUM_MAX_];

// For steps less than this number use just 2-d (or 4-th?) order Runge-Kutta;
#define _RK_SMALL_STEP_DEFAULT_ (2.0)

// Light velocity --> cm/s; @@@MM@@@
#define _LIGHT_SPEED_ (299792458E2)

extern t_htc_interpolation RK_htci;

// Yes, it's a bad idea to make such variables global; fix later;
extern double a2, a3, a4, a5, a6;

#ifdef  __cplusplus
extern "C" {
#endif
  int parse_htc_interpolation_string(char *string, t_htc_interpolation *htci);

  int runge_kutta_fun(int argc, char **argv);
#ifdef  __cplusplus
}
#endif

#define _INTERPOLATION_PREFIX_ "interpolation="

#define _RK_HERMES_ 0
#define _RK_HERA_B_ 1

extern int RK_flavor;

extern double RK_fixed_cell_width, RK_cell_width_max, RK_small_step_limit;
extern int RK_small_step_order;

#endif
