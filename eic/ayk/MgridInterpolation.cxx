/* -------------------------------------------------------------------------- */
/*  MgridDirection.cc                                                         */
/*                                                                            */
/*    Magnetic field map interpolation routines.                              */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>
#include <cassert>
#include <cstring>

#include <ayk.h>
#include <Mgrid.h>

/* ========================================================================== */
/*  Well, it is convenient to have an option to calculate the same point with */
/* different interpolation techniques; however switching should be fast; so   */
/* it makes sense to cook few interpolation frames for a particular grid once */
/* and then simply switch pointers;                                           */

MgridInterpolation::MgridInterpolation(Mgrid *mgrid)
{
  memset(this, 0x00, sizeof(MgridInterpolation));

  // Well, procedure makes no sense for a collection of grids; 
  assert (mgrid->__type != _MGRID_HEAP_);

  _mgrid = mgrid;

  // Per default only normal and edge cells used; 
  allowed_self_cells_mask      = _SAFE_CELL_ | _EDGE_CELL_;
  allowed_neighbour_cells_mask = _SAFE_CELL_ | _EDGE_CELL_;

  // May move cube (at least away from grid border);        
  // more complicated positioning may be implemented later; 
  cube_shift_allowed = 1;

  // This is true for most of the modes; 
  ideal_cube_required = 1;
} /* MgridInterpolation::MgridInterpolation */

/* ========================================================================== */
/*  Per default only _NORMAL_CELL_ cells used; may be changed by this routine;*/

int MgridInterpolation::setAllowedCells(unsigned char _allowed_self_cells_mask,
					unsigned char _allowed_neighbour_cells_mask)
{
  // Only 4 lower bits should be present; 
  if (_allowed_self_cells_mask & 0xF0 || _allowed_neighbour_cells_mask & 0xF0)
    return -1;

  // Of course no sense to use _DEAD_CELL_ as neighbours; 
  if (_allowed_neighbour_cells_mask & _DEAD_CELL_) return -1;

  // This is also clear; only 3dim orth polynomial modes may work 
  // with missing self cells;                                     
  if (mode != _MULTI_DIM_INTERPOLATION_ && mode != _MULTI_DIM_FITTING_ && 
      _allowed_self_cells_mask & _DEAD_CELL_) 
    return -1;

  allowed_self_cells_mask      = _allowed_self_cells_mask;
  allowed_neighbour_cells_mask = _allowed_neighbour_cells_mask;

  return 0;
} /* MgridInterpolation::setAllowedCells */

/* ========================================================================== */

int MgridInterpolation::allocateCellCubeMemory()
{
  // Allocate cube of appropriate size for cell pointers; 
  cube_cell_num = 1;
  for(int ik=0; ik<3; ik++)
      cube_cell_num *= adim[ik];

  if (cube) delete[] cube;
  cube = new MgridCell* [cube_cell_num];

  return 0;
} /* MgridInterpolation::allocateCellCubeMemory */

/* ========================================================================== */

void MgridInterpolation::calculateReversedSequence()
{
  for(int ik=0; ik<3; ik++)
    reversed[sequence[ik]] = ik;
} /* MgridInterpolation::calculateReversedSequence */

/* -------------------------------------------------------------------------- */
/*   Probably not really best to the moment; just ascending order in adim[];  */
/* the organization is a bit strange (I mean inter->sequence[] array will be  */
/* called once again) ...;                                                    */

int MgridInterpolation::assignBestSequence()
{
  // Default initialization; 
  for(int ii=0; ii<3; ii++)
    sequence[ii] = ii;

  // Trivial bubble sorting suffices; 
  for(int ii=0; ii<2; ii++)
    for(int ik=0; ik<2; ik++)
      if (adim[sequence[ik]] > adim[sequence[ik+1]])
      {
	int bff = sequence[ik];

	sequence[ik] = sequence[ik+1];
	sequence[ik+1] = bff;
      } /*if..for*/

  // And assign reversed mapping array as well; 
  calculateReversedSequence();

  return 0;
} /* MgridInterpolation::assignBestSequence */

/* -------------------------------------------------------------------------- */

