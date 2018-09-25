/* -------------------------------------------------------------------------- */
/*  MgridInterpolation.h                                                      */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <ThreeDeePolySpace.h>

#ifndef _MGRID_INTERPOLATION_H
#define _MGRID_INTERPOLATION_H

typedef double (*interfun)(double xx, double coord[], double value[]);

// No interpolation at all (take nearest node value); 
#define _NO_INTERPOLATION_                 0
// Sequential interpolation in XYZ direction in some order;
// dimensions are 2..4; '2' is linear interpolation,       
// '3' - quadratic, '4' - weighted quadratic (the latter   
// two are after WWC);                                     
#define _SEQUENTIAL_FAST_INTERPOLATION_    1
// The same, but at any dimension, orthogonal polynomials used; 
// moderately CPU intensive;                                    
#define _SEQUENTIAL_HIGH_INTERPOLATION_    2
// Well, this mode is mathematically incorrect, because it does not    
// take into account possible spread of fitting errors after first 1-2 
// directions; on the other hand taking them into account may lead     
// to a complete screw up, because one either have to rely on the      
// 'weight' field, or renormalize by sqrt(chi^2/ndf) and it is not     
// clear would not it be better to ignore errors all together;         
// besides this CPU usage will blow up since basis needs to be redone  
// for every new xx[];                                                 
// for practical reasons this mode is left in (it is indeed more       
// reasonable that interpolation with big adim[] since it gives        
// more smooth fit); if one wants fit instead of interpolation         
// and cares about mathematical correctness, _MGRID_MULTI_DIM_FITTING_ 
// is the way to go;                                                    
#define _SEQUENTIAL_HIGH_FITTING_          3
// A global 3dim polynomial interpolation (very CPU intensive); 
#define _MULTI_DIM_INTERPOLATION_          4
// The same, but number of cells may be more than overall          
// degree of polynomial; NB: basis is built only ONCE, assuming    
// that weight=1. in all points; attempt to change weights for     
// individual xx[] points afterwards will result in wrong fitting; 
// in principle one can of course introduce a 'professional' mode  
// when basis is calculated every time new (and it is in fact very 
// easy to do), but for now assume that all points have always     
// equal errors;                                                   
#define _MULTI_DIM_FITTING_                5


// For now do not see need for more than 6 doubles (3 field   
// components + 3 estimated errors); do not want 'double *B'  
// because the beginning of this structure will go directly   
// into a data file; NB: the code will be backward compatible 
// with older versions even if one day this DIM will be       
// increased, because the actual length is written in file    
// header upon creation;                                      
#define _CELL_DATA_DIM_MAX_ 10

// This format header lines (property&B[]) should never change!; 
struct MgridCell {
  // N field components; perhaps anything else dazu; 
  double B[_CELL_DATA_DIM_MAX_];

  //
  // !!! Do not touch fields above this line (apart from changing    
  // _CELL_DATA_DIM_MAX_); well, in fact B[] to be the first element 
  // in this structure is the only requirement;                      
  //

  // The rest will not be stored in data files, but it is         
  // convenient to have this stuff in right here;                 
  void *ptr;

  MgridCell *next;
} ;

// Possible bits in mgrid->object_contents_bits; 
// cell->B[] stuff (field + errors + ...);       
#define _CELL_DATA_                0x01
// Cell properties (since they are of char type it is 
// more economic to keep them in a separate array);   
#define _CELL_PROPERTIES_          0x02

// Possible bits of mgrid->cell_contents_bits; do not forget to 
// change max known bit #define below;                          
#define _FIELD_COMPONENT_VALUES_   0x01
#define _FIELD_COMPONENT_ERRORS_   0x02
#define _FIT_CHI_SQUARE_           0x04
#define _CELL_COORDINATES_         0x08
#define _FIELD_GRADIENTS_          0x10
#define _1D_FIELD_ERRORS_          0x20
#define _3D_FIELD_ERRORS_          0x40
// Some sanity; does not help much; ok; 
#define _MAX_KNOWN_CELL_CONTENTS_BIT_ 7


//
// Possible cell status; should fit in 4 bits; the more the number 
// the worse this cell was;                                        
//

// After the ASCII input all cells will be SAFE; after regriding cell 
// remains SAFE if 1) self-cell was SAFE, 2) ideal cube was used      
// for interpolation/fitting without repositioning, 3) all neighbour  
// cells used were SAFE;                                              
#define _SAFE_CELL_                0x01
// Basically means that regriding procedure for this cell either had 
// to reposition the cube, or to use EDGE self/neighbour cells;      
// in other words this cell is on the edge of the grid;               
#define _EDGE_CELL_                0x02
// This cell was either extrapolated from the other (known) cells    
// or during it's creation EXTRA cells were used (means they were at 
// least allowed to be used - see self/neighbour allowed masks);     
// creation of such cells is in principle straighforward, but not    
// implemented yet;                                                  
#define _EXTRA_CELL_               0x04
// No info for this cell available; 
#define _DEAD_CELL_                0x08

// When creating a grid out of cell list either to put all components    
// into separate grids and arrange mgrid heap (ON) or create a (2-3)-dim 
// mgrid right away (OFF); the former may be of interest if component    
// grids are not guaranteed to be perfectly sitting on top of each other 
// for instance there are relative shifts which need to be found - it is 
// much easier to do moving around daughter grids inside a heap;           
#define _SPLIT_OFF_                   0
#define _SPLIT_ON_                    1

class Mgrid;

struct MgridCombiCell {
  unsigned char property;
  MgridCell *cell;
} ;

class MgridInterpolation {
  friend class Mgrid;

