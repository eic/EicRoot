/* -------------------------------------------------------------------------- */
/*  ThreeDeePolySpace.h                                                       */
/*                                                                            */
/*    Fitting routines with orthogonal polynomials header file.               */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <3d.h>

#include <ThreeDeePolynomial.h>

#ifndef _THREE_DEE_POLY_SPACE_H
#define _THREE_DEE_POLY_SPACE_H

struct ThreeDeePolyPoint {
  TVector3 xx;

  // NB: need to assign "weight" if ever want to use it!;
  double f, weight;
} ;

#define _ODD_  0x01
#define _EVEN_ 0x02
#define _BOTH_ 0x03

class ThreeDeePolySpace {
  friend class ThreeDeePolynomial;

 public:
  // Constructor; there is no good reason to split off 3D point allocation;
  // as of conversion to C++ just put this part of code into constructor; 
  // this restricts the functionality a bit (no option to change/reallocate 
  // point set later), but I do not seem to need this anyway; also this 
  // solves a potential problem of inconsistency betwee "space" and "poly->space"
  // pointers; fine;
  ThreeDeePolySpace(int max_power[3], int _point_num, unsigned char _parity[3] = 0);

  // 3D point array; will move to private once convert mgrid routines to C++;
  int point_num;
  ThreeDeePolyPoint *points;

  // Fitting polynomial calculation using basis[] built on top of the 3D 
  // points[] set;
  void calculateFittingPolynomial(ThreeDeePolynomial *fit);

  // Orthogonal polynomial calculation for a given set of points[];
  void buildOrthogonalPolynomials();

  // Naive fit error estimate;
  double getNaivePolyFitError(double xx[3]);

 private:
  // Max polynomial power in XYZ (1 means p0 only); 
  int _max_power[3], degrees_of_freedom[3];

  // It can happen that in irregular case not all the expected      
  // basis polynomials are independent; prefer to introduce         
  // a cutoff parameter for these purposes; yes, it's a poor style; 
  // but it is not that easy to figure out number of independent    
  // linear combinations;                                           
  double norm_cutoff;

  // _ODD_/_EVEN_/_ANY_; not really used;
  unsigned char parity[3];

  // Max number of independent polynomials; 
  int dim;
  // Basis polynomials; 
  ThreeDeePolynomial **basis;
  // Derivatives of basis polynomials; '3': db/d{XYZ}; 
  ThreeDeePolynomial ***dbasis;

  // Buffer polynomial for intermediate calculations;
  ThreeDeePolynomial *buffer;

  // Basically the difference between "point_num" and "dim";
  int getNDF();

  // Polynomial copy, product and projection operations; do not 
  // bother to create overloaded operators for the first 2 functions;
  void polyCopy(ThreeDeePolynomial *dest, ThreeDeePolynomial *source);
  double polyProduct(ThreeDeePolynomial *p1, ThreeDeePolynomial *p2);
  double polyProjection(ThreeDeePolynomial *poly);

  // As the name says: calculate gradients of basis[] polynomials;
  int buildBasisGradients();

  // chi^2 of a polynomial fit;
  double getPolyFitChiSquare(ThreeDeePolynomial *fit);

  // "Realistic" polynomial fit error;
  double getRealisticPolyFitError(ThreeDeePolynomial *fit, double xx[3]);
} ;

// Well, eventually PARITY_CHECK() loops must be optimized; 
#define PARITY_CHECK(space, xyz, i) ((space)->parity[xyz] & ((i)%2 ? _ODD_ : _EVEN_))

#endif
