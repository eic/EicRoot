/* -------------------------------------------------------------------------- */
/*  ThreeDeePolynomial.h                                                      */
/*                                                                            */
/*    3D polynomial basic routines header file.                               */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#ifndef _THREE_DEE_POLYNOMIAL_H
#define _THREE_DEE_POLYNOMIAL_H

class ThreeDeePolySpace;

class ThreeDeePolynomial {
  friend class ThreeDeePolySpace;

 public:
  // Constructor;
  ThreeDeePolynomial(ThreeDeePolySpace *_space);

  // Polynomial value at a given point;
  double value(double xx[3]);

  // Calculate polynomial gradient;
  int calculateGradient(ThreeDeePolynomial *gradient[3]);

 private:
  // Polynomial may be turned off alltogether; really used?;
  int off;

  // Back door; in principle one could use a declared polynomial 
  // on top of some other (parameter-compatible) ThreeDeePolySpace object;
  // to this point I just do not want to overload the code; should 
  // check however that some functionality of mgrid routines (the
  // only ones which are actually using this stuff) is not lost 
  // after moving this pointer from parameters to ThreeDeePolynomial class
  // variable;
  ThreeDeePolySpace *space;

  // 'linear' means 'dim' coefficients with respect to the 
  // basis vectors; 'cff' are 3-dim array of coefficients  
  // to polynomials like 'x2yz';                           
  double ***cff, *linear;

  // Get polynomial value at a given 3D space point;
  double linearValue(double xx[3]);

  // Normalization, rescaling, increment; do not bother to create 
  // overloaded operators, sorry;
  int normalize();
  void multiply(double _cff);
  void increment(ThreeDeePolynomial *incr);

  // Recalculate basis representation to cff[] array;
  void convertLinearToCff();
} ;

#endif
