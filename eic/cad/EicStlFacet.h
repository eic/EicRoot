//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; STL facet class;
//

#include <TVector3.h>

#include <EicStlVertex.h>

class EicStlFacetEdge;
class EicStlAssembly;

#ifndef _EIC_STL_FACET_
#define _EIC_STL_FACET_

/// \brief Three 3D vertices defining one of the facets in STL input file
///
/// \note Triangular STL facet is oriented, but only during assmebly creation; 
/// vertices are sitting in the vEntry map (see below) in their EicStlKey order;
/// as of Oct'2017 there is no more concept of mirror vertices (and mirror edges), 
/// but orientation is considered undefined upon export (in other words imported 
/// STL files are not trusted for consistency);
class EicStlFacet {
 public:
  /// Main constructor
  ///
  /// @param v1     1-st vertex 
  /// @param v2     2-d vertex 
  /// @param v3     3-d vertex 
  EicStlFacet(EicStlVertex * const v1, EicStlVertex * const v2, EicStlVertex * const v3);
  /// Destructor; just need to clean up the keys
  ///
  ~EicStlFacet() { 
    if (mVertices) {
      mVertices->clear();
      delete mVertices;
    } //if

    if (mKey) delete mKey; 
  };

  /// Access method; returns unique read-only STL key for this facet
  ///
  const EicStlKey *key()                         const { return mKey;};

  /// Printout method; basically exploits \ref EicStlVertex printout
  ///
  //void Print() const { vertices[0]->Print(); vertices[1]->Print(); vertices[2]->Print(); };

  // Basic calculations of the facet (normal, etc);
  int Calculate();

  void SetCounter(unsigned counter)                    { mCounter = counter; };
  void DecrementCounter( void )                        { mCounter--; };
  void IncrementCounter( void )                        { mCounter++; };
  unsigned GetCounter( void )                    const { return mCounter; };

  const vEntry *vertices( void )                 const { return mVertices; };
  const TVector3 &GetNormal( void )              const { return mNormal; };

  void SetEdge(unsigned iq, EicStlFacetEdge *edge) {
    if (iq <= 2) mEdges[iq] = edge;
  };
  const EicStlFacetEdge *edge(unsigned iq)       const { return (iq <= 2 ? mEdges[iq] : NULL); };

  //private:
  // Intermediate vector of vertices; since vertices can be merged later, ordering is 
  // not known at a time of the STL file import;
  std::vector<EicStlVertex*> mVbuffer;

  /*! Facet unique 3*3*8-byte STL key */
  EicStlKey *mKey;
  /*! Normal to this facet, calculated by hand as a (v2-v1)x(v3-v1) vector product */
  TVector3 mNormal;

  // Vertex pointers; they are ordered in vertex mKey keys;
  vEntry *mVertices;

  // Edge pointers; since they are v1->v2, v2->v3 & v3->v1 in 1-2-3 order as in the 
  // "mVertices" map (see above), first two edges are always oriented "in sync" with the 
  // facet normal while the third one is "out of sync", because condition 
  // "mKey(v3)>mKey(v1)" is always "true";
  EicStlFacetEdge *mEdges[3];

  // There can be more than one identical facet; as long as this running counter
  // is greater than 1, assembly builder can attach it to the current assembly;
  unsigned mCounter;
};

#endif
