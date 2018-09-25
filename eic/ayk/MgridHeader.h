/* -------------------------------------------------------------------------- */
/*  MgridHeader.h                                                             */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <3d.h>

#include <MgridDirection.h>
#include <CoordSystem.h>

// MgridHeader is filled with __u64 (or __s64) in order to allow         
// compatibility with 64-bit machines (like Opterons); the problem   
// is that Intel architecture requires alignment of variables, in    
// particular elements in structures are aligned on 4-byte (32-bit   
// arch) or 8-byte (64-bit arch) boundary; sizeof(double)=8 on both; 
// so in order to avoid gaps on 64-bit architecture I'm forced to    
// use 8-bit integeres in t_mgrid (and also to keep 'property'       
// field of cells in a separate (char) array);    

#ifndef _MGRID_HEADER_H
#define _MGRID_HEADER_H

// These 8 bytes are a usual magic signature; it            
// is a sort of consistency check for a given file; to a    
// certain extent it is also a check whether machine is a   
// little or big endian (or something else); namely file    
// created on i386 will NOT be recognized on mips; this is  
// ok; if usage of a certain map file on a big endian       
// machine becomes of interest, one can figure out machine  
// endianness out of 'magic' bytes order and extend loading 
// by appropriate byte swapping; I do not see this becomes  
// necessary any time soon; usage on a 64-bit machine like  
// AMD Opteron is certainly possible on the other hand;     
// (as long as __u64 and double types match); different size
// of pointers does not matter since they are not used in   
// either of t_mgrid or t_mgstack structures;               
//#define _MFILE_MAGIC_ 0x91FA011B1122FE43
// Well, check please, that this "ull" suffix really works in I/O;
#define _MFILE_MAGIC_ 0x91FA011B1122FE43ull

// Well, th eonly format version for now (and forever);
#define _MGRID_FORMAT_ID_              0x00

//
//  NB: MgridHeader size should never change; format may vary, but 
//  then format_id variable should reflect this and I/O adjusted 
// accordingly;  
//

// Please never change!; 128=8*16, so 8-byte alignment is Ok; 
#define MGRID_NAME_LENGTH_MAX 128

// Root node of a collection of daughter mgrids; 
//#define _MGRID_HEAP_         0x00
// 'Usual' 1..3 dim rectangular grid; 
//#define _RECTANGULAR_MGRID_  0x01

enum MgridType {_MGRID_HEAP_, _RECTANGULAR_MGRID_};

// Possible creation methods; 
#define _FROM_SCRATCH_       0x00
#define _ASCII_INPUT_        0x01
#define _REGRID_             0x02

// Possible ways of field calculation in a _HEAP_;        
// _SUPERPOSITION_ means fields from all daughters        
// with ret=0 will be summed up; _FIRST_MATCH_ means      
// the very first ret=0 will be taken (this may be useful 
// for a collection of overlapping map pieces);           
#define _SUPERPOSITION_      0x00
#define _FIRST_MATCH_        0x01

//
// -> never change data fields order, etc in this class!;
//

// This header will be dumped   
// into a file; it is customed in a machine-independent way (as long as  
// 1) __u64 and double size match, 2) 128-bit archs are not around,      
// 3) there is no byte swapping; the latter which can be implemented     
// if needed; never insert anything between magic_header and             
// magic_trailer!; if extension needed, backward compatibility should    
// be always preserved by adding more types of 'format_id&type';         
// additional fields may be taken from reserved[] (in 8-byte portions!); 
class MgridHeader {
  friend class MgridInterpolation;

 public:
  const char *getName() { return name;};
  // Please, range check later;
  void setCreationMethod(unsigned method) { creation_method = method;};

 protected:
  // Well, even if a single mgrid is a part of a stack, want 
  // to keep this header in because prefer to keep single    
  // grids completely independent on the context (so that    
  // a very general function called with address of a single 
  // grid can figure out what it is and behave accordingly); 
  __u64 magic_header;
  // Layout is not expected to change ever; if for wnatever reason    
  // one wants to do extensions, it is perfectly possible to increase 
  // format identifier in this field, use reserved[] space and put a  
  // switch() statement in the downloading code;                       
  __u64 format_id;

  // _MGRID_HEAP_, _RECTANGULAR_MGRID_; yes, want this __u64;
  __u64 __type;

  // Well, it's the time of create_general_mgrid_header() call; 
  __u64 creation_time;
  // Method (_FROM_SCRATCH_, _ASCII_INPUT_, etc); 
  __u64 creation_method;
  // In case of regriding store parent name as well as it's type; 
  char parent_name[MGRID_NAME_LENGTH_MAX];
  __u64 parent_type;

  // Grid name; NB: MGRID_NAME_LENGTH_MAX is a multiple of 8!; 
  char name[MGRID_NAME_LENGTH_MAX];

  // Number of daughter-objects  
  __u64 child_num;

  // Make life easier for now;
 public: 
  // A way of calculating field in case of _MGRID_HEAP_ 
  // (_SUPERPOSITION_/_FIRST_MATCH_);
  __u64 field_calculation_method;

 protected:
  // This field describes which blocks are stored for this object; 
  __u64 object_contents_bits;

  // Make life easier for now;
 public:
  // These bits describe what is actually kept in cells; options         
  // for now are 1) N field components, 2) N field component errors,     
  // 3) N chi^2's of the fits, 4) M cell coordinates; useful_cell_length 
  // is calculated based on this pattern and coord.coord_num,            
  // field.coord_num;                                                    
  __u64 cell_contents_bits;

  // Describes what is actually stored in this grid;                     
  // Actual number of *stored* field components (may be 2 for RZ-grid or 
  // even 1 if just one component was measured); once again: field       
  // calculation routines will always expand returned field value to     
  // dimension '3'; the way field will be expanded from the stored data  
  // is strictly defined by the combination of field coordinate system   
  // type, dimension and number of components;                           
  CoordSystem coord, field;

  // '3': natural XYZ (RFZ) sequence; does not make sense to put this     
  // stuff into CoordSystem since it is not used for field components; 
  MgridDirection dir[3];

 protected:
  // Shift of this object with respect to it's father reference system; 
  // Can not use ThreeDeeVector in this binary fixed format header!;
  double shift[3];
  // GEANT-like rotation parameters; this is probably not the most wise         
  // method to introduce 3 independent variables; but certainly these variables 
  // is easy to derive out of any other more convenient (say for fitting)       
  // parametrization, where one would like to have just 3 small rotations       
  // around X,Y,Z axes;                                                         
  double theta[3], phi[3];

  // Reserve few bytes for possible further extensions; does not 
  // matter on a level of ~MB required to store the grid itself; 
  // NB: usage should proceed in 8-byte portions!;               
  unsigned char reserved[1024];

  // This is the last 'persistent' word in this structure; 
  // it will be kept into binary file including this word;  
  __u64 magic_trailer;
} ;

#endif
