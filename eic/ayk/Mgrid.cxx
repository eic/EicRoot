/* -------------------------------------------------------------------------- */
/*  Mgrid.cc                                                                  */
/*                                                                            */
/*    Magnetic field map handling routines.                                   */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <ayk.h>
#include <Mgrid.h>
                               
/* ========================================================================== */

//
//  -> Constructors are private; user interface is done via external functions;
//

Mgrid::Mgrid(char *_name, MgridType _type)
{
  // Put magic sign and format ID in; 
  magic_header  = _MFILE_MAGIC_;
  format_id     = _MGRID_FORMAT_ID_;
  creation_time = time(0);

  __type        = _type;

  magic_trailer = _MFILE_MAGIC_;

  // Assign mgrid name; 
  snprintf(name, MGRID_NAME_LENGTH_MAX-1, "%s", _name);

  shift[0] = shift[1] = shift[2] = 0.0;
  // Assign rotations in a way which means 'no rotation';    
  theta[0] = 90.; theta[1] = 90.; theta[2] =  0.;
  // phi[2] is of no importance if theta[2]=0.; 
  phi  [0] =  0.; phi  [1] = 90.;

  position = NULL;

  repetition_flag = 0;
} /* Mgrid::Mgrid */

/* -------------------------------------------------------------------------- */

//
// -> error handling is needed; 
//    also handle shift[] and repetition_flag init?;
//

Mgrid::Mgrid(FILE *fin)
{
  memset(this, 0x00, sizeof(Mgrid));

  // Read in the header; 
  if (fread(this, sizeof(MgridHeader), 1, fin) != 1) return; // NULL;
  // Check magic word; 
  if (magic_header != _MFILE_MAGIC_ || magic_trailer != _MFILE_MAGIC_) 
    return; //NULL;

  if (__type == _MGRID_HEAP_)
  {
    // Perform initialization first of all; 
    if (initializeAsHeap()) return; // NULL;

    // Loop through all the children; recursion;
    // will this actually work in constructor?;
    for(int ch=0; ch<child_num; ch++)
    {
      //children[ch] = import_mgrid(fin);
      children[ch] = new Mgrid(fin);
      //if (!children[ch]) return NULL;
    } /*for*/
  }
  else
  {
    // Perform initialization first of all; 
    if (initializeAsSingleMgrid(0)) return; // NULL;

    // Read in mgrid cell data; NB: block read possible since 
    // initialize_mgrid(mgrid, 0) was called before (so that  
    // ram_cell_size=useful_cell_size);                       
    if (object_contents_bits & _CELL_DATA_ &&
        fread(cells, useful_cell_size, cell_num, fin) != cell_num)
      return; // NULL;

    if (object_contents_bits & _CELL_PROPERTIES_)
    {
      int tail = suggestedPropertyArrayTail();

      if (fread(properties, sizeof(char), cell_num, fin) != cell_num ||
	  (tail && fread(properties, sizeof(char), tail, fin) != tail))
	return; // NULL;
    } 
    else
      // Best guess values; 
      for(int ik=0; ik<cell_num; ik++)
	properties[ik] = _SAFE_CELL_;
  } /*if*/
} /* Mgrid::Mgrid */

/* ========================================================================== */
/*   calloc() call + initialize what is possible to default values; of course */
/* if this t_mgrid frame is used to store input grid, everything will be      */
/* overwritten by fread();                                                    */

//
// -> header creation is different for mgrid heap and single mgrid;
//

Mgrid *create_single_mgrid_header(char *name, CoordSystem *coord, 
  CoordSystem *field, MgridDirection *dir[3], unsigned cell_contents_bits)
{
  Mgrid *mgrid = new Mgrid(name, _RECTANGULAR_MGRID_);

  // Copy over structures describing coordinates and field components; 
  mgrid->coord = *coord;
  mgrid->field = *field;

  // Assign direction descriptors; NB: all 3 must be present  
  // in dir[3] on input, and in a natural XYZ (RFZ) sequence; 
  for(int ik=0; ik<3; ik++)
    if (mgrid->coord.fake[ik])
    {
      // Force predefined situation in this case; check that provided pointer 
      // is really NULL (in order to avoid any ambiguities in input parameters); 
      if (dir[ik]) 
      {
	printf("[create_single_mgrid_header()]: dir[] pointer should be NULL"
	       " for fake coordinates\n");
	return NULL;
      } /*if*/

      mgrid->dir[ik] = MgridDirection(1, 0.0, 0.0);
    }
    else
    {
      // Full frame copy; 
      mgrid->dir[ik] = *dir[ik];

      // For non-fake dimensions step should be non-zero; 
      //if (!mgrid->dir[ik].step) return NULL;
      if (!mgrid->dir[ik].getStep()) return NULL;
    } /*if..for*/

  // 5 bits known for now; no problem to expand later; 
  if (cell_contents_bits >> _MAX_KNOWN_CELL_CONTENTS_BIT_) return NULL;
  mgrid->cell_contents_bits = cell_contents_bits;

  return mgrid;
} /* create_single_mgrid_header */

/* -------------------------------------------------------------------------- */

Mgrid *create_mgrid_heap_header(char *name, int field_calculation_method)
{
  Mgrid *mgrid = new Mgrid(name, _MGRID_HEAP_);

  mgrid->field_calculation_method = field_calculation_method;

  // Per default it is reasonable to assign _CARTESIAN_ initialization;  
  // just bacause if positioning is wanted, _CYLINDRICAL_ case would     
  // require additional transformation;                                  
  // I means these could be 2 additional parameters, but normally nobody 
  // should care about them; will be used in conversion routines;        
  //mgrid->coord.system_type = mgrid->field.system_type = _CARTESIAN_;    
  mgrid->coord.setSystemType(_CARTESIAN_);
  mgrid->field.setSystemType(_CARTESIAN_);

  return mgrid;
} /* create_mgrid_heap_header */

/* ========================================================================== */

int Mgrid::initializeAsHeap()
{
  // Position calculation is needed in both HEAP/SINGLE cases; 
  if (recalculatePosition()) return -1;

  if (child_num)
  {
    children = (Mgrid**)calloc(child_num, sizeof(Mgrid*));
    if (!children) return -1;
  } /*if*/

  scaling_factor = 1.;

  return 0;
} /* Mgrid::initializeAsHeap */

/* -------------------------------------------------------------------------- */

