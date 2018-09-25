/* -------------------------------------------------------------------------- */
/*  ayk.h                                                                     */ 
/*                                                                            */
/*    Just a few defines; all the rest is now moved to other header files.    */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#ifndef _AYK_H
#define _AYK_H

// Used everywhere;
#define _X_ 0
#define _Y_ 1
#define _Z_ 2

// PI should be handy; 
#ifndef PI
//#include <math.h>
#include <cmath>
#define PI M_PI
#endif

// Yes, a macro; 
#ifndef SQR
#define SQR(x) ((x)*(x))
#endif

// Few trivial but useful defines; 
#define     mm2cm(val)  ((val)/10.)
#define      m2cm(val)  ((val)*100.)
#define   inch2cm(val)  ((val)*2.54)
#define      m2mm(val)  ((val)*1000.)
#define     cm2mm(val)  ((val)*10.)
#define      cm2m(val)  ((val)/100.)
#define   gev2mev(val)  ((val)*1000.)
#define   mev2gev(val)  ((val)/1000.)
#define   deg2rad(val)  ((val)*PI/180.)
#define  deg2mrad(val)  ((val)*PI/.18)
#define   rad2deg(val)  ((val)*180./PI)
#define  mrad2deg(val)  ((val)*.18/PI)
#define tesla2kgs(val)  ((val)*10.)

#endif
