//
// AYK (ayk@bnl.gov), 2014/07/08
//
//  A native EIC reconstructed vertex class; 
//

#include <TObject.h>

#ifndef _EIC_RC_VERTEX_
#define _EIC_RC_VERTEX_

class EicRcVertex: public TObject {
 public:
  EicRcVertex() {};
  ~EicRcVertex() {};

  ClassDef(EicRcVertex, 1);
};

#endif