int Mgrid::initializeAsSingleMgrid(int _ram_cell_size)
{
  // Position calculation is needed in both HEAP/SINGLE cases; 
  if (recalculatePosition()) return -1;

  // Initialize as a single grid; calculate useful cell size first; 
  if (cell_contents_bits & _FIELD_COMPONENT_VALUES_)
    cell_double_word_num = field.coord_num;
  else
    // Well, I guess something useful must be stored?; 
    return -1;

  cell_ferr_shift =  cell_double_word_num;
  if (cell_contents_bits & _FIELD_COMPONENT_ERRORS_)
    cell_double_word_num += field.coord_num;
  cell_fchi_shift =  cell_double_word_num;
  if (cell_contents_bits & _FIT_CHI_SQUARE_)
    cell_double_word_num += field.coord_num;

  // Calculate possible gradient[][] array shift in cell frame;  
  cell_grad_shift =  cell_double_word_num;
  if (cell_contents_bits & _FIELD_GRADIENTS_)
    cell_double_word_num += coord.coord_num*field.coord_num;

  // Calculate possible swerr[] shift in cell frame;  
  cell_1derr_shift =  cell_double_word_num;
  if (cell_contents_bits & _1D_FIELD_ERRORS_)
    cell_double_word_num += coord.coord_num*field.coord_num;

  cell_3derr_shift =  cell_double_word_num;
  if (cell_contents_bits & _3D_FIELD_ERRORS_)
    cell_double_word_num += field.coord_num;

  // Calculate possible xx[] shift in cell frame; 
  cell_xx_shift =  cell_double_word_num;
  if (cell_contents_bits & _CELL_COORDINATES_)
    cell_double_word_num += coord.coord_num;

  // Change _CELL_DATA_DIM_ and recompile if needed; 
  assert(cell_double_word_num <= _CELL_DATA_DIM_MAX_);
  useful_cell_size = cell_double_word_num*sizeof(double);

  // Now check suggested RAM-stored cell size; 
  if (!_ram_cell_size)
    ram_cell_size = useful_cell_size;
  else
  {
    if (_ram_cell_size <= 0 || _ram_cell_size < useful_cell_size || 
	_ram_cell_size > sizeof(MgridCell)) 
      return -1;

    ram_cell_size = _ram_cell_size;
  } /*if*/

    // Calculate coordinate/field mapping rules to convert       
    // possibly '<3'-dim coordinated and fields into 3dim space; 
  coord.calculateExpansionRules(&cexp);
  field.calculateExpansionRules(&fexp);

  cell_num = 1;
  for(int ik=0; ik<3; ik++)
    cell_num *= dir[ik].dim;
  // Calculation of cell centers is needed even for the skewed case; 
  for(int ik=0; ik<3; ik++)
  {
    MgridDirection *pdir = dir + ik; 

    // So numbering in this array matches internal grid 
    // numbering (not interpolation one!);               
    cell_center_coord[ik] = (double*)malloc(pdir->dim*sizeof(double));
    if (!cell_center_coord[ik]) return -1;
    
    for(int ip=0; ip<pdir->dim; ip++)
      cell_center_coord[ik][ip] = pdir->min + pdir->step*(ip + 0.5);
  } /*for*/

    // Then do the actual allocation; 
  cells      = (unsigned char*)calloc(cell_num, ram_cell_size);
  properties = (unsigned char*)calloc(cell_num, sizeof(char));
  if (!cells || !properties) return -1;
    /* Do not mind to reset properties to _DEAD_CELL_ per default; */
  for(int ik=0; ik<cell_num; ik++)
    properties[ik] = _DEAD_CELL_;

  // And eventually attach default interpolation mode (wwc's 4x4x4       
  // weighted interpolation in all non-fake directions); NB: it is NOT   
  // the default HRC interpolation mode (which is qudratic interpolation 
  // along Y + weighted qudratic interpolation along X + quadratic       
  // interpolation along Z in a 4x3x3 cube;   
  { 
    int adim[3];                          

    for(int ik=0; ik<3; ik++)
      if (coord.fake[ik])
	adim[ik] = 1;
      else
      {
	adim[ik] = coord.fake[ik] ? 1 : 4;
	if (adim[ik] > dir[ik].dim) adim[ik] =  dir[ik].dim;
      } /*if..for*/
    if (setInterpolationMode(_SEQUENTIAL_FAST_INTERPOLATION_, adim))
      return -1;
  }

  scaling_factor = 1.;

  return 0;
} /* Mgrid::initializeAsSingleMgrid */

/* ========================================================================== */
/*   User-level gateway functions; it is assumed that users at most want to   */
/* change default interpolation mode once, without any extra complications;   */
/* all the rest requires inclusion of mgsystem.h and usage of system-level    */
/* routines like cook_interpolation() directly; 2 separate routines for       */
/* interpolation and fitting look redundant, but will probably help users to  */
/* control better what they are doing;                                        */

int Mgrid::setModeWrapper(unsigned mode, int adim[], int pdim[], char sequence[])
{
  MgridInterpolation *inter;

  if (__type == _MGRID_HEAP_) return -1;

  // Create and assign this mode for all field components; 
  for(int ik=0; ik<field.coord_num; ik++)
  {
    inter = cookInterpolation(mode, adim, pdim, sequence);
    if (!inter) return -1;

    // No sense to call switch_interpolation(), just do it; 
    saved_interpolation[ik] = interpolation[ik];
    interpolation      [ik] = inter;
  } /*for*/

  return 0;
} /* setModeWrapper */

/* -------------------------------------------------------------------------- */

int Mgrid::setInterpolationMode(unsigned mode, int adim[])
{
  if (__type == _MGRID_HEAP_) return -1;

  if (mode != _SEQUENTIAL_FAST_INTERPOLATION_ &&
      mode != _SEQUENTIAL_HIGH_INTERPOLATION_ &&
      mode != _MULTI_DIM_INTERPOLATION_)
    return -1;

  return setModeWrapper(mode, adim, NULL, NULL);
} /* Mgrid::setInterpolationMode */

/* -------------------------------------------------------------------------- */

int Mgrid::setHrcInterpolationMode()
{
  // NB: adim[] is per definition in a natural XYZ sequence;  
  int adim[3] = {4, 3, 3};
  char sequence[3] = {'Y', 'X', 'Z'};

  if (__type == _MGRID_HEAP_) return -1;

  return setModeWrapper(_SEQUENTIAL_FAST_INTERPOLATION_, adim, NULL, sequence);
} /* Mgrid::setHrcInterpolationMode */

/* -------------------------------------------------------------------------- */

int Mgrid::setHtcInterpolationMode(t_htc_interpolation *htci)
{
  if (/*!mgrid ||*/ !htci) return -1;

  switch (htci->mode)
  {
  case _MODE_OFF_:
    return turnInterpolationOff();
  case _MODE_HRC_:
    return setHrcInterpolationMode();
  case _MODE_ADIM_:
    //return setInterpolationMode(_SEQUENTIAL_HIGH_INTERPOLATION_, htci->adim);
    return setInterpolationMode(_SEQUENTIAL_FAST_INTERPOLATION_, htci->adim);
  default:
    return -1;
  } /*switch*/
} /* Mgrid::setHtcInterpolationMode */

/* -------------------------------------------------------------------------- */

int Mgrid::setFittingMode(unsigned mode, int adim[], int pdim[])
{
  if (__type == _MGRID_HEAP_) return -1;

  if (mode != _SEQUENTIAL_HIGH_FITTING_ && mode != _MULTI_DIM_FITTING_)
    return -1;

  return setModeWrapper(mode, adim, pdim, NULL);
} /* Mgrid::setFittingMode */   

/* -------------------------------------------------------------------------- */

int Mgrid::turnInterpolationOff()
{
  if (__type == _MGRID_HEAP_) return -1;

  return setModeWrapper(_NO_INTERPOLATION_, NULL, NULL, NULL);
} /* Mgrid::turnInterpolationOff */             

/* ========================================================================== */

int Mgrid::switchInterpolation(char field_component_name, MgridInterpolation *inter)
{
  t_coord_name *cname = find_coord_by_name(field.system_type, field_component_name);
  int true_id = 0;

  if (!cname) return -1;

  // Check that this interpolation was really created for this mgrid; 
  if (this != inter->_mgrid) return -1;

  // Figure out 'compressed' id; 
  for(int ik=0; ik<3; ik++)
  {
    if (field.fake[ik]) continue;

    if (ik == cname->id) break;

    true_id++;
  } /*for*/

  saved_interpolation[true_id] = interpolation[true_id];
  interpolation      [true_id] = inter;

  return 0;
} /* Mgrid::switchInterpolation */

/* -------------------------------------------------------------------------- */
/*  No stack, just one-time 'pop' mode;                                       */

int Mgrid::restoreInterpolations()
{
  for(int ik=0; ik<field.coord_num; ik++)
  {
    if (!saved_interpolation[ik]) return -1;

    interpolation[ik]       = saved_interpolation[ik];
    saved_interpolation[ik] = NULL;
  } /*for*/

  return 0;
} /* Mgrid::restoreInterpolations */

/* ========================================================================== */

MgridInterpolation *Mgrid::cookInterpolationOff()
{
  MgridInterpolation *inter = new MgridInterpolation(this);

  inter->mode = _NO_INTERPOLATION_;

  return inter;
} /* Mgrid::cookInterpolationOff */

/* ========================================================================== */

#define quad1(x,xarr,yarr) \
  (d3 = ((x)-(xarr)[2])/((xarr)[1]-(xarr)[0]), \
   d1 = ((x)-(xarr)[1])/((xarr)[2]-(xarr)[0]), \
   d2 = ((x)-(xarr)[0])/((xarr)[2]-(xarr)[1]), \
   ( (yarr)[2]*d1*d2  + d3*((yarr)[0]*d1-(yarr)[1]*d2) ))

