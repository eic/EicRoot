/* -------------------------------------------------------------------------- */
/*  CoordSystem.h                                                             */
/*                                                                            */
/*  A.Kisselev, PNPI, St.Petersburg, Russia.                                  */
/*    e-mail: kisselev@hermes.desy.de                                         */
/* -------------------------------------------------------------------------- */

#include <3d.h>

// Well, want easy access to __u64 definition; think later;
#include <MgridDirection.h>

#ifndef _COORD_SYSTEM_
#define _COORD_SYSTEM_

// Prefer users to specify axes by name; just in order to avoid     
// bad mistakes like specifying _R_ coordinate for _CARTESIAN_      
// system (which in this case will be silently interpreted as _X_);  
struct t_coord_name {
  // Name --> 'X', 'Y', 'Z', 'F', 'R'; 
  char name;
  // ID in the interval 0..2; 
  int id;
  // Bitwise OR of allowed system types; 
  unsigned char allowed_systems;
} ;

// NB: 2-dim grids (like RZ-grid for the recoil detector        
// simulation) will be expanded into 3-dim grids setting 3-d    
// dimension - number of cells - to '1'; NB: next entry         
// should be 0x04 (since 'coord_names[]' uses bitwise pattern);  
#define _CARTESIAN_                    0x01
#define _CYLINDRICAL_                  0x02

//
// -> never change data fields order, etc in this class!;
//

//class Mgrid;

class CoordSystem {
  friend class Mgrid;
  friend class MgridInterpolation;

  //friend Mgrid *create_mgrid_heap_header(char *name, int field_calculation_method);

 public:
  // Again, is the place holder usage correct for mgrid import?;
  CoordSystem() {};
  // Meaningful constructor;
  CoordSystem(unsigned char _system_type, int _coord_num, char 
	      _coord_names[]);

  // Wan to keep "system_type" private; no range check?; do enum later;
  void setSystemType(unsigned type) { system_type = type;};

  unsigned getCoordNum() { return coord_num;};
  void setCoordNum(unsigned num) { coord_num = num;};

 private:
  int calculateExpansionRules(int **expansion);
  void projectToLocalCoordinates(double in[], double out[]);

  // _CARTESIAN_/_CYLINDRICAL_ for now; do not mind to spend 8 bytes; 
  __u64 system_type;

  // Number of coordinates; yes, it's redundant (see 'fake[]' field); 
  __u64 coord_num;

  // Make life easier ...;
 public:
  // If '1', this coordinate is missing; natural XYZ (RFZ) sequence; 
  __u64 fake[3];
} ;

#ifdef  __cplusplus
extern "C" {
#endif
  t_coord_name *find_coord_by_name(unsigned char system_type, char name);

  void expand_to_global_coordinates(int ldim, TVector3 &in, TVector3 &out, int l2g[]);
#ifdef  __cplusplus
}
#endif

#endif
