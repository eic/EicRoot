/* -------------------------------------------------------------------------- */
/*  import.cc                                                                 */
/*                                                                            */
/*    'mgrid' package import routines.                                        */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include <ayk.h>
#include <Splitter.h>
#include <Mgrid.h>

// Coordinates will be rounded in convert_cell_list_to_mgrid()      
// (see source code) before doing anything else; well, in fact this 
// rounding only affects step[] calculation; 1um is Ok?;            
#define _COORD_ROUNDING_ (1E-4)

/* ========================================================================== */
/*  Later will have to rewrite this slightly, so that the whole file is read  */
/* in and 'cell' pointer is assigned appropriately by hand; this way array    */
/* may be used as a piece of shared memory;                                   */

Mgrid *import_field_map(char *file_name)
{
  FILE *fin;
  Mgrid *mgrid;

  // Check sizes of basic types; 
  if (!basic_types_match()) return NULL;

  // Of course file should be opened only once; 
  fin = fopen(file_name, "r");
  if (!fin) return NULL;

  // All the rest is done recursively; 
  mgrid = new Mgrid(fin);

  fclose(fin);

  // Yes, it can be zero; no sense to check; 
  return mgrid;
} /* import_field_map */

/* ========================================================================== */
/* Want them as functions, since they are explicitely used as extra arguments */
/* to import_ascii_map;                                                       */

static double tesla2kgs_fun(double val)
{
  return tesla2kgs(val);
} /* tesla2kgs_fun */

/* -------------------------------------------------------------------------- */

static double m2cm_fun(double val)
{
  return m2cm(val);
} /* m2cm_fun */

/* ========================================================================== */
/*   Well, this intermeaidte call is a small slowdown; don't care;            */

static double import_value(const char *string, conversionfun fun)
{
  double value = atof(string);

  if (fun) value = fun(value);

  return value;
} /* import_value */

/* ========================================================================== */
/*  'coord_num' is needed because for instance in a 4-column file there is no */
/* way to decide whether it's 1 component in XYZ coordinates or say RZ        */
/* component in RZ coordinates; 'component_num' is needed since like in       */
/* recoil MC file there is an extra (unused) column; no memory clean up on    */
/* error, sorry;                                                              */

void release_cell_list_ram(MgridCell *cells)
{
  MgridCell *cell = cells, *bff;

  while (cell)
  {
    bff = cell->next;

    free(cell);

    cell = bff;
  } /*while*/
} /* release_cell_list_ram */

/* ========================================================================== */
/*  This routine only cares to restore the correct order (ie XYZ/RFZ);        */
/* 2->3 mapping and stuff like this will be done in                           */
/* calculate_expansion_rules() later; routine is inefficient, don't care;     */

static void calculate_swapping_rules(t_ascii_coord *coord, int swap[])
{
  char buffer[coord->coord_num], bff;

  // Fill out intermediate array of characters to be ordered; 
  for(int ii=0; ii<coord->coord_num; ii++)
    buffer[ii] = coord->coord_names[ii];

  // Reorder buffer[] array; 
  for(int ii=0; ii<coord->coord_num-1; ii++)
    for(int ik=0; ik<coord->coord_num-1; ik++)
    {
      t_coord_name *left  = find_coord_by_name(coord->system_type, buffer[ik]); 
      t_coord_name *right = find_coord_by_name(coord->system_type, buffer[ik+1]); 
      // Should not happen, since  create_coord_system()      
      // succeeded few lines above on the same 'coord' frame; 
      assert(left && right);

      if (left->id > right->id)
      {
	bff          = buffer[ik];
	buffer[ik]   = buffer[ik+1];
	buffer[ik+1] = bff;
      } /*if*/
    } /*for..for*/

  // Now check out the correct order and assign swap[] array; 
  for(int ii=0; ii<coord->coord_num; ii++)
    for(int ik=0; ik<coord->coord_num; ik++)
    {
      if (coord->coord_names[ii] == buffer[ik])
      {
	swap[ii] = ik;
	break;
      } /*if*/
    } /*for..for*/
} /* calculate_swapping_rules */

