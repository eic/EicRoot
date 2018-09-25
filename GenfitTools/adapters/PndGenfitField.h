#ifndef PNDGENFITFIELD_H
#define PNDGENFITFIELD_H

#include"GFAbsBField.h"

/** @brief  Magnetic field
 *
 *  @author Promme (Prometeusz Jasinski)
 * 
 */

class PndGenfitField : public GFAbsBField
{

 public:
  //! define the field in this ctor (?)
  PndGenfitField();

  //! return value at position
  TVector3 get(const TVector3& pos) const;
  
 private:
  
};

#endif
