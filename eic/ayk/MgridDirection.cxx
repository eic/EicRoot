/* -------------------------------------------------------------------------- */
/*  MgridDirection.cc                                                         */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cassert>
#include <cstring>

#include <MgridDirection.h>

/* ========================================================================== */

MgridDirection::MgridDirection(int _dim, double _min, double _max)
{
  memset(this, 0x00, sizeof(MgridDirection));

  // Few sanity checks; 
  assert(_dim > 0 && _min <= _max);

  dim = _dim;
  min = _min;
  max = _max;

  // Well, may be 0; 
  step = (max - min)/dim;
} /* MgridDirection::MgridDirection */

/* ========================================================================== */