/* ========================================================================== */
/*   NB: this procedure expects that only non-fake xx[] are put in cell       */
/* frames (and this is done in a proper sequence - for instance RZ coordinates*/
/* are put in this sequence and not as ZR); so all arrays here are compressed */
/* accordingly;                                                               */

static Mgrid *convert_cell_list_to_mgrid(MgridCell *cells, char *name, CoordSystem *coord, 
					 CoordSystem *field, int split_mode, unsigned cell_contents_bits)
{
  // Well, need to adapt to TVector3 and debug at some point;
  assert(0);
#if _APR2014_
  int idx, linear;
  MgridCell *cell = cells, *out;
  MgridDirection *fdir[coord->getCoordNum()];
  int dim[coord->getCoordNum()];
  double min[coord->getCoordNum()], max[coord->getCoordNum()];
  double next_to_min[coord->getCoordNum()], *xx, step[coord->getCoordNum()];

  Mgrid *mgrid;
  char chname[MGRID_NAME_LENGTH_MAX];
  CoordSystem chfield;
  double chmin[3], chmax[3];

  // Well, I guess I should require this?; 
  if (!(cell_contents_bits & _FIELD_COMPONENT_VALUES_)) return NULL;

  // Loop through all cells and figure out min/max points; 
  while (cell)
  {
      // Yes, to this moment mgrid->cell_xx_shift is not                 
      // known --> calculate manually; and assume that cells in the list 
      // do not contain anything but field and coordinate values;        
    xx = cell->B + field->getCoordNum();

      // Ignore fake directions in all loops; 
    for(unsigned ik=0; ik<coord->getCoordNum(); ik++)
    {  
        // Change limits if needed; 
      if (cell == cells || xx[ik] < min[ik]) min[ik] = xx[ik];
      if (cell == cells || xx[ik] > max[ik]) max[ik] = xx[ik];
    } /*for*/

    cell = cell->next;
  } /*while*/

    // Figure out grid step (cell size); loop once again and figure 
    // out next to min point; is it more efficient than reordering  
    // during first while?; hmm;                                    
  for(unsigned ik=0; ik<coord->getCoordNum(); ik++)
    next_to_min[ik] = max[ik];
  cell = cells;
  while (cell)
  {
    xx = cell->B + field->getCoordNum();
    
    for(unsigned ik=0; ik<coord->getCoordNum(); ik++)
        // if (xx[ik] < next_to_min[ik] && xx[ik] != min[ik]) 
      if (xx[ik] < next_to_min[ik] && (xx[ik] - min[ik]) > _COORD_ROUNDING_)
	next_to_min[ik] = xx[ik];

    cell = cell->next;
  } /*while*/
  for(unsigned ik=0; ik<coord->getCoordNum(); ik++)
  {
      // NB: step will be calculated once again in initialize_mgrid(); 
      // since there it will be (max-min)/dim, this minimizes impact   
      // of possible rounding errors in ASCII file had too short       
      // float format;                                                  
      // NB: this will FAIL if there is a gap between min and next_to_min of 
      // more than 1 cell; what a piece of shit this code is!;         
    step[ik] = next_to_min[ik] - min[ik];
    if (!step[ik]) step[ik] = 1.;

      // Shift min&max half-step outside (so  
      // that nodes are sitting cell centers;  
    min[ik] -= step[ik]/2.;
    max[ik] += step[ik]/2.;

      // Well, no checks that all points are really sitting on a grid 
      // with this step&dim; user should bother himself;              
    dim[ik] = (int)rint((max[ik] - min[ik])/step[ik]);
  } /*for*/

    // In case of a non-skewed grid create just a single mgrid; otherwise 
    // create a separate mgrid for each field component (even if 1);      
  if (split_mode == _SPLIT_ON_)
  {
      // Allocate and initialize mgrid HEAP header; 
    mgrid = create_mgrid_heap_header(name, _SUPERPOSITION_);
    if (!mgrid || mgrid->initializeAsHeap()) return NULL;
    
      // Initialize children (component) mgrids; 
    assert(field->getCoordNum() == 3);
    for(int ch=0; ch<3; ch++)
    {
        // Recalculate field coordinate system frame for 1 component; 
        // also here full 3D/XYZ case assumed;                        
      chfield = *field;
      chfield.setCoordNum(1);
      for(int ik=0; ik<3; ik++)
	if (ik != ch)
	  chfield.fake[ik] = 1;
      
        // Create direction frames; NB: all this works only   
        // in full 3D/XYZ case => no worries about remapping; 
      for(int ik=0; ik<3; ik++)
      {
	assert(!coord->fake[ik]);

	chmin[ik] = min[ik];
	chmax[ik] = max[ik];

	//fdir[ik] = create_mgrid_direction(dim[ik], min[ik], max[ik]);
	//if (!fdir[ik]) return NULL;
	fdir[ik] = new MgridDirection(dim[ik], min[ik], max[ik]);
      } /*for*/
      
      snprintf(chname, MGRID_NAME_LENGTH_MAX, "%s#%03d", mgrid->getName(), ch);
      {
	Mgrid *child = create_single_mgrid_header(chname, coord, &chfield, 
						  fdir, cell_contents_bits);
        // Do all the necessary initializations; '0': RAM cell size 
        // should match useful cell size;                           
	if (!child || child->initializeAsSingleMgrid(0)) return NULL;

	//if (attach_mgrid_to_heap(mgrid, child)) return NULL;
	if (child->attachToHeap(mgrid)) return NULL;
      }
    } /*for*/

      // Eventually loop through all the cells and allocate data; 
    cell = cells;
    while (cell)
    {
      xx = cell->B + field->getCoordNum();
    
      for(int ch=0; ch<3; ch++)
      {
	Mgrid *child = mgrid->children[ch];

	linear = child->compressedCoordToLinearAddr(xx);
	// Should not happen; be paranoid; 
	assert(linear != -1);
	out  = child->linearAddrToCellPtr(linear);

          // Copy necessary cell fractions only; 
	out->B[0] = cell->B[ch];

          // Properties are stored in a separate array; 
          // of course, nothing bad happened up to now; 
	//child->properties[linear] = _SAFE_CELL_;
	child->markCellAsSafe(linear);
      } /*for*/

      cell = cell->next;
    } /*while*/
  } 
  else
  {
      // Create direction frames; all 3 must be present!; NB: xx[]     
      // were in correct order --> just skip few (no swapping needed); 
    idx = 0;
    for(int ik=0; ik<3; ik++)
      if (coord->fake[ik])
	fdir[ik] = NULL;
      else
      {
	//fdir[ik] = create_mgrid_direction(dim[idx], min[idx], max[idx]);
	//if (!fdir[ik]) return NULL;
	fdir[ik] = new MgridDirection(dim[idx], min[idx], max[idx]);
	
	idx++;
      } /*if..for*/

    mgrid = create_single_mgrid_header(name, coord, field, fdir, cell_contents_bits);
      // Do all the necessary initializations; '0': RAM cell size 
      // should match useful cell size;                           
    if (!mgrid || mgrid->initializeAsSingleMgrid(0)) return NULL;

      // And eventually copy over useful fraction of    
      // temporary cell frames to their proper location; 
    cell = cells;
    while (cell)
    {
      xx = cell->B + field->getCoordNum();
    
      linear = mgrid->compressedCoordToLinearAddr(xx);
      // Should not happen; be paranoid; 
      assert(linear != -1);
      out  = mgrid->linearAddrToCellPtr(linear);

        // Copy necessary cell fractions only;              
        // assume that only field values need to be stored; 
      for(unsigned ik=0; ik<field->getCoordNum(); ik++)
	out->B[ik] = cell->B[ik];
        // memcpy(out, cell, mgrid->ram_cell_size); 

        // Properties are stored in a separate array; 
        // of course, nothing bad happened up to now; 
      //mgrid->properties[linear] = _SAFE_CELL_;
      mgrid->markCellAsSafe(linear);

      cell = cell->next;
    } /*while*/
  } /*if*/    

  return mgrid;
#endif
} /* convert_cell_list_to_mgrid */

