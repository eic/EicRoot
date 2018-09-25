/* -------------------------------------------------------------------------- */
/*  Mgrid.h                                                                   */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <cstdio>

#ifndef _MGRID_H
#define _MGRID_H 

#include <MgridHeader.h>
#include <MgridInterpolation.h>
#include <Mgrid.h>
#include <ThreeDeePolySpace.h>

// Accomplish existing _X_,_Y_,_Z_ set; 
#define _R_ 0
#define _F_ 1

class MgridPosition {
 public:
  // Just marks the necessity to apply shift   
  // and/or rotation (save CPU if not needed); 
  int shift_flag, rotation_flag;

  // 'direct' is for input coordinates; 'reversed' for the output field;
  TRotation direct, reversed;
} ;

struct t_3d_cs_vector {
  // _CARTESIAN_/_CYLINDRICAL_; 
  unsigned char system_type;
  // 3D vector in this coordinate system;
  TVector3 xx;
} ;

#define _MODE_OFF_  0
#define _MODE_HRC_  1
#define _MODE_ADIM_ 2

typedef struct {
  int mode, adim[3];
} t_htc_interpolation;

class Mgrid: public MgridHeader {
  friend class MgridInterpolation;

  //
  // NB: it is critically important that 1) MgridHeader size and format 
  // do not change (except for the reserved[] bytes contents); 2) MgridHeader
  // is the very first data block in Mgrid class; otherwise at least I/O 
  // routines will become confused;
  //
  // Variables below may follow in any order, they are dynamically 
  // allocated & filled; 
  //

 public:
  // Basically memory allocation and creation of header;
  Mgrid(char *_name, MgridType _type);
  // Direct import from binary file;
  Mgrid(FILE *fin);

  // Interpolation setting routines;
  int turnInterpolationOff();
  int setInterpolationMode(unsigned mode, int adim[]);
  int setHrcInterpolationMode();
  int setHtcInterpolationMode(t_htc_interpolation *htci);
  int setFittingMode(unsigned mode, int adim[], int pdim[]);

  // Interpolation toggling routines;
  int switchInterpolation(char field_component_name, MgridInterpolation *inter);
  int restoreInterpolations();

  // Binary mgrid file export routine;
  int exportFieldMap(char *file_name, unsigned char contents);

  // Used in import.cc, sorry;
  int attachToHeap(Mgrid *heap);

  // Used in my user code;
  int initializeAsHeap();
  int initializeAsSingleMgrid(int _ram_cell_size);

  unsigned getCellNum() { return cell_num;};

  void markCellAsSafe(unsigned id) {properties[id] = _SAFE_CELL_;};

  // A field value of a grid which has no further childred;
 private:
  int directFieldValue(TVector3 &xx, TVector3 &B);

  // Internal interpolation creation methods;
  int setModeWrapper(unsigned mode, int adim[], int pdim[], char sequence[]);
  MgridInterpolation *cookInterpolationOff();
  MgridInterpolation *cookSequentialFastInterpolation(int _adim[], char _sequence[]);
  MgridInterpolation *cookSequentialHighInterpolation(int _adim[], char _sequence[]);
  MgridInterpolation *cookSequentialHighFitting(int _adim[], int _pdim[], 
						     char _sequence[]);
  MgridInterpolation *cookMultiDimInterpolation(int _adim[]);
  MgridInterpolation *cookMultiDimFitting(int _adim[], int _pdim[]);
  MgridInterpolation *cookInterpolation(unsigned mode, int _adim[] = 0, 
					 int _pdim[] = 0, char _sequence[] = 0);

  // Address calculation methods;
  int linearAddrToMulti(int linear, int id[3]);
  MgridCell *multiAddrToCellPtr(int id[3]);
  int multiAddrToLinear(int id[3]);
  int multiAddrToCombiCell(int id[], MgridCombiCell *qcell);
 public:
  // Ok, these two are used in my codes;
  int cellPtrToLinear(const MgridCell *cell) const;
  MgridCell *coordToCellPtr(double xx[]);
  MgridCell *linearAddrToCellPtr(int linear);
  int coordToMultiAddr(TVector3 &xx, int id[]);
  int coordToCombiCell(TVector3 &xx, MgridCombiCell *qcell);
  MgridCell *coordToCellPtr(TVector3 &xx);
  int linearAddrToCoord(int linear, TVector3 &xx);
  int multiAddrToCoord(int id[], TVector3 &xx);
  // And this call is used in import.cc;
  int compressedCoordToLinearAddr(TVector3 &xx);
  int cellPtrToCoord(MgridCell *cell, TVector3 &xx);

 private:
  // Well, this method should become public once I need it in user code;
  int changePosition(double shift[3], double theta[3], double phi[3]);

  int recalculatePosition();

  // Field value calculation routines;
 public:
  int getCylindricalFieldValue(TVector3 &xx, TVector3 &B);
  int getCartesianFieldValue(TVector3 &xx, TVector3 &B);
 private:
  int getFieldValue(t_3d_cs_vector *X, t_3d_cs_vector *D);