 private:
  // Yes, convert old allocate_interpolation_frame() to a constructor;
  MgridInterpolation(Mgrid *mgrid);

  // There are few different types (see definitions below); they            
  // all give different accuracy and have a very different CPU consumption;  
  unsigned char mode;

  // For safety (cross-check) purposes store address of a grid 
  // for which this interpolation was cooked; 
  Mgrid *_mgrid;

  // Algorithm is expected to be fast and straightforward;           
  // therefore KxMxN cube with adim[] sizes is chosen really as a    
  // cube around xx[] point in a local grid coordinate sequence,     
  // without any attempt to move it in order to have all             
  // cells defined; if 'big' cubes wanted, it is advisable           
  // to add *extrapolated* 'galo' cells outside of the measured area 
  // when producing mfile; NB: this may change in the future;        
  // natural XYZ (RFZ) sequence;                                     
  int adim[3];

  // Interpoaltion sequence -> XYZ (RFZ) coordinates mapping;     
  // sequence[0]=_Y_ means that interpolation is first done along 
  // Y (F) direction of XYZ (RFZ) coordinate system; used         
  // in case of SEQUENTIAL modes only;                            
  int sequence[3], reversed[3];


  // -------------- The rest are working variables; --------------- 


  // Bitwise mask of which types of cells may be considered    
  // _EDGE_/_EXTRA_/_DEAD_; _DEAD_ for 'self_cell' may make    
  // sense during extrapolation of original grig for exapmple; 
  unsigned char allowed_self_cells_mask, allowed_neighbour_cells_mask;
  // Whether or not cube finder may shift cube with respect to 'best' 
  // position; is allowed per default; no need to change?;            
  unsigned char cube_shift_allowed, ideal_cube_required;

  // KxMxN cell area; allocated linearly for simplicity; 
  int cube_cell_num;
  
  // This is needed for the 'FAST' mode; NB: interpolation index sequence!;
  interfun fun[3];

  // This is a polynomial degree used for fitting (may differ from adim[]); 
  // natural XYZ (RFZ) sequence, of course;                                 
  int pdim[3];
  // If cubic area fails, few of the dimensions may be prohibited 
  // for non-cubic extension; say may want to have a true 1-dim   
  // fitting for double entries in recoil detector magnet map;    
  int extension_prohibited[3];

  // If cell points have different weights regular cube case makes not too        
  // much sense since polynomial basis has to be rebuilt every time anyway;       
  // since point weights can be changed by user any time (protect later, please!) 
  // it is also expected that user is responsible for setting this flag;          
  int force_irregular_case, force_1dim_weight_usage, force_3dim_weight_usage;

  // Gradient calculation takes CPU time => disabled per default;  
  // in fact this works in 3D XYZ case only --> need to check;    
  int fit_error_wanted, gradient_wanted;

  // KxMxN cell area; allocated linearly for simplicity; 
  int cell_counter;
  MgridCell **cube;
  
  // N-dim position of the leftmost cell in case of a cubic positioning; 
  // natural XYZ (RFZ) sequence;                                         
  int left[3];
  
  // This is needed for the 'FAST' mode; NB: interpolation index sequence!;
  double *v[3];

  // This is needed for the '1DIM POLY' mode; interpolation sequence!; 
  ThreeDeePolySpace *hspace[3];
  ThreeDeePolynomial *hfit[3];
  
  // This is needed for the 'GLOBAL' mode; in fact it would probably make 
  // sense to equip polynomial structures with local thread buffers?;
  ThreeDeePolySpace *gspace, *irregular, *actual;
  // grad[] is the gradient; 
  ThreeDeePolynomial *gfit, *grad[3];
  // Gradient value in the last xx[] point where field value 
  // was calculated; do it better later;                     
  double gvalue[3], ferr;

  // Cell, cube, etc handling routines;
  int setAllowedCells(unsigned char _allowed_self_cells_mask,
		      unsigned char _allowed_neighbour_cells_mask);
  int prepairIrregularCase(int cmp);
  int appendArbitraryNeighbouringCells(int cmp);
  int checkNeighbouringCubicArea(TVector3 &xx, int cmp);
  void fillOutputCubeArray(int gdim[3], MgridCombiCell *ok_arr, int shift[3]);
  int checkSmallCube(int gdim[3], MgridCombiCell *ok_arr, int shift[3]);
  int fillOkArray(int left[3], int adim[3], int cmp, MgridCombiCell *ok_arr);

  // Interpolation creation routines;
  void calculateReversedSequence();
  int allocateCellCubeMemory();
  int assignBestSequence();
  int assignSuggestedSequence(char _sequence[]);
  int precookSequentialHighCommon(int _adim[], char _sequence[]);
  int postcookSequentialHighCommon();
  int preCookMultiDimCommon(int _adim[]);
  int postCookMultiDimCommon();
} ;

// Possible bits in 'status' word; can not use 8 lower bits; 
#define _BAD_MGRID_POINTER_          0x00000100
#define _ZERO_INTERPOLATION_POINTER_ 0x00000200
#define _CELL_OUT_OF_GRID_           0x00000400
// This type of self cells is not allowed; 
#define _ILLEGAL_SELF_CELL_          0x00000800
// Cube may not be positioned properly; 
#define _CUBE_SHIFT_                 0x00001000
// Was not able to position cube of requested size at all; 
#define _CUBE_FAILURE_               0x00002000
// Cube movement was off and not allowed neighbour cell encountered;                               
#define _ILLEGAL_NEIGHBOUR_CELL_     0x00004000

#define _FAILURE_(mgrid, bit) {(mgrid)->last_field_status |= (bit); return -1; }

#endif
