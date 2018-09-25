//
// AYK (ayk@bnl.gov), 2017/10/07
//
//  Eventually introduce oftenly used units; follow the G4 logic; 
//  do not want to introduce duplicate constants like just 'cm', but 
//  wrap them into the 'eic' namespace; NB: bear in mind that ROOT 
//  stores values in 'cm' (rather than in 'mm' like G4), so direct 
//  usage of G4 length definitions would not work anyway;
//

#ifndef _EIC_UNITS_
#define _EIC_UNITS_

namespace eic {
  // For now I need only convenient length units I guess;
  static const double   cm =   1.0;
  static const double   mm =   0.1;
  static const double    m = 100.0;
  static const double   um =  1E-4;
  static const double inch =  2.54;
}

#endif