#define quad2(x,xarr,yarr) \
  (d6 = ((x)-(xarr)[2])/((xarr)[1]-(xarr)[0]), \
   d4 = ((x)-(xarr)[1])/((xarr)[2]-(xarr)[0]), \
   d5 = ((x)-(xarr)[0])/((xarr)[2]-(xarr)[1]), \
   ( (yarr)[2]*d4*d5  + d6*((yarr)[0]*d4-(yarr)[1]*d5) ))

static double quad3( double x, double xa[4], double ya[4])
{
  double w1,w2;
  double d1,d2,d3,d4,d5,d6;  /* for quad macros */
  if( xa[1] < xa[2] ) {
    w1 = x - xa[1];

    if( w1 < 0 ) 
      return quad1( x, xa, ya);
    
    w2 = xa[2] - x;
    if( w2 < 0 )
      return quad1( x, xa+1, ya+1);

    return (quad1( x, xa, ya) * w2 + quad2( x, xa+1, ya+1) * w1 )/(w1+w2);
  } else {
    w1 = xa[1] - x;
    if( w1 < 0 )
      return quad1( x, xa, ya);
    
    w2 = x - xa[2];
    if( w2 < 0 )
      return quad1( x, xa+1, ya+1);
    
    return (quad1( x, xa, ya) * w2 + quad2( x, xa+1, ya+1) * w1 )/(w1+w2);
  }
} /* quad3 */

/* -------------------------------------------------------------------------- */
/*  Fake interpolation routine; used in order to allow 3 loops during field   */
/* calculation even if the actual grid coordinate system has <3 dimensions;   */

static double trivial_assignment(double xx, double coord[1], double value[1])
{
  return value[0];
} /* trivial_assignment */

/* -------------------------------------------------------------------------- */
/* No check on coord[0] != coord[1] (should be done in the calling routine);  */

static double linear_interpolation(double xx, double coord[2], double value[2])
{
  double slope = (value[1] - value[0])/(coord[1] - coord[0]);

  return value[0] + slope*(xx - coord[0]);
} /* linear_interpolation */

/* -------------------------------------------------------------------------- */
/* A wrapper for wwc's quad1 macro;                                           */

static double quad_interpolation(double xx, double coord[3], double value[3])
{
  double d1, d2, d3; 

  return quad1(xx, coord, value);
} /* quad_interpolation */

/* -------------------------------------------------------------------------- */
/* sequence[] and adim[] arrays here and below: all 3 elements expected;      */
/* adim[]/pdim[] are always in a natural sequence;                            */

MgridInterpolation *Mgrid::cookSequentialFastInterpolation(int _adim[], 
							   char _sequence[])
{
  MgridInterpolation *inter = new MgridInterpolation(this);

  inter->mode = _SEQUENTIAL_FAST_INTERPOLATION_;

  // Check sequence[] input and assign inter->sequence[]; 
  if (_sequence)
  {
    if (inter->assignSuggestedSequence(_sequence)) return NULL;
  }
  else
    inter->assignBestSequence();

  // Check adim[] input; loop is in a natural sequence; 
  for(int ik=0; ik<3; ik++)
  {
    if (_adim[ik] < 1 || _adim[ik] > 4 ||
	// Well, mgrid->dir[ik].dim may be <4 :-) 
	_adim[ik] > this->dir[ik].dim)
      return NULL;

    // Remap right here (adim[] array is in a natural XYZ sequence); 
    inter->adim[ik] = _adim[ik];
  } /*for*/

  if (inter->allocateCellCubeMemory()) return NULL;

  // And intialization, specific for this mode; loop is along interpolation 
  // sequence!!!; NB: fun[] is also in interpolation sequence; 
  for(int ik=0; ik<3; ik++)
    switch(inter->adim[inter->sequence[ik]])
    {
      case 1: inter->fun[ik] = trivial_assignment;
	      break;
      case 2: inter->fun[ik] = linear_interpolation;
	      break;
      case 3: inter->fun[ik] = quad_interpolation;
	      break;
      case 4: inter->fun[ik] = quad3;
	      break;
    } /*for ik..switch*/

  for(int ik=0; ik<3; ik++)
  {
    //if (inter->v[ik]) free(inter->v[ik]);
    if (inter->v[ik]) delete[] inter->v[ik];

    // NB: v[] is also in interpolation sequence!; 
    inter->v[ik] = 
      //(double*)malloc(inter->adim[inter->sequence[ik]]*sizeof(double));
      new double [inter->adim[inter->sequence[ik]]];
    //if (!inter->v[ik]) return NULL;
  } /*for ik*/

  return inter;
} /* Mgrid::cookSequentialFastInterpolation */

/* ========================================================================== */

MgridInterpolation *Mgrid::cookSequentialHighInterpolation(int _adim[], 
							   char _sequence[])
{
  MgridInterpolation *inter = new MgridInterpolation(this);

  inter->mode = _SEQUENTIAL_HIGH_INTERPOLATION_;

  if (inter->precookSequentialHighCommon(_adim, _sequence)) return NULL;

  for(int ik=0; ik<3; ik++)
    inter->pdim[ik] = inter->adim[ik];

  if (inter->postcookSequentialHighCommon()) return NULL;

  return inter;
} /* Mgrid::cookSequentialHighInterpolation */

/* ========================================================================== */

MgridInterpolation *Mgrid::cookSequentialHighFitting(int _adim[], int _pdim[], 
						     char _sequence[])
{
  MgridInterpolation *inter = new MgridInterpolation(this);

  inter->mode = _SEQUENTIAL_HIGH_FITTING_;

  if (inter->precookSequentialHighCommon(_adim, _sequence)) return NULL;

  // Check pdim[] input; 
  for(int ik=0; ik<3; ik++)
  {
    if (_pdim[ik] < 1 || _pdim[ik] > dir[ik].dim ||
	(!coord.fake[ik] && _pdim[ik] >= _adim[ik]))  
      return NULL;

    inter->pdim[ik] = _pdim[ik];
  } /*for*/

  if (inter->postcookSequentialHighCommon()) return NULL;

  return inter;
} /* Mgrid::cookSequentialHighFitting */

/* ========================================================================== */
/*  This mode does not need any special sequence => drop this complication    */
/* and assume that both adim[] and pdim[] have natural grid indexing;         */

MgridInterpolation *Mgrid::cookMultiDimInterpolation(int adim[])
{
  MgridInterpolation *inter = new MgridInterpolation(this);

  inter->mode = _MULTI_DIM_INTERPOLATION_;

  if (inter->preCookMultiDimCommon(adim)) return NULL;

  // This is per definition for the interpolation mode; 
  for(int ik=0; ik<3; ik++)
    inter->pdim[ik] = inter->adim[ik];

  if (inter->postCookMultiDimCommon()) return NULL;

  return inter;
} /* Mgrid::cookMultiDimInterpolation */

/* ========================================================================== */
/* Of course pdim[] is expected in a natural XYZ (RFZ) sequence;              */
/* !! NB: all 3 elements in adim[]/pdim[] expected;                           */

MgridInterpolation *Mgrid::cookMultiDimFitting(int adim[], int pdim[])
{
  MgridInterpolation *inter = new MgridInterpolation(this);

  inter->mode = _MULTI_DIM_FITTING_;

  if (inter->preCookMultiDimCommon(adim)) return NULL;

  for(int ik=0; ik<3; ik++)
  {
    // '>=': hmm, in fact may want '>' (part of directions 
    // fitting and part of directions interpolation); yes!;
    if (pdim[ik] < 1 || pdim[ik] >/*=*/ inter->adim[ik]) return NULL;

    // pdim[] array is in interpolation sequence -> no remapping; 
    inter->pdim[ik] = pdim[ik];
  } /*for*/

  if (inter->postCookMultiDimCommon()) return NULL;

  return inter;
} /* Mgrid::cookMultiDimFitting */

/* ========================================================================== */
/*  Common gateways to interpolation creation routines;                       */

