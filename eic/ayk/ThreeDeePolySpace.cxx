/* -------------------------------------------------------------------------- */
/*  ThreeDeePolySpace.cc                                                      */
/*                                                                            */
/*    Collection of fitting routines with orthogonal polynomials.             */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>

#include <ayk.h>
#include <ThreeDeePolySpace.h>

/* ========================================================================== */

ThreeDeePolySpace::ThreeDeePolySpace(int max_power[3], int _point_num, unsigned char _parity[3])
{
  int odd, even;

  // Check later, whether this is really needed;
  memset(this, 0x00, sizeof(ThreeDeePolySpace));

  // Start off "1"; the actual calculation is below;
  dim = 1;

  // Figure out overall dimension; 
  for(int ik=0; ik<3; ik++)
  {
    _max_power[ik] = max_power[ik];

    parity[ik] = _parity ? _parity[ik] : _BOTH_;

    // Figure out actual number of degrees of freedom; 
    degrees_of_freedom[ik] = 0;
    odd = even = max_power[ik]/2;
    if (max_power[ik]%2) even++;
    if (parity[ik] & _ODD_)  degrees_of_freedom[ik] += odd;
    if (parity[ik] & _EVEN_) degrees_of_freedom[ik] += even;

    dim *= degrees_of_freedom[ik];
  } /*for*/

    // Allocate memory for basis and derivatives; 
  basis = new ThreeDeePolynomial* [dim];
  for(int ik=0; ik<dim; ik++)
    basis[ik] = new ThreeDeePolynomial(this);

  dbasis = new ThreeDeePolynomial** [dim];
  for(int ii=0; ii<dim; ii++)
  {
    dbasis[ii] = new ThreeDeePolynomial* [3];

    for(int ik=0; ik<3; ik++)
      dbasis[ii][ik] = new ThreeDeePolynomial(this);
  } /*for*/

    // Working space; 
  buffer = new ThreeDeePolynomial(this);

  // Allocate 3D points;
  point_num = _point_num;
  points = new ThreeDeePolyPoint [point_num];

  // A reasonable default for weight, please; 
  for(int ik=0; ik<point_num; ik++)
    points[ik].weight = 1.;
} /* ThreeDeePolySpace::ThreeDeePolySpace */

/* ========================================================================== */
/*   Does not take into account possibility that some of the directions may   */
/* have too few points; fix later;                                            */

int ThreeDeePolySpace::getNDF()
{
  return point_num - dim;
} /* ThreeDeePolySpace::getNDF */

/* ========================================================================== */

//
//  -> do not bother to create operators; just convert functions to methods;
//     since at least few of these are logically related to ThreeDeePolySpace rather
//     than ThreeDeePolynomial class, put them all into ThreeDeePolySpace;  
//

double ThreeDeePolySpace::polyProduct(ThreeDeePolynomial *p1, ThreeDeePolynomial *p2)
{
  double ret = 0.0;

  for(int pt=0; pt<point_num; pt++)
  {
    ThreeDeePolyPoint *point = points + pt;

    assert(0);
    //@@@ ret += point->weight*p1->value(point->xx)*p2->value(point->xx);
  } /*for*/

  return ret;
} /* ThreeDeePolySpace::polyProduct */

/* -------------------------------------------------------------------------- */

double ThreeDeePolySpace::polyProjection(ThreeDeePolynomial *poly)
{
  double ret = 0.0;

  if (poly->off) return 0.0;

  for(int pt=0; pt<point_num; pt++)
  {
    ThreeDeePolyPoint *point = points + pt;

    assert(0);
    //@@@ ret += point->weight*poly->value(point->xx)*point->f;
  } /*for*/

  return ret;
} /* ThreeDeePolySpace::polyProjection */

/* -------------------------------------------------------------------------- */

void ThreeDeePolySpace::polyCopy(ThreeDeePolynomial *dest, ThreeDeePolynomial *source)
{
  int ix, iy, iz;

  for(ix=0; ix<_max_power[_X_]; ix++)
  {
    if (!PARITY_CHECK(this, _X_, ix)) continue;

    for(iy=0; iy<_max_power[_Y_]; iy++)
    {
      if (!PARITY_CHECK(this, _Y_, iy)) continue;

      for(iz=0; iz<_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(this, _Z_, iz)) continue;

	dest->cff[ix][iy][iz] = source->cff[ix][iy][iz];
      } /*for*/
    } /*for*/
  } /*for*/
} /* ThreeDeePolySpace::polyCopy */

