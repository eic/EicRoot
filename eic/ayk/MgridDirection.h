/* -------------------------------------------------------------------------- */
/*  MgridDirection.h                                                          */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#ifndef _MGRID_DIRECTION_H
#define _MGRID_DIRECTION_H

//#include <asm/types.h>
// import only relevant parts here ...;
//@@#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
//typedef __signed__ long long __s64;
typedef unsigned long long __u64;
//@@#endif
//typedef __signed__ char __s8;
typedef unsigned char __u8;

//typedef __signed__ short __s16;
typedef unsigned short __u16;

//typedef __signed__ int __s32;
typedef unsigned int __u32;

//
// -> never change data fields order, etc in this class!;
//

class MgridDirection {
  friend class Mgrid;
  friend class MgridInterpolation;

 public:
  // Would need to think whether this placeholder constructor works
  // for importing binary mgrid files;
  MgridDirection() {};
  // Meaningful constuctor;
  MgridDirection(int _dim, double _min, double _max);

  // Do not want to declare few more firend functions -> allow read 
  // access to this variable;
  inline double getStep() { return step;};

 private:
  // These numbers characterize a given 
  // direction in a simple grid case;   
  __u64 dim;
  double min, max;

  // This number will in fact be calculated out of min/max/dim;   
  // but it is convenient to keep it in this same structure; yes, 
  // it is dumped into a data file;                               
  double step;
} ;

#endif