MgridInterpolation *Mgrid::cookInterpolation(unsigned mode, int adim[], 
					     int pdim[], char sequence[])
{
  switch (mode)
  {
    case _NO_INTERPOLATION_:
        /* Here and below: unused parameters must be strictly NULL, */
        /* in order to avoid misinterpretation;                     */
      if (adim || pdim || sequence) return NULL;
      return cookInterpolationOff();
    case _SEQUENTIAL_FAST_INTERPOLATION_:
      if (pdim) return NULL;
      return cookSequentialFastInterpolation(adim, sequence);
    case _SEQUENTIAL_HIGH_INTERPOLATION_:
      if (pdim) return NULL;
      return cookSequentialHighInterpolation(adim, sequence);
    case _SEQUENTIAL_HIGH_FITTING_:
      return cookSequentialHighFitting(adim, pdim, sequence);
    case _MULTI_DIM_INTERPOLATION_:
      if (pdim || sequence) return NULL;
      return cookMultiDimInterpolation(adim);
    case _MULTI_DIM_FITTING_:
      if (sequence) return NULL;
      return cookMultiDimFitting(adim, pdim);
  default: return NULL;
  } /*switch*/
} /* Mgrid::cookInterpolation */

/* ========================================================================== */
/* Calculate linear address in a grid cell array and return pointer;          */

int Mgrid::multiAddrToLinear(int id[3])
{
  // Sanity check on the limits first; 
  for(int ik=0; ik<3; ik++)
    if (id[ik] < 0 || id[ik] >= dir[ik].dim) return -1;

  return id[_X_]*dir[_Y_].dim*dir[_Z_].dim + id[_Y_]*dir[_Z_].dim + id[_Z_];
} /* Mgrid::multiAddrToLinear */

/* -------------------------------------------------------------------------- */
/*   May use 'mgrid->dir[]' here because component->dir[] have the same 'dim' */
/* entries;                                                                   */     

int Mgrid::linearAddrToMulti(int linear, int id[3])
{
  // Sanity check; 
  if (linear < 0 || linear >= cell_num) return -1;

  // This way it will work for 2 and 3 dim case; 
  for(int ik=_Z_; ik>=_X_; ik--)
  {
    MgridDirection *pdir = dir + ik;

    id[ik]  = linear % pdir->dim;
    linear /= pdir->dim;
  } /*for*/

  return 0;
} /* Mgrid::linearAddrToMulti */

/* -------------------------------------------------------------------------- */

MgridCell *Mgrid::linearAddrToCellPtr(int linear)
{
  if (linear < 0 || linear >= cell_num) return NULL;

  return (MgridCell *)(cells + linear*ram_cell_size);
} /* Mgrid::linearAddrToCellPtr */

/* -------------------------------------------------------------------------- */

MgridCell *Mgrid::multiAddrToCellPtr(int id[3])
{
  int linear = multiAddrToLinear(id);

  if (linear == -1) return NULL;

  return (MgridCell *)(cells + linear*ram_cell_size);
} /* Mgrid::multiAddrToCellPtr */

/* -------------------------------------------------------------------------- */

int Mgrid::multiAddrToCoord(int id[], TVector3 &xx)
{
  for(int ik=0; ik<3; ik++)
  {
    MgridDirection *pdir = dir + ik;

    // Sanity check, please; in fact it is not always needed; ignore; 
    if (id[ik] < 0 || id[ik] >= pdir->dim) return -1;

    xx[ik] = pdir->min + (id[ik] + 0.5)*pdir->step;
  } /*for*/

  return 0;
} /* Mgrid::multiAddrToCoord */

/* -------------------------------------------------------------------------- */

int Mgrid::linearAddrToCoord(int linear, TVector3 &xx)
{
  int id[3];

  if (linear < 0 || linear >= cell_num) return -1;

  // Now convert to 3dim indices; 
  if (linearAddrToMulti(linear, id)) return -1;

  // Not too much efficient, but suffices; 
  return multiAddrToCoord(id, xx);
} /* Mgrid::linearAddrToCoord */

/* -------------------------------------------------------------------------- */

int Mgrid::cellPtrToCoord(MgridCell *cell, TVector3 &xx)
{
  int linear, offset = (char *)cell - (char*)cells;

  // Sanity check; 
  if (offset % ram_cell_size) return -1;
  linear = offset / ram_cell_size;

  if (linear < 0 || linear >= cell_num) return -1;

  return linearAddrToCoord(linear, xx);
} /* Mgrid::cellPtrToCoord */

/* -------------------------------------------------------------------------- */

int Mgrid::coordToMultiAddr(TVector3 &xx, int id[])
{
  int ret = 0;

  // Well, floor() can easily fail with a floating point exception 
  // in case if integer is out of range --> introduce a safe_floor() later;
  //printf("%d: %f %f %f\n", (unsigned)mgrid, xx[0], xx[1], xx[2]); fflush(stdout);

  for(int ik=0; ik<3; ik++)
    if (coord.fake[ik])
      id[ik] = 0;
    else
    {
      MgridDirection *pdir = dir + ik;

      // Well, introduce some safety against fpe, endlich;
      if (xx[ik] < pdir->min)
	// Yes, set to something "wrong";
      	id[ik] = -1;
      else
      {
	if (xx[ik] > pdir->max)
	  // Also set to something "wrong";
	  id[ik] = pdir->dim;
	else
	  id[ik] = (int)floor((xx[ik] - pdir->min)/pdir->step);
      } /*if*/      

      // Do not return immediately, since id[] array 
      // makes sense even in this case;              
      if (id[ik] < 0 || id[ik] >= pdir->dim) ret = -1;
    } /*for*/

  return ret;
} /* Mgrid::coordToMultiAddr */
 
/* -------------------------------------------------------------------------- */

MgridCell *Mgrid::coordToCellPtr(TVector3 &xx)
{
  int id[3];
  
  // First calculate milti-dim indices; 
  if (coordToMultiAddr(xx, id)) return NULL;

  // And then return cell pointer (or NULL); 
  return multiAddrToCellPtr(id);
} /* Mgrid::coordToCellPtr */ 
 
/* -------------------------------------------------------------------------- */

int Mgrid::multiAddrToCombiCell(int id[], MgridCombiCell *qcell)
{
  int linear = multiAddrToLinear(id);

  if (linear == -1) return -1;

  // Then assign cell pointer and property; 
  qcell->cell = (MgridCell *)(cells + linear*ram_cell_size);
  qcell->property = properties[linear];

  return 0;
} /* Mgrid::multiAddrToCombiCell */ 

/* -------------------------------------------------------------------------- */

int Mgrid::coordToCombiCell(TVector3 &xx, MgridCombiCell *qcell)
{
  int id[3];
  
    /* First calculate milti-dim indices; */
  if (coordToMultiAddr(xx, id)) return -1;

  return multiAddrToCombiCell(id, qcell);
} /* Mgrid::coordToCombiCell */
 
/* -------------------------------------------------------------------------- */

MgridCell *Mgrid::coordToCellPtr(double xx[])
{
  int id[3];
  TVector3 xv(xx[0], xx[1], xx[2]);

  // First calculate milti-dim indices; 
  //if (coordToMultiAddr(xx, id)) return NULL;
  if (coordToMultiAddr(xv, id)) return NULL;

  // And then return cell pointer (or NULL); 
  return multiAddrToCellPtr(id);
} /* Mgrid::coordToCellPtr */ 
 
/* -------------------------------------------------------------------------- */

int Mgrid::cellPtrToLinear(const MgridCell *cell) const
{
  // This is BS of course (assume cell points to array member); FIXME: do better later;
  //return cell - (MgridCell *)cells; 
  //const unsigned char *from = cells, *to = (const unsigned char*)cell;
  //return (to - from)/ram_cell_size; 
  return ((const unsigned char*)cell - cells)/ram_cell_size;
} // Mgrid::cellPtrToLinear()

/* -------------------------------------------------------------------------- */
/*   Expects that fake coordinates are omitted in xx[] (but proper sequence   */
/* is preserved);                                                             */

