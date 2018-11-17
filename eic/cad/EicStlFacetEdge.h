//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; facet edge class;
//

#include <EicStlVertex.h>

class EicStlFacet;
class EicStlAssembly;

#ifndef _EIC_STL_FACET_EDGE_
#define _EIC_STL_FACET_EDGE_

/// \brief A pair of 3D vertices defining one of the three facet edges in STL input file
///
/// \note Triangular STL facet is oriented, so vertex order matters!
class EicStlFacetEdge {
 public:
  /// Main constructor
  ///
  /// @param v1     "from" vertex 
  /// @param v2     "to" vertex (but order is irrelevant)
  EicStlFacetEdge(EicStlVertex * const v1, EicStlVertex * const v2);
  /// Destructor; just need to clean up the keys; 
  ///
  ~EicStlFacetEdge() { 
    // FIXME: unify with EicStlFacet;
    if (mKey) delete mKey; 

    if (mVertices) {
      mVertices->clear();
      delete mVertices;
    } //if
  };

  /// Access method; returns unique read-only STL key for this edge
  ///
  const EicStlKey *key()                          const { return mKey; };
  void AddFacet(EicStlFacet *facet)                     { mFacets.push_back(facet); };
  const std::vector<EicStlFacet*> &facets( void ) const { return mFacets; };
  const vEntry *vertices( void )                  const { return mVertices; };
 
 private:
  /*! Edge unique 2*3*4-byte STL key */
  EicStlKey *mKey;

  // Pointers to the two (ordered) vertices;
  vEntry *mVertices;

  /*! Array of facets to which this edge belongs */
  std::vector<EicStlFacet*> mFacets; 
};

#endif
