/* -------------------------------------------------------------------------- */
/*  CoordSystem.cc                                                            */
/*                                                                            */
/*    Coordinate system handling service routines.                            */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cassert>
#include <cstdlib>

#include <CoordSystem.h>

/* ========================================================================== */
/*  Well, it looks reasonable to establish a standalone routine which shapes  */
/* up the coordinate system frame;                                            */

CoordSystem::CoordSystem(unsigned char _system_type, int _coord_num, char 
			 _coord_names[])
{
  // Only 2 types are known for now; 
  assert(_system_type == _CARTESIAN_ || _system_type == _CYLINDRICAL_);
  system_type = _system_type;

  // Well, do not want to bother with >3 case; 
  assert(_coord_num > 0 && _coord_num <= 3);// return NULL;
  coord_num = _coord_num;

  if (!_coord_names)
  {
    assert(coord_num == 3);
  }
  else
  {
    // Set fake[] flags per default; 
    for(int ip=0; ip<3; ip++)
      fake[ip] = 1;

    // Check that there is no overlapping; 
    for(int ip=0; ip<coord_num; ip++)
      for(int iq=0; iq<ip; iq++)
	assert(_coord_names[ip] != _coord_names[iq]);

    // Use _coord_names[] array and find proper element; 
    for(int ip=0; ip<coord_num; ip++)
    {
      t_coord_name *cname = find_coord_by_name(system_type, _coord_names[ip]);
      // Coordinate name is unknown; 
      assert(cname);

      // True coordinate --> reset 'fake' flag; 
      fake[cname->id] = 0;
    } /*for*/
  } /*if*/
} /* CoordSystem::CoordSystem */

/* ========================================================================== */
/*  This procedure assumes that compressed coordinates are in a right order,  */
/* but few of them may be missing (say 2 coordinates can NOT be 'ZR', while   */
/* 'RZ' is possible);                                                         */

int CoordSystem::calculateExpansionRules(int **expansion)
{
  int idx = 0;

  // If all 3 coordinates are used, no expansion needed; but reset pointer to NULL;
  if (coord_num == 3) 
  {
    *expansion = 0;
    return 0;
  } /*if*/

  int *ptr = *expansion = (int*)malloc(3*sizeof(int));
  if (!ptr) return -1;

  for(int ip=0; ip<3; ip++)
    if (fake[ip]) 
      ptr[ip] = -1;
    else
      ptr[ip] = idx++;

  return 0;
} /* CoordSystem::calculateExpansionRules */

/* ========================================================================== */

//
// -> Has not been checked?;
//

void CoordSystem::projectToLocalCoordinates(double in[], double out[])
{
  int idx= 0;
  double buffer[3], *ptr;

  // Save all 3 components if in[] and out[] is the same pointer; 
  // again, no overlap check :-);                                 
  if (in == out)
  {
    for(int ik=0; ik<3; ik++)
      buffer[ik] = in[ik];

    ptr = buffer;
  }
  else
    ptr = in;

  // Calculate all projected components; 
  for(int ik=0; ik<3; ik++)
  {
    if (fake[ik]) continue;

    out[idx++] = ptr[ik];
  } /*for*/
} /* CoordSystem::projectToLocalCoordinates */

/* ========================================================================== */

void expand_to_global_coordinates(int ldim, double in[], double out[], int l2g[])
{
  double buffer[3], *ptr;

  // Save all local components if in[] and out[] are the same; 
  // well, no overlap check :-);                               
  if (in == out)
  {
    for(int ik=0; ik<ldim; ik++)
      buffer[ik] = in[ik];

    ptr = buffer;
  }
  else
    ptr = in;

  // Calculate all 3 components; 
  for(int ik=0; ik<3; ik++)
    out[ik] = (l2g[ik] == -1) ? 0. : ptr[l2g[ik]];
} /* expand_to_global_coordinates */

/* ========================================================================== */