int Mgrid::compressedCoordToLinearAddr(TVector3 &xx)
{
  // Well, need to adapt to TVector3 and debug at some point;
  assert(0);
#ifdef _APR2014_
  int id[3];
  double bff[3], *ptr;
  
  // First expand to 3-dim coordinates; this routine is rarely used     
  // and should not be too efficient;                                 
  if (cexp)
  {
    assert(0);
    //@@@ expand_to_global_coordinates(coord.coord_num, xx, bff, cexp);
    ptr = bff;
  }
  else
    ptr = xx;

  // Calculate milti-dim indices; 
  if (coordToMultiAddr(ptr, id)) return -1;

  // And then return linear address (or -1); 
  return multiAddrToLinear(id);
#endif
} /* Mgrid::compressedCoordToLinearAddr */ 

/* ========================================================================== */

int Mgrid::exportCore(FILE *fout, unsigned char contents)
{
  int tail = suggestedPropertyArrayTail();
  unsigned char *addr = cells;

  if (__type == _MGRID_HEAP_)
  {
    // Loop through all the children; recursion; 
    for(int ch=0; ch<child_num; ch++)
      if (children[ch]->exportCore(fout, contents)) 
	return -1;
  }
  else
  {
    // If more bits wanted, this should be changed; 
    if (contents >> 2) return -1;
    // Store contents bits in the header; 
    object_contents_bits = contents;

    // Write out single mgrid header and cell data; 
    //if (fwrite(this, current_mgrid_header_length(), 1, fout) != 1) return -1;
    if (fwrite(this, sizeof(MgridHeader), 1, fout) != 1) return -1;

    if (contents & _CELL_DATA_)
    {
      if (useful_cell_size == ram_cell_size)
      {
	// What is stored in 'cells' is all useful --> dump as a whole; 
	if (fwrite(cells, ram_cell_size, cell_num, fout) != cell_num)
	  return -1;
      }
      else
	// Cut out 'useless' part of cells stored in 'cells'; 
	for(int ik=0; ik<cell_num; ik++)
	{
	  if (fwrite(addr, useful_cell_size, 1, fout) != 1)
	    return -1;
	
	  addr += ram_cell_size;
	} /*for..if*/
    } /*if*/

    // And eventually dump 'properties' as a whole; 
    if (contents & _CELL_PROPERTIES_)
    {
      if (fwrite(properties, sizeof(char), cell_num, fout) != cell_num)
	return -1;

      // Perhaps few bytes more?; 
      if (tail && fwrite(properties, sizeof(char), tail, fout) != tail)
	return -1;
    } /*if*/
  } /*if*/

  return 0;
} /* Mgrid::exportCore */

/* -------------------------------------------------------------------------- */

int Mgrid::exportFieldMap(char *file_name, unsigned char contents)
{
  // Check sizes of basic types; 
  if (!basic_types_match()) return -1;

  FILE *fout = fopen(file_name, "w");
  if (!fout) return -1;
  
  // All the rest is done recursively; 
  int ret = exportCore(fout, contents);

  fclose(fout);

  return ret;
} /* Mgrid::exportFieldMap */

/* ========================================================================== */
/*  Well, want property[] to occupy 8byte-even chunk of memory; this is of    */
/* interest at least for the shared memory application;                       */

int Mgrid::suggestedPropertyArrayTail()
{
  int rest = cell_num % 8;

  if (!rest) 
    return 0;
  else
    return 8 - rest;
} /* Mgrid::suggestedPropertyArrayTail */

/* ========================================================================== */
/*   Well, perhaps later put shift/theta/phi as parameters; for now mgrid     */
/* will be located in heap with it's present position parameters;             */

int Mgrid::attachToHeap(Mgrid *heap)
{
  if (heap->__type != _MGRID_HEAP_) return -1;

  heap->child_num++;
  heap->children = (Mgrid**)realloc(heap->children, heap->child_num*sizeof(Mgrid*));
  if (!heap->children) return -1;

  heap->children[heap->child_num-1] = this;

  return 0;
} /* Mgrid::attachToHeap */

/* ========================================================================== */

static t_coord_name coord_names[] = {
  {'X', _X_, _CARTESIAN_},
  {'Y', _Y_, _CARTESIAN_},
  {'Z', _Z_, _CARTESIAN_|_CYLINDRICAL_},
  {'R', _R_, _CYLINDRICAL_},
  {'F', _F_, _CYLINDRICAL_}};
#define COORD_NAMES_NUM (sizeof(coord_names)/sizeof(coord_names[0]))

t_coord_name *find_coord_by_name(unsigned char system_type, char name)
{
  for(int ik=0; ik<COORD_NAMES_NUM; ik++)
  {
    t_coord_name *cname = coord_names + ik;

    if (name == cname->name)
    {
      // Check that this coordinate is allowed for this coordinate system 
      // at all; if not, return -1;                                       
      if (cname->allowed_systems & system_type) 
	return cname;
      else
	return NULL;
    } /*if*/
  } /*for*/

    // No such coordinate name known; 
  return NULL;
} /* find_coord_by_name */

/* ========================================================================== */
/*  Well, since t_mgrid contains only 64-bit words, cell data are doubles and */
/* property array is characters with it's tail aligned at 8-byte boundary,    */
/* the only thing I have to check is that sizeof(double)=8; or probably       */
/* should I also check that sizeof(long)<=8?;                                 */

int basic_types_match( void )
{
  if (sizeof(double) != 8) return 0;

  return 1;
} /* basic_types_match */

/* ========================================================================== */
/* Recalculates shift/rotation flags and direct/reversed rotation matrices;   */

int Mgrid::recalculatePosition()
{
  int shift_flag = 0, rotation_flag = 0;
  TVector3 basis[3], buffer;
  double projection;

  // First check that shift/rotation are needed; 
  for(int ik=0; ik<3; ik++)
    if (shift[ik])
    {
      shift_flag = 1;
      break;
    } /*if..for*/

  if (theta[0] != 90. || theta[1] != 90. || theta[2] !=  0. ||
        /* phi[2] is of no importance if theta[2]=0.; */
      phi  [0] !=  0. || phi  [1] != 90.)
    rotation_flag = 1;

  if (!shift_flag && !rotation_flag && position)
  { 
    delete position;
    position = NULL;
  }
  else
  {
    if (!position) 
    {
      position = new MgridPosition();
      memset(position, 0x00, sizeof(MgridPosition));
    } /*if*/

    if (shift_flag) position->shift_flag = 1;

    if (rotation_flag)
    {
      position->rotation_flag = 1;

        /* Calculate new basis vectors; */
      for(int ik=0; ik<3; ik++)
      {
	basis[ik][_X_] = sin(deg2rad(theta[ik]))*
			     cos(deg2rad(phi[ik]));
	basis[ik][_Y_] = sin(deg2rad(theta[ik]))*
				       sin(deg2rad(phi[ik]));
	basis[ik][_Z_] = cos(deg2rad(theta[ik]));
      } /*for*/

        // Check that these vectors are orthogonal and define 
        // right-handed coordinate system; fix if needed;     
        // '1': basis[0] is normalized (see creation);        
      for(int ii=1; ii<3; ii++)
      {
	for(int ik=0; ik<ii; ik++)
	{
	  projection = basis[ii].Dot(basis[ik]);//smul_v_v(basis[ii], basis[ik]);

	  //mul_s_v(buffer, -projection, basis[ik]);
	  //add_v_v(basis[ii], basis[ii], buffer);
	  basis[ii] -= projection * basis[ik]; 
	} /*for*/

	basis[ii].SetMag(1.0);//normalize_v(basis[ii]);
      } /*for*/

      buffer = basis[0].Cross(basis[1]);//vmul_v_v(buffer, basis[0], basis[1]);
      // Well, this is a patology; 
      //if (smul_v_v(buffer, basis[2]) < 0.)
      if (buffer.Dot(basis[2]) < 0.)//smul_v_v(buffer, basis[2]) < 0.)
      {
	printf("Transformation to a left-handed system!\n");
	return -1;
      } /*if*/

      // Need to incorporate TRotation::MakeBasis() later;
      assert(0);
#if 0
        // And eventually calculate direct/reversed matrices; 
      for(int ii=0; ii<3; ii++)
	for(int ik=0; ik<3; ik++)
	  position->direct[ii][ik] = 
	    position->reversed[ik][ii] = 
	      basis[ii][ik];
#endif
    } /*if*/
  } /*if*/

  return 0;
} /* Mgrid::recalculatePosition */

