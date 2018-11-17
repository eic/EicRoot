//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; facet edge class;
//

#include <EicStlFacetEdge.h>

// =======================================================================================

EicStlFacetEdge::EicStlFacetEdge(EicStlVertex * const v1, EicStlVertex * const v2)
{
  // Allocate vertices and store them ordered according to their mKey's;
  mVertices = new vEntry(EicStlKeyCompare);
  (*mVertices)[v1->key()] = v1; (*mVertices)[v2->key()] = v2; 

  // Calculate key; 
  mKey = GetVertexArrayKey(mVertices);
} // EicStlFacetEdge::EicStlFacetEdge()

// =======================================================================================