int MgridInterpolation::assignSuggestedSequence(char _sequence[])
{
  CoordSystem *system = &_mgrid->coord;

  //
  // Sequential interpolation will alway proceed as a sequence     
  // in ALL 3 dimensions (even if say internal grid is 2dim only); 
  // I mean it is just 3 loops :-); for the missing dimensions     
  // 'adim' is set to 1; but they still need to be assigned;       
  //

  // This loop is along _sequence[] characters;   
  for(int ii=0; ii<3; ii++)
  {
    // Double counting check; 
    for(int ik=0; ik<ii; ik++)
      if (_sequence[ii] == _sequence[ik])
	return -1;

    t_coord_name *cname = find_coord_by_name(system->system_type, _sequence[ii]);
    // Bad coordinate name; 
    if (!cname) return -1;

    sequence[ii] = cname->id;
  } /*for*/

  // And assign reversed mapping array as well; 
  calculateReversedSequence();

  return 0;
} /* MgridInterpolation::assignSuggestedSequence */

/* ========================================================================== */

int MgridInterpolation::precookSequentialHighCommon(int _adim[], char _sequence[])
{
  // Check _sequence[] input; 
  if (_sequence)
  {
    if (assignSuggestedSequence(_sequence)) return -1;
  }
  else
    assignBestSequence();

  // Check _adim[] input; 
  for(int ik=0; ik<3; ik++)
  {
    if (_adim[ik] < 1 || _adim[ik] > _mgrid->dir[ik].dim) return -1;

    adim[ik] = _adim[ik];
  } /*for*/

  if (allocateCellCubeMemory()) return -1;

  return 0;
} /* MgridInterpolation::precookSequentialHighCommon */

/* -------------------------------------------------------------------------- */

int MgridInterpolation::postcookSequentialHighCommon()
{
  // And intialization, specific for this mode; 
  for(int ip=0; ip<3; ip++)
  {  
    // Only one of the dimensions will not be equal to '1' (see below); 
    int bff[3] = {1, 1, 1}, true_coord = sequence[ip];

    bff[true_coord] = pdim[true_coord];

    // NB: hspace[] is in interpolation sequence; 
    ThreeDeePolySpace *space = hspace[ip] = 
      new ThreeDeePolySpace(bff, adim[true_coord]);

    for(int iq=0; iq<adim[true_coord]; iq++)
    {
      ThreeDeePolyPoint *point = space->points + iq;

      // Other 2 coordinates do not matter; 
      point->xx[true_coord] = _mgrid->cell_center_coord[true_coord][iq];
    } /*for iq*/

    // Now when everything is prepared calculate basis; 
    space->buildOrthogonalPolynomials();

    hfit[ip] = new ThreeDeePolynomial(space);
  } /*for ip*/

  return 0;
} /* MgridInterpolation::postcookSequentialHighCommon */

/* ========================================================================== */
/* Common part for multi-dim interpolation&fitting; adim[] here is assumed in */
/* a natural XYZ (RFZ) sequence /since sequence[] paramater is not used/;     */
/* !! NB: all 3 elements in adim[] expected (so for RZ-grid to be fitted in   */
/* 4x4 squares qdim[]= {4, 1, 4});                                            */ 

int MgridInterpolation::preCookMultiDimCommon(int _adim[])
{
  // Check adim[] input; 
  for(int ik=0; ik<3; ik++)
  {
    if (_adim[ik] < 1 || _adim[ik] > _mgrid->dir[ik].dim) return -1;

    adim[ik] = _adim[ik];
  } /*for ik*/

  if (allocateCellCubeMemory()) return -1;

  return 0;
} /* MgridInterpolation::preCookMultiDimCommon */

/* -------------------------------------------------------------------------- */
/* Common part for multi-dim interpolation&fitting;                           */