/* -------------------------------------------------------------------------- */
/*   Changes position of this object with respect to it's father; (just       */
/* assigns new values; if pointer is NULL, no change to this array;           */

int Mgrid::changePosition(double _shift[3], double _theta[3], double _phi[3])
{
  if (_shift)
    for(int ik=0; ik<3; ik++)
      shift[ik] = _shift[ik];

  if (_theta)
    for(int ik=0; ik<3; ik++)
      theta[ik] = _theta[ik];

  if (_phi)
    for(int ik=0; ik<3; ik++)
      phi  [ik] = _phi  [ik];

  return recalculatePosition();
} /* Mgrid::changePosition */

/* ========================================================================== */
/*  The sequence is:                                                          */
/*    - find cell area around xx[] which will be used for interpolation       */
/*        (algorithm depends on the interpolation mode);                      */
/*    - calculate interpolated (fitted) field value in a local grid system;   */
/*  NB: this routine is expected to preserve xx[];                            */

int Mgrid::directFieldValue(TVector3 &xx, TVector3 &B)
{
  int i012[3], seq[3], rev[3];
  // xxx[] is needed as intermediate array because this routine 
  // is not allowed to change xx[]; 
  double xxx[3];

  // Will need to find self[] array anyway (cell to 
  // which this xx[] belongs) --> do it now;        
  if (findSelfCell(xx)) return -1;

  // The rest depends on the mode; loop through the field components; 
  for(int cmp=0; cmp<field.coord_num; cmp++)
  {                            
    MgridInterpolation *inter = interpolation[cmp];

    // If interpolation is not associated with this mgrid, return immediately; 
    if (!inter) _FAILURE_(this, _ZERO_INTERPOLATION_POINTER_);

    // Check that self cell may be used at all; 
    if (!(self_qcell.property & inter->allowed_self_cells_mask))
      _FAILURE_(this, _ILLEGAL_SELF_CELL_)

    if (inter->mode == _NO_INTERPOLATION_)
    {
      // inter->self_cell can not be _DEAD_CELL_ because   
      // this bit is masked out for this mode => go ahead; 
      B[cmp] = self_qcell.cell->B[cmp];
    } 
    else
    if (inter->mode == _SEQUENTIAL_FAST_INTERPOLATION_ || 
	inter->mode == _SEQUENTIAL_HIGH_INTERPOLATION_  || 
	inter->mode == _SEQUENTIAL_HIGH_FITTING_ )
    {
      //printf("Here!\n"); exit(0);
        // These modes require strict cubic areas;           
        // 'repetition_flag=1' means inter->left[] array was 
        // assigned by hand already;                         
      if (inter->checkNeighbouringCubicArea(xx, cmp)) 
	  return -1;

      for(int ik=0; ik<3; ik++)
      {
	seq[ik] = inter->sequence[ik];
	rev[ik] = inter->reversed[ik];

	  // Move point to the leftmost position (expansion is per 
	  // definition done with respect to the leftmost corner); 
	  // another option would be to keep xx[], but choose a    
	  // proper starting point in coord[] array;  
	xxx[ik] = xx[ik];
	if (inter->left[ik]) xxx[ik] -= inter->left[ik]*dir[ik].step;
      } /*for*/

      if (inter->mode == _SEQUENTIAL_FAST_INTERPOLATION_)
      {
	for(i012[2]=0; i012[2]<inter->adim[seq[2]]; i012[2]++)
        {
	  for(i012[1]=0; i012[1]<inter->adim[seq[1]]; i012[1]++)
	  {
	      // Fill out array of values along inter->sequence[0]; 
	    for(i012[0]=0; i012[0]<inter->adim[seq[0]]; i012[0]++)
	      inter->v[0][i012[0]] = inter->cube[i012[rev[_X_]]*inter->adim[_Y_]*inter->adim[_Z_] + 
					  i012[rev[_Y_]]*inter->adim[_Z_] + i012[rev[_Z_]]]->B[cmp];
	      // Interpolation along inter->sequence[0]; 
	    inter->v[1][i012[1]] = inter->fun[0](xxx[seq[0]], cell_center_coord[seq[0]], 
						 inter->v[0]);
	  } /*for*/

	    // Interpolation along inter->sequence[1]; 
	  inter->v[2][i012[2]] = inter->fun[1](xxx[seq[1]], cell_center_coord[seq[1]],
					       inter->v[1]);
	} /*for*/

          // Interpolation along inter->sequence[2]; 
	B[cmp] = inter->fun[2](xxx[seq[2]], cell_center_coord[seq[2]],
			       inter->v[2]);
      }
      else
      {
	// Yes, for now just hardcode this check; perhaps this has something 
	// to do with weight usage; if weights are of no interest, it seems 
	// one can remove this restiction; think later;
#if _BACK_
	assert(coord.coord_num == 3 && field.coord_num == 1);
#endif

	ThreeDeePolySpace *space0 = inter->hspace[0];
	ThreeDeePolySpace *space1 = inter->hspace[1];
	ThreeDeePolySpace *space2 = inter->hspace[2];

	for(i012[2]=0; i012[2]<inter->adim[seq[2]]; i012[2]++)
        {
	  ThreeDeePolyPoint *point2 = space2->points + i012[2];

	  for(i012[1]=0; i012[1]<inter->adim[seq[1]]; i012[1]++)
	  {
	    ThreeDeePolyPoint *point1 = space1->points + i012[1];

	      // Fill out array of values along inter->sequence[0];  
	    for(i012[0]=0; i012[0]<inter->adim[seq[0]]; i012[0]++)
	    {
	      MgridCell *cell = inter->cube[i012[rev[_X_]]*inter->adim[_Y_]*inter->adim[_Z_] + 
					       i012[rev[_Y_]]*inter->adim[_Z_] + i012[rev[_Z_]]];
	      ThreeDeePolyPoint *point0 = space0->points + i012[0];

	      point0->f = cell->B[cmp];

	      // If point weights should be taken into account, do it now; 
	      if (inter->force_1dim_weight_usage)
		point0->weight = 1./SQR(cell->B[cell_1derr_shift + cmp*coord.coord_num + 0]);
	    } 

	    // Rebuild basis polynomial vectors (since weights changed); 
	    space0->buildOrthogonalPolynomials();
	    // Interpolation along inter->sequence[0];  
	    space0->calculateFittingPolynomial(inter->hfit[0]);

	    point1->f = inter->hfit[0]->value(xxx);

	    if (inter->force_1dim_weight_usage)
	      point1->weight = 1./(SQR(space0->getNaivePolyFitError(xxx)) + 
				     // Use self cell for now; may want to do interpolation later; 
				   SQR(self_qcell.cell->B[cell_1derr_shift + cmp*coord.coord_num + 1]));
	  } /*for*/

	    // Rebuild basis polynomial vectors (since weights changed); 
	  space1->buildOrthogonalPolynomials();
	    // Interpolation along inter->sequence[1];  
	  space1->calculateFittingPolynomial(inter->hfit[1]);

	  point2->f = inter->hfit[1]->value(xxx);

	  if (inter->force_1dim_weight_usage)
	    point2->weight = 1./(SQR(space1->getNaivePolyFitError(xxx)) + 
				   // Use self cell for now; may want to do interpolation later; 
				 SQR(self_qcell.cell->B[cell_1derr_shift + cmp*coord.coord_num + 2]));
	} /*for*/ 

	  // Rebuild basis polynomial vectors (since weights changed); 
	space2->buildOrthogonalPolynomials();
          // Interpolation along inter->sequence[2];  
	space2->calculateFittingPolynomial(inter->hfit[2]);
	//B[cmp] = poly_value(space2, inter->hfit[2], xxx);
	B[cmp] = inter->hfit[2]->value(xxx);

	if (inter->fit_error_wanted)
	  inter->ferr = space2->getNaivePolyFitError(xxx);
      } /*if*/
    } 
    else
    if (inter->mode == _MULTI_DIM_INTERPOLATION_ ||
	inter->mode == _MULTI_DIM_FITTING_)
    {
      // For now repetition mode works for simple cases only;  
      assert(!repetition_flag);

      // Prefer cubic area (faster!), but may work with (almost) 
      // any collection of points;                               
      inter->checkNeighbouringCubicArea(xx, cmp);

      // This is indeed a redundant reassignment for the non-regular case; 
      // however this allows to unify few more lines of code (see after    
      // the below if{} statement; and anyway irregular case is so         
      // CPU-intensive that few reassignments do not contribute much;      
      for(int ik=0; ik<3; ik++)
	xxx[ik] = xx[ik];

      if (inter->cell_counter == inter->cube_cell_num && !inter->force_irregular_case)
      {
	// Need to shift xx[] to the leftmost corner of the grid because  
	// polynomial basis was built in post_cook_multi_dim_common() for 
	// the cube shifted to this corner;                               
	for(int ik=0; ik<3; ik++)
	  if (inter->left[ik])
	    xxx[ik] -= inter->left[ik]*dir[ik].step;

	inter->actual = inter->gspace;
      } 
      else
      {
	// Ok, no luck with the ideal cube --> append as many more neighboring 
	// points as needed;                                                   
	last_field_status |= _CUBE_FAILURE_;
	
	if (inter->appendArbitraryNeighbouringCells(cmp)) return -1;

	// Unfortunately need to fill in all the coordinates of all cells 
	// and (even worse) to rebuild basis;                             
	if (inter->prepairIrregularCase(cmp)) return -1;

	inter->actual = inter->irregular;
      } /*if*/

        // To this point inter->actual is the pointer to the actually used 
        // polynomial frame; now may produce 'best polynomial' and take    
        // it's value at xxx[]; first assign function values;              
      for(int ip=0; ip<inter->cube_cell_num; ip++)
	inter->actual->points[ip].f = inter->cube[ip]->B[cmp];

      // This place was not checked after conversion to C++;
      inter->actual->calculateFittingPolynomial(inter->gfit);
      B[cmp] = inter->gfit->value(xxx);

      if (inter->gradient_wanted)
      {
	inter->gfit->calculateGradient(inter->grad);
	for(int ik=0; ik<3; ik++)
	  inter->gvalue[ik] = inter->grad[ik]->value(xxx);
      } /*if*/
    } /*if*/
  } /*for*/

      //printf("@W@ %f %f %f\n", B[0], B[1], B[2]);


      // Now B[] contains fit result, but in the (probably even 2-dim) 
      // local coordinate system of the grid; need to convert it to    
      // the 3-dim value;                                              
  if (fexp) {
    assert(0);
    //@@@ expand_to_global_coordinates(field.coord_num, B, B, fexp);
  } //if

  return 0;
} /* Mgrid::directFieldValue */

