//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; 3D vertex class;
//

#include <math.h>

#include <map>
#include <set>
#include <vector>

#include <cstdio>

#include <EicStlKey.h>

class EicStlFacet;

#ifndef _EIC_STL_VERTEX_
#define _EIC_STL_VERTEX_

/// 3D vertex defined in STL input file
class EicStlVertex {
 public:
  /// Main constructor
  ///
  /// @param coord     vertex 3D coordinates
 EicStlVertex(const double coord[3]): mMerged(false), mMergedVertex(NULL) {
    mKey = new EicStlKey(3, coord);

    mDistanceToOrigin = sqrt(coord[0]*coord[0]+coord[1]*coord[1]+coord[2]*coord[2]);
  };
  /// Destructor; just need to clean up the key; 
  ///
  ~EicStlVertex() { if (mKey) delete mKey; };

  // Access methods;
  const EicStlKey *key()                const { return mKey;};
  double DistanceToOrigin( void )       const { return mDistanceToOrigin; };
  void SetMergedFlag( void )                  { mMerged = true; };
  void SetMergedVertex(EicStlVertex *vertex)  { mMergedVertex = vertex; };
  bool IsMerged( void )                 const { return mMerged; };       
  EicStlVertex *GetMergedVertex( void ) const { return mMergedVertex; };

  void AddFacet(EicStlFacet *facet)           { mFacets.insert(facet); };
  std::set<EicStlFacet*> &facets( void )      { return mFacets; };

  // This is trivial indeed; prefer to avoid using 'friends' though;
  void IncrementDegenerateNeighbor1D(EicStlVertex* vertex) {
    mDegenerateNeighbors1D[vertex]++;
  };
  const std::map<EicStlVertex*, unsigned> &DegenerateNeighbors1D( void ) { 
    return mDegenerateNeighbors1D; 
  };
  void AddDegenerateNeighbors3D(EicStlVertex* vertex) {
    mDegenerateNeighbors3D.push_back(vertex);
  };
  const std::vector<EicStlVertex*> &DegenerateNeighbors3D( void ) {
    return mDegenerateNeighbors3D;
  };

 private:
  /*! Vertex unique 3*8-byte STL key */
  EicStlKey *mKey;

  // Vertex distance to (0,0,0);
  double mDistanceToOrigin;

  // Facets joining in this vertex;
  std::set<EicStlFacet*> mFacets; 

  // This stuff is used to cure STL file in case few neighbor vertices are 
  // artificially displaced within tolerances from each other; 
  bool mMerged;
  EicStlVertex *mMergedVertex;
  std::map<EicStlVertex*, unsigned> mDegenerateNeighbors1D;
  std::vector<EicStlVertex*> mDegenerateNeighbors3D;

  /// Printout method; do not mind to use fixed format
  ///
 public:
  void Print() const { 
    printf("%16.8f %16.8f %16.8f\n", 
	   mKey->GetData()[0], mKey->GetData()[1], mKey->GetData()[2]);
  };
};

typedef std::map<const EicStlKey*, EicStlVertex*, bool(*)(const EicStlKey*, const EicStlKey*)> vEntry;

// Yes, prefer to have this call as a standalone function;
EicStlKey *GetVertexArrayKey(const vEntry *vertices);

#endif