int MgridInterpolation::postCookMultiDimCommon()
{
  int i012[3];

  // It is easy to define coordinates to the leftmost corner, build basis and 
  // then only change function values and shift xx[] accordingly;  
  gspace = new ThreeDeePolySpace(pdim, cube_cell_num);

  for(i012[0]=0; i012[0]<adim[0]; i012[0]++)
    for(i012[1]=0; i012[1]<adim[1]; i012[1]++)
      for(i012[2]=0; i012[2]<adim[2]; i012[2]++)
	{
	  ThreeDeePolyPoint *point = gspace->points + 
	    i012[0]*adim[_Y_]*adim[_Z_] + i012[1]*adim[_Z_] + i012[2];

	  // '3': will not hurt for missing dimensions as well;   
	  for(int ik=0; ik<3; ik++)
	    point->xx[ik] = _mgrid->cell_center_coord[ik][i012[ik]];
	} /*for..for*/

  // Now when everything is prepared calculate basis; indeed 
  // this is done only once assuming weight=1. in all points;
  gspace->buildOrthogonalPolynomials();

  // And allocate the same frame for a non-regular case; prefer a separate loop 
  // for clarity; 'weight=1' is now done in this constructor per default;   
  // cell coordinates are not known to this point => that's it; 
  irregular = new ThreeDeePolySpace(pdim, cube_cell_num);
      
  // Also want to intialize polynomial frames to be used for fitting;       
  // inter->gspace & inter->irregular have the same parameters => may       
  // have one set of fitting polynomials and use gspace for initialization; 
  gfit = new ThreeDeePolynomial(gspace);

  for(int ik=0; ik<3; ik++)
    grad[ik] = new ThreeDeePolynomial(gspace);

  // (Almost) any collection of cells will work;        
  // and it is unreasonable to shift the cube (means    
  // it is better to use all cells which are most close 
  // to the self one;                                     
  cube_shift_allowed = ideal_cube_required = 0;

  return 0;
} /* MgridInterpolation::postCookMultiDimCommon */

/* ========================================================================== */

int MgridInterpolation::fillOkArray(int left[3], int adim[3], int cmp, 
				    MgridCombiCell *ok_arr)
{
  int ok_counter = 0, bff[3], dim = adim[0]*adim[1]*adim[2];
  MgridCombiCell qcell;

  // All cells bad per default; 
  memset(ok_arr, 0x00, dim*sizeof(MgridCombiCell));

  // Check all cells of suggested cube and set '1' for those which pass all tests; 
  for(int i0=0; i0<adim[0]; i0++)
  {
    bff[0] = left[0] + i0;

    for(int i1=0; i1<adim[1]; i1++)
    {
      bff[1] = left[1] + i1;

      for(int i2=0; i2<adim[2]; i2++)
      {
	bff[2] = left[2] + i2;

	if (_mgrid->multiAddrToCombiCell(bff, &qcell) ||
	    // Extra checks if weight usage is foreseen; for directional (1dim) errors it 
	    // is sufficient to check that error along sweeping direction is non-zero;    
	    (force_1dim_weight_usage && 
	     !qcell.cell->B[_mgrid->cell_1derr_shift + cmp*_mgrid->coord.coord_num + 0]) ||
	    (force_3dim_weight_usage && !qcell.cell->B[_mgrid->cell_3derr_shift + cmp]))
	  continue;

	if (qcell.property == _DEAD_CELL_)                           continue;
	if (!(qcell.property & allowed_neighbour_cells_mask)) continue;

	ok_counter++;
	ok_arr[i0*adim[1]*adim[2] + i1*adim[2] + i2] = qcell;
      } /*for i2*/
    } /*for i1*/
  } /*for i0*/

  return ok_counter;
} /* MgridInterpolation::fillOkArray */

/* -------------------------------------------------------------------------- */
/*  Unify with th eoutput routine later, please;                              */ 

int MgridInterpolation::checkSmallCube(int gdim[3], MgridCombiCell *ok_arr, 
				       int shift[3])
{
  // 3D loop has adim[] indexing of course; 
  for(int i0=0; i0<adim[0]; i0++)
  {
    int i0s = i0 + shift[0];

    for(int i1=0; i1<adim[1]; i1++)
    {
      int i1s = i1 + shift[1];

      for(int i2=0; i2<adim[2]; i2++)
      {
	int i2s = i2 + shift[2];

	// Yes, ok_arr[][][] has gdim[] dimensions (and NOT necessarily adim[]!); 
	MgridCombiCell *qcell = ok_arr + i0s*gdim[1]*gdim[2] + i1s*gdim[2] + i2s;

	// Yes, if cell did not match the criteria, pointer is just NULL; 
	if (!qcell->cell) return -1;
      } /*for i2*/
    } /*for i1*/
  } /*for i0*/

  return 0;
} /* MgridInterpolation::checkSmallCube */

/* -------------------------------------------------------------------------- */