/* -------------------------------------------------------------------------- */
/*  * - accept input 3dim coordinates;                                        */
/*  * - shift/rotate input 3dim coordinates appropriately and do coordinate   */
/*      system type transformation if needed;                                 */
/*  * - if single mgrid, call mgrid_field_value(), otherwise propagate one    */
/*      level down recursively;                                               */ 
/*  * - rescale if needed;                                                    */
/*  * - expand this value to 3dim;                                            */
/*  * - rotate field value back and apply coordinate system type              */
/*        transformation if needed;                                           */

int Mgrid::getFieldValue(t_3d_cs_vector *X, t_3d_cs_vector *D)
{
  int ret = -1;
  // '=' per default just reuse 'X' pointer (save CPU time); 
  t_3d_cs_vector XL, *XP = X;
  // Save for further usage; 
  unsigned char wanted_field_coord_system_type = D->system_type;

  last_field_status = 0;

  if (magic_header != _MFILE_MAGIC_) _FAILURE_(this, _BAD_MGRID_POINTER_);

  // Check _CYLINDRICAL_/_CARTESIAN_ match, fix if needed; apply shift/rotation 
  // if needed; needed for both HEAP/SINGLE cases; 'XL': preserve 'X' if it 
  // should be modified;        
  if (position || X->system_type != coord.system_type)
  {
    // Well, X should be modified => create a local 
    // (modified) copy, and switch pointer;          
    XL = convertInputCoordinates(X);
    XP = &XL;
  } /*if*/

    // Ok, now X is transformed to the object native coordinates => 
    // proceed according to whether it's a collection of grids or   
    // a single grid;                                               
  if (__type == _MGRID_HEAP_)
  {
    switch (field_calculation_method)
    {
      case _SUPERPOSITION_:
	{
            // Well, in fact in case of HEAP field.system_type              
	    // does not make too much sense (as well as coord.system_type); 
	    // so one could put D->system_type in DL directly; but code becomes    
	    // even less readable in this case => sacrifice few CPU cycles for the 
	    // case when D->system_type != field.system_type (HEAP);
	  t_3d_cs_vector DL = {field.system_type};

	  for(int ik=0; ik<3; ik++)
	    D->xx[ik] = 0.;

	  for(int ch=0; ch<child_num; ch++)
	  {
	    Mgrid *child = children[ch];

              // Call the same routine recursively; 
	    ret = child->getFieldValue(XP, &DL);

	      // Whatever happens, HEAP status for this method      
	      // will be a logical OR of member mgrids status words;
	    last_field_status |= child->last_field_status;

	    if (ret) break;

	    for(int ik=0; ik<3; ik++)
	      D->xx[ik] += DL.xx[ik];
	  } /*for*/ 
	}
	break;
      case _FIRST_MATCH_:
	for(int ch=0; ch<child_num; ch++)
	{
	  Mgrid *child = children[ch];

            // Call the same routine recursively;                   
	    // this strange assignment is done because in all cases 
	    // I want D->xx[] to contain field in mgrid field       
	    // coordinate system; this is faster than using DL[] and
	    // copy DL --> D in a separate loop;                    
	  D->system_type = field.system_type;
	  ret = child->getFieldValue(XP, D);
	    // For this method (_FIRST_MATCH_) HEAP status word 
	    // is the (success) word of the first match or (if  
	    // no successful mgrid found) a logical OR of all   
	    // failed children mgrids;                          
	  if (ret)
	    last_field_status |= child->last_field_status;
	  else
	  {
	    last_field_status  = child->last_field_status;
	    break;
	  } /*if*/
	} /*for*/ 
	break;
      default:
	ret = -1;
    } /*switch*/
  } 
  else
      // Calculate single grid interpolated field value;     
      // NB: convert_input_coordinates() already happened in 
      // the very beginning of get_field_value() routine;    
    ret = directFieldValue(XP->xx, D->xx);

  // In case of success few more actions; 
  if (!ret)
  {
      // Well, perhaps need to scale; NB: all 3 components!;             
      // for a single few-component mgrid it would be more CPU-saving to 
      // do rescaling before expanding to global coordinates, but prefer 
      // to unify HEAP/SINGLE codes as much as possible;                 
    if (scaling_factor != 1.)
      for(int ik=0; ik<3; ik++)
	D->xx[ik] *= scaling_factor;
  
      // At this point field is already 3dim and rescaled in all     
      // cases; in all cases it is mgrid field coordinate system --> 
      // still coordinate type transformation may be needed; and     
      // probably rotation as well; in trivial cases (read: usually) 
      // this is not needed;                                         
    if (field.system_type != wanted_field_coord_system_type ||
	(position && position->rotation_flag))
      convertOutputField(XP, D, wanted_field_coord_system_type);
  }
  else
      // Reset field for clarity in case of ret=-1; 
    for(int ik=0; ik<3; ik++)
      D->xx[ik] = 0.;

  return ret;
} /* Mgrid::getFieldValue */

/* -------------------------------------------------------------------------- */
/*  Be pedantic, since both cartesian/cylindrical cases wanted, make common   */
/* part separate; only system usage --> no 'system_type' check;               */

