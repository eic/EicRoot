/* -------------------------------------------------------------------------- */
/*  ThreeDeePolynomial.cc                                                     */
/*                                                                            */
/*    3D polynomial basic routines.                                           */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>

// Yes, need ThreeDeePolySpace definition as well;
#include <ThreeDeePolySpace.h>

/* ========================================================================== */

ThreeDeePolynomial::ThreeDeePolynomial(ThreeDeePolySpace *_space)
{
  // Check later, whether this is really needed;
  memset(this, 0x00, sizeof(ThreeDeePolynomial));

  space = _space;

  linear = new double [space->dim]; 
  memset(linear, 0x00, space->dim*sizeof(double));

  cff = new double** [space->_max_power[_X_]];

  for(int ix=0; ix<space->_max_power[_X_]; ix++)
  {
    cff[ix] = new double* [space->_max_power[_Y_]];

    for(int iy=0; iy<space->_max_power[_Y_]; iy++)
    {
      cff[ix][iy] = new double [space->_max_power[_Z_]];
      memset(cff[ix][iy], 0x00, space->_max_power[_Z_]*sizeof(double));
    } /*for*/
  } /*for*/
} /* ThreeDeePolynomial::ThreeDeePolynomial */

/* ========================================================================== */

double ThreeDeePolynomial::value(double xx[3])
{
  double ret = 0.0, argx[space->_max_power[_X_]];
  double argy[space->_max_power[_Y_]], argz[space->_max_power[_Z_]];

  if (off) return 0.0;

    /* Well, I was not able to make it looking better; */
    /* pow() is very slow on the other hand;           */
  argx[0] = argy[0] = argz[0] = 1.;
  for(int ik=1; ik<space->_max_power[_X_]; ik++)
    argx[ik] = argx[ik-1]*xx[_X_];
  for(int ik=1; ik<space->_max_power[_Y_]; ik++)
    argy[ik] = argy[ik-1]*xx[_Y_];
  for(int ik=1; ik<space->_max_power[_Z_]; ik++)
    argz[ik] = argz[ik-1]*xx[_Z_];

  for(int ix=0; ix<space->_max_power[_X_]; ix++)
  {
    if (!PARITY_CHECK(space, _X_, ix)) continue;

    for(int iy=0; iy<space->_max_power[_Y_]; iy++)
    {
      if (!PARITY_CHECK(space, _Y_, iy)) continue;
      
      for(int iz=0; iz<space->_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(space, _Z_, iz)) continue;
	
	ret += cff[ix][iy][iz]*argx[ix]*argy[iy]*argz[iz];
      } /*for iz*/
    } /*for iy*/
  } /*for ix*/

  return ret;
} /* ThreeDeePolynomial::value */

/* ========================================================================== */

double ThreeDeePolynomial::linearValue(double xx[3])
{
  double ret = 0.0;

  if (off) return 0.;

  for(int ik=0; ik<space->dim; ik++)
    ret += linear[ik]*space->basis[ik]->value(xx);

  return ret;
} /* ThreeDeePolynomial::linearValue */

/* ========================================================================== */

int ThreeDeePolynomial::normalize()
{
  // Looks ugly, I agree;
  double norm = sqrtl(space->polyProduct(this, this));

  if (!norm || (space->norm_cutoff && norm < space->norm_cutoff))
  {
    for(int ix=0; ix<space->_max_power[_X_]; ix++)
      for(int iy=0; iy<space->_max_power[_Y_]; iy++)
	for(int iz=0; iz<space->_max_power[_Z_]; iz++)
	  cff[ix][iy][iz] = 0.;

    off = 1;

    return -1;
  } /*if*/

  for(int ix=0; ix<space->_max_power[_X_]; ix++)
  {
    if (!PARITY_CHECK(space, _X_, ix)) continue;

    for(int iy=0; iy<space->_max_power[_Y_]; iy++)
    {
      if (!PARITY_CHECK(space, _Y_, iy)) continue;

      for(int iz=0; iz<space->_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(space, _Z_, iz)) continue;

	cff[ix][iy][iz] /= norm;
      } /*for iz*/
    } /*for iy*/
  } /*for ix*/

  return 0;
} /* ThreeDeePolynomial::normalize */