void MgridInterpolation::fillOutputCubeArray(int gdim[3], MgridCombiCell *ok_arr, 
					     int shift[3])
{
  // 3D loop has adim[] indexing of course; 
  for(int i0=0; i0<adim[0]; i0++)
  {
    int i0s = i0 + shift[0];

    for(int i1=0; i1<adim[1]; i1++)
    {
      int i1s = i1 + shift[1];

      for(int i2=0; i2<adim[2]; i2++)
      {
	int i2s = i2 + shift[2];

	// Yes, ok_arr[][][] has gdim[] dimensions (and NOT necessarily adim[]!); 
	MgridCombiCell *qcell = ok_arr + i0s*gdim[1]*gdim[2] + i1s*gdim[2] + i2s;

	// Yes, if cell did not match the criteria, pointer is just NULL; 
	if (!qcell->cell) continue;

	// Record USED neighbour cell status; eventually these 4 bits may  
	// contain a mixture of SAFE/EDGE/EXTRA;                           
	if (qcell->cell != _mgrid->self_qcell.cell)
	    _mgrid->last_field_status |= qcell->property << 4;

	// Put cell address into cube[] array and advance counter;  
	cube[cell_counter++] = qcell->cell;
      } /*for i2*/
    } /*for i1*/
  } /*for i0*/
} /* MgridInterpolation::fillOutputCubeArray */

/* -------------------------------------------------------------------------- */
/* In this case have to position a true KxMxN cube of cells around xx[]; first*/
/* figure out coordinates of the best possible 'leftmost' in all 3 directions */
/* cell; if all cells in this cube satisfy the conditions, take it; otherwise */
/* (and this was for whatever reason missing before May'2008) if ideal cube   */
/* was required (say for sequential interpolation) try ALL possible cubes     */
/* containing xx[] cell and choose the best one with full set of cells;       */
/* NB: software stopped working correctly for non-potato cell area required   */
/* for HERMES spectrometer magnet field map (septum plate gap!); only one     */
/* fixed option of ideal cube was tried out and since direction towards septum*/
/* plate was available this cube hit DEAD cell area and failed;               */