int Mgrid::getFixedTypeFieldValue(unsigned char system_type, TVector3 &xx, TVector3 &B)
{
  t_3d_cs_vector X = {system_type, TVector3(xx[0], xx[1], xx[2])};
  t_3d_cs_vector D = {system_type};

  int ret = getFieldValue(&X, &D);

  for(int ik=0; ik<3; ik++)
    B[ik] = D.xx[ik];

  return ret;
} /* Mgrid::getFixedTypeFieldValue */

/* -------------------------------------------------------------------------- */
/*  The most popular case --> separate routine; CPU overhead?;                */

int Mgrid::getCartesianFieldValue(TVector3 &xx, TVector3 &B)
{
  return getFixedTypeFieldValue(_CARTESIAN_, xx, B);
} /* Mgrid::getCartesianFieldValue */

/* -------------------------------------------------------------------------- */

int Mgrid::getCylindricalFieldValue(TVector3 &xx, TVector3 &B)
{
  return getFixedTypeFieldValue(_CYLINDRICAL_, xx, B);
} /* Mgrid::getCylindricalFieldValue */

/* ========================================================================== */
/*   Well, structure copy  does not take too much compared to FP operations;  */
/* and there is no need to check for the source/destination pointer overhead; */
/* I mean this function does not care about possible overlap :-)              */

//
// -> Ok, do not mind to keep there 2 routines as external functions;
//

static t_3d_cs_vector transform_coord_type(t_3d_cs_vector *X, 
					   unsigned char desired_coord_system)
{
  double angle_in_radians;
  t_3d_cs_vector XL;

  // Sanity check, perhaps there is nothing to do; 
  if (X->system_type == desired_coord_system) return *X;

  // For now only 2 system types are in used, consider both cases; 
  if (X->system_type == _CARTESIAN_ && desired_coord_system == _CYLINDRICAL_)
  {
    XL.xx[_R_] = sqrt(SQR(X->xx[_X_])+SQR(X->xx[_Y_]));
    XL.xx[_F_] = X->xx[_X_] ? rad2deg(atan(X->xx[_Y_]/X->xx[_X_])) : (X->xx[_Y_] > 0. ? 90. : -90.);
    if (X->xx[_X_] < 0.) XL.xx[_F_] += 180.;
  } 
  else
  if (X->system_type == _CYLINDRICAL_ && desired_coord_system == _CARTESIAN_)
  { 
    angle_in_radians = deg2rad(X->xx[_F_]);
    XL.xx[_X_] = X->xx[_R_]*cos(angle_in_radians);
    XL.xx[_Y_] = X->xx[_R_]*sin(angle_in_radians);
  } 
  else
    // No more options known; terminate; 
    assert(0);

  // Z-component remains the same; 
  XL.xx[_Z_] = X->xx[_Z_];

  // And eventually change system type to a new one; 
  XL.system_type = desired_coord_system;

  return XL;
} /* transform_coord_type */

/* -------------------------------------------------------------------------- */
/*  Is suitable only for CAR/CYL case; if other systems will be needed in the */
/* future, will have to rework completely;                                    */

static void transform_field_type(double mtx[2][2], t_3d_cs_vector *D, 
				 unsigned char desired_coord_system)
{
  TVector3 bff;
  
  // Sanity check; 
  if (D->system_type == desired_coord_system) return;

  // '2': well, Z-component will stay the same; 
  for(int ik=0; ik<2; ik++)
  {
    bff[ik]   = D->xx[ik];
    D->xx[ik] = 0.;
  } /*for*/

    // For now only 2 system types are in use, consider both cases; 
  if (D->system_type == _CARTESIAN_ && desired_coord_system == _CYLINDRICAL_)
  { 
    
  } 
  else
  if (D->system_type == _CYLINDRICAL_ && desired_coord_system == _CARTESIAN_)
  {
    mtx[1][0] = -mtx[1][0];
    mtx[0][1] = -mtx[0][1];
  }
  else
    // No more options known; terminate; 
    assert(0);

  // Eventually apply 2dim rotation; 
  for(int ii=0; ii<2; ii++)
    for(int ik=0; ik<2; ik++)
      D->xx[ii] += mtx[ii][ik]*bff[ik];

  // And eventually change system type to a new one; 
  D->system_type = desired_coord_system;
} /* transform_field_type */

/* -------------------------------------------------------------------------- */
/*  Converts output B[] field to the coordinate system type of the calling    */
/* object; also applies reversed rotation if needed;                          */

void Mgrid::convertOutputField(t_3d_cs_vector *X, t_3d_cs_vector *D, 
			       unsigned char wanted_field_coord_system_type)
{
  // I'm not allowed to change X in this routine;
  t_3d_cs_vector BFF;
  double angle_in_radians;
  double mtx[2][2];

  // Will need to call transform_field_type() twice => calculate     
  // rotation matrix once and pass as a parameter; if spherical case 
  // is ever needed, this part should be reworked completely;        
  if (X->system_type != _CYLINDRICAL_)
  {
    BFF = transform_coord_type(X, _CARTESIAN_);
    // And forget about original X; 
    X = &BFF;
  } /*if*/
    // Calculate 2dim rotation matrix corresponding to the X'phi; 
  angle_in_radians = deg2rad(X->xx[_F_]);
  mtx[0][0] = mtx[1][1] = cos(angle_in_radians);
  mtx[0][1] = sin(angle_in_radians);
  mtx[1][0] = -mtx[0][1];

  // Do this strange reassignment because transform_coord_type() expects   
  // consistent xx[] and system_type in 'D'; in fact to this moment        
  // D->system_type was REALLY field.system_type (since it has just 
  // been out of mfile_field_value() in mgrid's coordinates);              
  D->system_type = field.system_type;

  // Check coordinate type match; do transformation to the calling 
  // object coordinate system if necessary;                        
  if (D->system_type == _CYLINDRICAL_ && 
      ((position && position->rotation_flag) || 
       wanted_field_coord_system_type == _CARTESIAN_))
    transform_field_type(mtx, D, _CARTESIAN_);

  // Now if rotation needed, it's Ok, since                 
  // D->system_type is _CARTESIAN_ => rotate now if needed; 
  if (position && position->rotation_flag)
    // This has never been checked after conversion to TRotation;
    //mul_a_v(D->xx, position->reversed, D->xx);
    D->xx = position->reversed * D->xx;

    // If to this point there is a coordinate type mismatch, do one more 
    // transformation;                                                   
  if (D->system_type != wanted_field_coord_system_type)
    transform_field_type(mtx, D, wanted_field_coord_system_type);
} /* Mgrid::convertOutputField */

/* -------------------------------------------------------------------------- */
/*  Converts input xx[] coordinates provided by caller to the native type and */
/* also applies shift/rotation if needed; the code is not well optimised, but */
/* just straightforward;                                                      */

t_3d_cs_vector Mgrid::convertInputCoordinates(t_3d_cs_vector *X)
{
  t_3d_cs_vector XL;

    // Check coordinate type match; do transformation to the object 
    // coordinate system if necessary;                              
  if (X->system_type == _CYLINDRICAL_ && 
      (position || coord.system_type == _CARTESIAN_))
    XL = transform_coord_type(X, _CARTESIAN_);
  else
    XL = *X;

    // Now if shift/rotation needed, it's Ok, since                  
    // XL->system_type is _CARTESIAN_ => shift and rotate if needed; 
  if (position)
  {
    if (position->shift_flag)
        // 'sub': reversed transformation from this object point of view!; 
    for(int iqq=0; iqq<3; iqq++)
      XL.xx[iqq] -= shift[iqq];

    if (position->rotation_flag)
      // This has never been checked after conversion to TRotation;
      //mul_a_v(XL.xx, position->direct, XL.xx);
      XL.xx = position->direct * XL.xx;
  } /*if*/

    /* If to this point there is a coordinate type mismatch, do one more */
    /* transformation; so it can happen that CYL->CAR->CYL is done since */
    /* shift/rotation are of course defined for _CARTESIAN_ type;        */
  if (XL.system_type != coord.system_type)
    return transform_coord_type(&XL, coord.system_type);
  else
    return XL;
} /* Mgrid::convertInputCoordinates */

/* ========================================================================== */