 private:
  int findSelfCell(TVector3 &xx);

  t_3d_cs_vector convertInputCoordinates(t_3d_cs_vector *X);

  int exportCore(FILE *fout, unsigned char contents);

  int getFixedTypeFieldValue(unsigned char system_type, TVector3 &xx, TVector3 &B);

  int suggestedPropertyArrayTail();

  void convertOutputField(t_3d_cs_vector *X, t_3d_cs_vector *D, 
			  unsigned char wanted_field_coord_system_type);

  // 'Useful' cell size in bytes; depends on the number             
  // of field components and on whether the error info is included; 
  // 'ram_cell_size' (what is actually stored in 'cells')           
  // may be bigger since it may contain extra pointers and          
  // intermediate data;                                             
  int cell_double_word_num, useful_cell_size, cell_xx_shift;
  int cell_ferr_shift, cell_fchi_shift;
  int cell_grad_shift, cell_1derr_shift, cell_3derr_shift;
  
  // It is convenient to keep this variable in the same structure;    
  // it defines scaling factor applied to the calculated field value; 
  double scaling_factor;

  // Make import.cc happy; fix later;
 public:
  // --- This stuff is used if this object  --- 
  // --- is a collection of daughter grids; --- 
  Mgrid **children;

  // --- This stuff is used if this object is a single grid; --- 
  
 private:
  // Expansion (mapping to 3dim) arrays for coordinates and field; 
  int *cexp, *fexp;

  // Total dimension of the 'cells' array; 
  int cell_num;
    // Actual size of cells stored in 'cells' array; may be bigger than 
    // mgrid->useful_cell_size; since grids may be expanded/squeezed    
    // dynamically (say during regriding, may want to store last actual 
    // 'ram_cell_size' as 'last_cell_size' in order to restore it later;
    // of course this mechanism will work for an ordered change/restore 
    // sequence (and not for say change/change/restore;                 
  int ram_cell_size, last_cell_size;
  // 1-dim array of grid cells; want a linear addressing     
  // because prefer to use a piece of memory directly (in    
  // the future share maps via HSM or a similar mechanism);  
  // t_mgrid_cell will be cutted down appropriately => makes 
  // no sense to define this as a t_mgrid_cell* array;       
  unsigned char *cells, *properties;

  // Coordinates of cell centers of cube edges; '[3]': 3 dimensions 
  // at most; ordering is in a natural XYZ (RFZ) sequence;          
  double *cell_center_coord[3];

  // Frame will be calculated out of mgrid->shift/theta/phi; if object 
  // needs to be positioned in a mother volume, this pointer will be   
  // non-zero --> need to convert input coordinates and output field;  
  MgridPosition *position;

  // Will default to the simple interpolation upon downloading; 
  MgridInterpolation *interpolation[3], *saved_interpolation[3];

  // 3dim index of cell containing xx[] point in grid array; 
  int self[3];
  MgridCombiCell self_qcell;

  // This is an additional return value; bitwise pattern   
  // describing interpolation result; reset to 0 and then  
  // calling subroutines will |= possible problems;        
  // lower 4 bits: self cell property; then 4 bits for the 
  // logical OR of the USED neighbour cells; then see the  
  // beginning of mvalue.c for other bits;                  
  unsigned last_field_status;

  // Make Runge-Kutta routines happy; fix later;
 public:
  // For MINUIT-like applications it is desirable to avoid any jumps     
  // in calculated values => it is better to keep cell structure used    
  // for fitting unchanged even if (by small xx[] shift inter->self cell 
  // actually changes; it is not completely fine (better to guarantee    
  // field differentiability by all conditions), but should work well;   
  int repetition_flag;
} ;

typedef double (*conversionfun)(double xx);

struct t_ascii_coord {
  // _CARTESIAN_/_CYLINDRICAL_ for now; 
  unsigned char system_type;

  // Number of coordinates; 
  int coord_num;

  // 'X', 'Y', 'Z', 'F', 'R', should be present 
  // in coord_names[] for this 'system_type';   
  char coord_names[3]; 

  // Function converting to 'cm' or 'kGs'; 
  conversionfun convfun;
} ;

#ifdef  __cplusplus
extern "C" {
#endif
  // User code external functions to create/import mgrids;
  Mgrid *import_field_map(char *file_name);

  Mgrid *create_single_mgrid_header(char *name, CoordSystem *coord, 
				      CoordSystem *field, MgridDirection *dir[3], 
				      unsigned cell_contents_bits);
  Mgrid *create_mgrid_heap_header(char *name, int field_calculation_method);
  
  Mgrid *import_ascii_field_map(char *file_name, char *mgrid_name, t_ascii_coord *coord, 
				  t_ascii_coord *field, int lines_to_skip);

  // Uncomment back if this routine is ever needed;
  //void release_cell_list_ram(MgridCell *cells);

  // This is indeed a service routine;
  int basic_types_match( void );
#ifdef  __cplusplus
}
#endif

#endif