int MgridInterpolation::checkNeighbouringCubicArea(TVector3 &xx, int cmp)
{
  // '0/1' depending on whether requested cube has even (0) or odd (1) number 
  // of cells along this XYZ (RFZ) side; natural XYZ (RFZ) sequence;          
  unsigned char oddity[3]; 

  cell_counter = 0;

  // Otherwise cube was selected already, just reuse; 
  if (!_mgrid->repetition_flag)
  {
    // Calculate oddity[] and left[] arrays; need true left corner of the cube;  
    for(int ik=0; ik<3; ik++)
      if (_mgrid->coord.fake[ik])
      {
	// Save CPU time on fake directions;  
	oddity[ik]        = 1;
	left[ik]   = 0;
      }
      else
      {  
	MgridDirection *dir = _mgrid->dir + ik;

	oddity[ik] = adim[ik]%2;

	// Depends on whether 'size' was even or odd; 
	left[ik] = _mgrid->self[ik] - (adim[ik]-1)/2;

	// Even number of cells along this direction and xx[] is to the left 
	// of it's own cell center => move by 1 to the left;                   
	if (!oddity[ik] && xx[ik] < _mgrid->cell_center_coord[ik][_mgrid->self[ik]]) 
	  left[ik]--;
      } /*if..for ik*/

    for(int ik=0; ik<3; ik++)
    {
      if (_mgrid->coord.fake[ik]) continue;

      MgridDirection *dir = _mgrid->dir + ik;

      // Correct obvious screw up if possible (and allowed); 
      if (left[ik] < 0 ) 
      {
	_mgrid->last_field_status |= _CUBE_SHIFT_;
	if (cube_shift_allowed)
	  left[ik] = 0; 
	else
        {
	  if (ideal_cube_required) _FAILURE_(_mgrid, _CUBE_FAILURE_)
	} /*if*/
      } /*if*/
      if (left[ik] + adim[ik] - 1 >= dir->dim) 
      {
	_mgrid->last_field_status |= _CUBE_SHIFT_;
	
	if (cube_shift_allowed)
	{
	  left[ik] = dir->dim - adim[ik];
	    // Cube is too big?; 
	  if (left[ik] < 0) _FAILURE_(_mgrid, _CUBE_FAILURE_)
        } 
	else
	{
	  if (ideal_cube_required) _FAILURE_(_mgrid, _CUBE_FAILURE_)
	} /*if*/
      } /*if*/
    } /*for*/
  } /*if*/

    // Ok, so first try this suggested cube; 
  {
    // Create 3D cube and stupidly check ALL it's cells; 
    MgridCombiCell ok[adim[0]][adim[1]][adim[2]];
    int ok_counter = fillOkArray(left, adim, cmp, (MgridCombiCell*)ok);

    // If all cells fine or ideal cube is not required, go fill out 
    // output array based on this returned qcell[] array; 
    if (ok_counter == adim[0]*adim[1]*adim[2] ||
	!ideal_cube_required)
    {
        // Yes, no shifts, exact match; 
      int shift[3] = {0, 0, 0};

      fillOutputCubeArray(adim, (MgridCombiCell*)ok, shift);

      return 0;
    }
    else
    {
      // Ok, want to find ideal cube; clearly have to investigate  
      // at most 2n-1 shifts in all directions; 
      int gdim[3], gleft[3];

      for(int iq=0; iq<3; iq++)
      { 
	gdim[iq]  = adim[iq]*2 - 1;
	gleft[iq] = _mgrid->self[iq] - adim[iq] + 1;
      } /*for iq*/

      {
	MgridCombiCell gok[gdim[0]][gdim[1]][gdim[2]];

	fillOkArray(gleft, gdim, cmp, (MgridCombiCell*)gok);

	// Loop through all small cubes of adim[] size                 
	// inside this "big" cube; for now fall out as soon as the very first 
	// "complete" cube found; do it better later;                         
	for(int i0=0; i0<adim[0]; i0++)
	  for(int i1=0; i1<adim[1]; i1++)
	    for(int i2=0; i2<adim[2]; i2++)
	    {
	      int gshift[3] = {i0, i1, i2};

	      if (checkSmallCube(gdim, (MgridCombiCell*)gok, gshift))
		continue;

	      // Ok, "good" cube found; fill output array and return 0; 
	      for(int iq=0; iq<3; iq++)
		left[iq] = gleft[iq] + gshift[iq];

	      fillOutputCubeArray(gdim, (MgridCombiCell*)gok, gshift);
	      return 0;	      
	    } /*for i0..i2*/
      }

      // No luck --> return failure code;
      _FAILURE_(_mgrid, _CUBE_FAILURE_)     
    }
  }

  return 0;
} /* MgridInterpolation::checkNeighbouringCubicArea */

/* -------------------------------------------------------------------------- */
/*  This mode is used for multi-dim irregular case where inter->left[] is not */
/* needed any longer --> may destroy; NB: in principle this code may loop     */
/* infinitely - fix this later;                                               */