/* -------------------------------------------------------------------------- */

void ThreeDeePolynomial::multiply(double _cff)
{
  for(int ix=0; ix<space->_max_power[_X_]; ix++)
  {
    if (!PARITY_CHECK(space, _X_, ix)) continue;

    for(int iy=0; iy<space->_max_power[_Y_]; iy++)
    {
      if (!PARITY_CHECK(space, _Y_, iy)) continue;

      for(int iz=0; iz<space->_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(space, _Z_, iz)) continue;

	cff[ix][iy][iz] *= _cff;
      } /*for iz*/
    } /*for iy*/
  } /*for ix*/
} /* ThreeDeePolynomial::multiply */

/* -------------------------------------------------------------------------- */

void ThreeDeePolynomial::increment(ThreeDeePolynomial *incr)
{
  for(int ix=0; ix<space->_max_power[_X_]; ix++)
  {
    if (!PARITY_CHECK(space, _X_, ix)) continue;

    for(int iy=0; iy<space->_max_power[_Y_]; iy++)
    {
      if (!PARITY_CHECK(space, _Y_, iy)) continue;

      for(int iz=0; iz<space->_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(space, _Z_, iz)) continue;

	cff[ix][iy][iz] += incr->cff[ix][iy][iz];
      } /*for iz*/
    } /*for iy*/
  } /*for ix*/
} /* ThreeDeePolynomial::increment */

/* ========================================================================== */

void ThreeDeePolynomial::convertLinearToCff()
{
  for(int ix=0; ix<space->_max_power[_X_]; ix++)
  {
    if (!PARITY_CHECK(space, _X_, ix)) continue;

    for(int iy=0; iy<space->_max_power[_Y_]; iy++)
    {
      if (!PARITY_CHECK(space, _Y_, iy)) continue;

      for(int iz=0; iz<space->_max_power[_Z_]; iz++)
      {
	if (!PARITY_CHECK(space, _Z_, iz)) continue;

	cff[ix][iy][iz] = 0.;
	
	// Or should I rather use poly_multiply() & poly_add()?; 
	for(int ii=0; ii<space->dim; ii++)
	  cff[ix][iy][iz] += linear[ii]*space->basis[ii]->cff[ix][iy][iz];
      } /*for iz*/
    } /*for iy*/
  } /*for ix*/
} /* ThreeDeePolynomial::convertLinearToCff */

/* ========================================================================== */
/*  Has this ever been checked?..;                                            */

int ThreeDeePolynomial::calculateGradient(ThreeDeePolynomial *gradient[3])
{
  for(int ik=0; ik<3; ik++)
  {
    ThreeDeePolynomial *comp = gradient[ik];

    for(int ix=0; ix<space->_max_power[_X_]; ix++)
    {
      if (!PARITY_CHECK(space, _X_, ix)) continue;

      for(int iy=0; iy<space->_max_power[_Y_]; iy++)
      {
	if (!PARITY_CHECK(space, _Y_, iy)) continue;

	for(int iz=0; iz<space->_max_power[_Z_]; iz++)
        {
	  if (!PARITY_CHECK(space, _Z_, iz)) continue;

	  if (ik == _X_ && ix >= 1) comp->cff[ix-1][iy  ][iz  ] = ix*cff[ix][iy][iz];
	  if (ik == _Y_ && iy >= 1) comp->cff[ix  ][iy-1][iz  ] = iy*cff[ix][iy][iz];
	  if (ik == _Z_ && iz >= 1) comp->cff[ix  ][iy  ][iz-1] = iz*cff[ix][iy][iz];
	} /*for iz*/
      } /*for iy*/
    } /*for ix*/
  } /*for ik*/

  return 0;
} /* ThreeDeePolynomial::calculateGradient */

/* ========================================================================== */
