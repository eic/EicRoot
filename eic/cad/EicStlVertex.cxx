//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; 3D vertex class;
//

#include <EicStlVertex.h>

// =======================================================================================

EicStlKey *GetVertexArrayKey(const vEntry *vertices) 
{
  if (!vertices || !vertices->size()) return NULL;

  {
    unsigned counter = 0;
    double arr[vertices->size()*3];
      
    for(vEntry::const_iterator vt=vertices->begin(); vt!=vertices->end() ; vt++) 
      memcpy(arr + (counter++)*3, vt->second->key()->GetData(), 3*sizeof(double));

    return new EicStlKey(vertices->size()*3, arr);
  }
} // GetVertexArrayKey()

// =======================================================================================