int MgridInterpolation::appendArbitraryNeighbouringCells(int cmp)
{
  int right[3], bff[3];
  MgridCombiCell qcell;
  
  // Figure out right side of the cube (remember: cube failed!); 
  for(int ik=0; ik<3; ik++)
    right[ik] = left[ik] + adim[ik] - 1;

  // Sequentially choose one of the 3 dimensions and loop through 
  // it's min/max values, always expanding by 1 (keeping other    
  // dimensions constant); this basically means that I always     
  // loop through possible cells on a 2-dim rectangle;            
  for(int xyz=_X_; ; xyz = (xyz+1)%3)
  {
    // If this is a fake direction, just skip it; 
    if (_mgrid->coord.fake[xyz] || extension_prohibited[xyz]) continue;

    MgridDirection *dir = _mgrid->dir + xyz;

    // 2 other directions; 
    int d1 = (xyz+1)%3, d2 = (xyz+2)%3;

    // If at the limit check whether it makes sense to deal with the other 
    // 2 directions; takes time but allows to avoid getting stuck; 
    if (left[xyz] <= 0 && right[xyz] >= dir->dim - 1)
    {
      // If extension in both other directions prohibited, I'm stuck; 
      if (extension_prohibited[d1] && extension_prohibited[d2])
	return -1;

      // Think once again later whether this logic is Ok; 
#if 1
      // If extension in one of the other directions prohibited and 
      // 3-d direction is exhausted, same story;                    
      if (extension_prohibited[d1] && 
	  left[d2] <= 0 && right[d2] >= _mgrid->dir[d2].dim - 1)
	return -1;
      if (extension_prohibited[d2] && 
	  left[d1] <= 0 && right[d1] >= _mgrid->dir[d1].dim - 1)
	return -1;

      // Eventually if both other directions are exhausted, return -1 as well; 
      if (left[d1] <= 0 && right[d1] >= _mgrid->dir[d1].dim - 1 && 
	  left[d2] <= 0 && right[d2] >= _mgrid->dir[d2].dim - 1)
	return -1;
#endif
    } /*if*/

    for(int side=0; side<=1; side++)
    {
      if (!side)
      {
	// At the very left edge or beyond --> can not move further  
	// --> try other side (or other direction);                       
	if (left[xyz] <= 0) continue;
	bff[xyz] = --left[xyz];
      } 
      else
      {
	// At the very right edge or beyond --> can not move further  
	// --> try other side (or other direction);             
	if (right[xyz] >= dir->dim - 1) continue;
	bff[xyz] = ++right[xyz];
      } /*if*/

        // 2-dim loop over other 2 directions; 
      for(bff[d1]=left[d1]; bff[d1]<=right[d1]; bff[d1]++)
	for(bff[d2]=left[d2]; bff[d2]<=right[d2]; bff[d2]++) 
	{
	  // No such cell, no worries; 
	  if (_mgrid->multiAddrToCombiCell(bff, &qcell)) continue;

	  // Extra check if weight usage is foreseen;  
	  if ((force_1dim_weight_usage && 
	       !qcell.cell->B[_mgrid->cell_1derr_shift + cmp*_mgrid->coord.coord_num + 0]) ||
	      (force_3dim_weight_usage && !qcell.cell->B[_mgrid->cell_3derr_shift + cmp]))
	    continue;

	  // Self cell was checked already -> skip this part; 
	  // in fact this can not ever happen (?);            
	  if (qcell.cell == _mgrid->self_qcell.cell ||
	      // This type of neighbour cells is not allowed; 
	      !(qcell.property & allowed_neighbour_cells_mask))
	    continue;

	  // Record USED neighbour cell status; eventually these 4 bits may 
	  // contain a mixture of SAFE/EDGE/EXTRA;                          
	  _mgrid->last_field_status |= qcell.property << 4;

	  // Put cell address into cube[] array and advance counter; 
	  cube[cell_counter++] = qcell.cell;
	  // All needed cells allocated -> return; 
	  if (cell_counter == cube_cell_num) return 0;
	} /*for..for*/
    } /*for*/
  } /*for*/
} /* MgridInterpolation::appendArbitraryNeighbouringCells */

/* -------------------------------------------------------------------------- */

int MgridInterpolation::prepairIrregularCase(int cmp)
{
  TVector3 xx;

  // Put in cell coordinates; 
  for(int ik=0; ik<cube_cell_num; ik++)
  {
    ThreeDeePolyPoint *point = irregular->points + ik;
    MgridCell *cell = cube[ik];

    if (_mgrid->cellPtrToCoord(cell, xx)) return -1;

    for(int iq=0; iq<3; iq++)
      point->xx[iq] = xx[iq];
	
    // Please close direct access to this flag setting and check in the setting 
    // routine that respective bit is actually present in mgrid cell structure;                                       
    if (force_3dim_weight_usage)
      point->weight = 1./SQR(cell->B[_mgrid->cell_3derr_shift+cmp]);
  } /*for*/

    // And rebuild basis polynomial vectors; 
  irregular->buildOrthogonalPolynomials();

  return 0;
} /* MgridInterpolation::prepairIrregularCase */

/* -------------------------------------------------------------------------- */

int Mgrid::findSelfCell(TVector3 &xx)
{
  // Find out 3dim xx[] indices in this grid;                     
  // 'repetition_flag': 'self' cell was assigned by hand already; 
  if (!repetition_flag)
  {
    if (coordToMultiAddr(xx, self)) 
      // Cell is out of grid at all --> definitely bad; 
      _FAILURE_(this, _CELL_OUT_OF_GRID_);

	// Figure out cell pointer; cell is out of grid at all 
	// (should not happen - see check above); 
	assert(!multiAddrToCombiCell(self, &self_qcell));
  } /*if*/

    // Record self cell status; 
  last_field_status |= self_qcell.property;
  
  return 0;
} /* Mgrid::findSelfCell */

/* ========================================================================== */