/* -------------------------------------------------------------------------- */

void ThreeDeePolySpace::buildOrthogonalPolynomials()
{
  // Do I actually need this "off_counter"?;
  int id = 0, off_counter = 0;

  // Start with 1, x, x^2, ...; 
  for(int ix=0; ix<_max_power[_X_]; ix++)
    for(int iy=0; iy<_max_power[_Y_]; iy++)
      for(int iz=0; iz<_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(this, _X_, ix) ||
	    !PARITY_CHECK(this, _Y_, iy) ||
	    !PARITY_CHECK(this, _Z_, iz)) 
	  continue;

	{
	  ThreeDeePolynomial *poly = basis[id++];
	
	  for(int qx=0; qx<_max_power[_X_]; qx++)
	    for(int qy=0; qy<_max_power[_Y_]; qy++)
	      for(int qz=0; qz<_max_power[_Z_]; qz++)
		poly->cff[qx][qy][qz] = 0.;
	  
	  poly->cff[ix][iy][iz] = 1.;
	}
      } /*for..for*/

  for(int ii=0; ii<dim; ii++)
  {
    for(int ik=0; ik<ii; ik++)
    {
      double projection = polyProduct(basis[ii], basis[ik]);

      polyCopy(buffer, basis[ik]);
      buffer->multiply(-projection);
      basis[ii]->increment(buffer);
    } /*for*/

    if (basis[ii]->normalize()) off_counter++;
  } /*for*/
} /* ThreeDeePolySpace::buildOrthogonalPolynomials */

/* ========================================================================== */

int ThreeDeePolySpace::buildBasisGradients()
{
  for(int ii=0; ii<dim; ii++)
    if (basis[ii]->calculateGradient(dbasis[ii]))
      return -1;

  return 0;
} /* ThreeDeePolySpace::buildBasisGradients */

/* ========================================================================== */

//
// -> yes, at least here I should use ThreeDeePolySpace-based routine (not to 
//    blindly reuse fit->space pointer);
//

void ThreeDeePolySpace::calculateFittingPolynomial(ThreeDeePolynomial *fit)
{
  for(int ii=0; ii<dim; ii++)
    fit->linear[ii] = polyProjection(basis[ii]);

  fit->convertLinearToCff();
} /*  ThreeDeePolySpace::calculateFittingPolynomial */

/* ========================================================================== */

//
//  -> yes, as any point-coupled routine this one should be ThreeDeePolySpace-related;
//

double ThreeDeePolySpace::getPolyFitChiSquare(ThreeDeePolynomial *fit)
{
  double ret = 0.0;

  for(int pt=0; pt<point_num; pt++)
  {
    ThreeDeePolyPoint *point = points + pt;

    assert(0);
    //@@@ ret += point->weight*SQR(fit->value(point->xx) - point->f);
  } /*for*/

  return ret;
} /* ThreeDeePolySpace::getPolyFitChiSquare */

/* ========================================================================== */
/*  This calculation assumes that chi^2 is correct; in this case errors on    */
/* 'linear' coefficients are all equal '1.' and below calculation is Ok;      */

double ThreeDeePolySpace::getNaivePolyFitError(double xx[3])
{
  double err = 0.0;

  for(int ii=0; ii<dim; ii++)
    err += SQR(basis[ii]->value(xx));

  return sqrt(err);
} /* ThreeDeePolySpace::getNaivePolyFitError */

/* ========================================================================== */
/*  This calculation takes chi^2 into account and renormalizes naive error    */
/* estimate by sqrt(chi^2/ndf); this way error estimate will not depend on the*/
/* weight scaling factor; so for instance if errors in all points are more or */
/* less the same, they can be safely set to 1. and this function will provide */
/* best guess error estimate;                                                 */

//
// -> not sure this has ever been checked;
//

double ThreeDeePolySpace::getRealisticPolyFitError(ThreeDeePolynomial *fit, double xx[3])
{
  int ndf = getNDF();
  double err = 0.0, chi2 = getPolyFitChiSquare(fit);

  for(int ii=0; ii<dim; ii++)
    err += SQR(basis[ii]->value(xx));

  return sqrt(err*chi2/ndf);
} /* ThreeDeePolySpace::getRealisticPolyFitError */

/* ========================================================================== */