/* ========================================================================== */
/*   NB: this procedure puts only non-fake xx[] components in cell frames;    */
/* it takes care itself to (possibly) swap the coordinates and field          */
/* components so that they are in a proper /XYZ or RFZ/ sequence (for instance*/
/* if input file has colums in ZR sequence, cell->B[] will contain numbers in */
/* RZ sequence);                                                              */

//
// -> this routine has never been checked since Splitter class introduction;
//

Mgrid *import_ascii_field_map(char *file_name, char *mgrid_name, t_ascii_coord *coord, 
				t_ascii_coord *field, int lines_to_skip)
{
  int ik, cswap[3], fswap[3], line_counter = 0;
  //char buffer[STRING_LEN_MAX];
  FILE *fmap;
  int expected_column_num = coord->coord_num + field->coord_num;
  Mgrid *mgrid;
  MgridCell *cells = NULL, **tail = &cells, *cell;
  double *xx;
  CoordSystem *csystem, *fsystem;
  Splitter *splitter = new Splitter();

    // This is clear, I need enough storage space; otherwise 
    // please increase _CELL_DATA_DIM_MAX_ and recompile;    
  assert(_CELL_DATA_DIM_MAX_ >= expected_column_num);

    // Create and verify coordinate system frames; 
  csystem = new CoordSystem(coord->system_type, coord->coord_num, coord->coord_names);
  fsystem = new CoordSystem(field->system_type, field->coord_num, field->coord_names);
  if (!csystem || !fsystem) return NULL;

    // Calculate remapping (swapping) arrays; 
  calculate_swapping_rules(coord, cswap);
  calculate_swapping_rules(field, fswap);

  fmap = fopen(file_name, "r");
  if (!fmap) return NULL;

    // Prefer to import everything, and only after this start 
    // handling data;                                         
  //while (fgets(buffer, STRING_LEN_MAX-1, fmap))
  while (splitter->splitNextString(fmap) >= 0)
  {
    line_counter++;
    if (line_counter <= lines_to_skip) continue;

    //if (split_string(buffer)) continue;
    
      // Well, may be MORE columns (like in recoil MC map); 
    if (splitter->getArgn() < expected_column_num) return NULL;

      // Allocate new cell; 
    cell = *tail = (MgridCell*)calloc(1, sizeof(MgridCell));
    if (!cell) return NULL;

      // NB: want field components first in cell->B[]; 
    for(ik=0; ik<field->coord_num; ik++)
      cell->B[fswap[ik]] = 
	import_value(splitter->getArgp(coord->coord_num+ik), field->convfun);
      // For a better readability; 
    xx = cell->B + field->coord_num;
    for(ik=0; ik<coord->coord_num; ik++)
      xx[cswap[ik]] = import_value(splitter->getArgp(ik), coord->convfun);

    tail = &cell->next;
  } /*while*/

  fclose(fmap);

    // Now when all the lines are read in and are sitting in a linked   
    // list, call a general routine which works on this list; this      
    // routine can be called after one does a custom format file import; 
  mgrid = convert_cell_list_to_mgrid(cells, mgrid_name, csystem, fsystem, 
				     _SPLIT_OFF_, _FIELD_COMPONENT_VALUES_);

  if (mgrid) mgrid->setCreationMethod(_ASCII_INPUT_);

    // Release linked list memory; 
  release_cell_list_ram(cells);

  delete splitter;

  return mgrid;
} /* import_ascii_field_map */

/* ========================================================================== */
