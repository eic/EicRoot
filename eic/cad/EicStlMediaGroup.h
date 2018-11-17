//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; facets/vertices/etc with a given medium type;
//

#include <TGeoMedium.h>

#include <EicStlFacetEdge.h>
#include <EicStlFacetAssembly.h>

#ifndef _EIC_STL_MEDIA_GROUP_
#define _EIC_STL_MEDIA_GROUP_

typedef std::map     <const EicStlKey*, EicStlFacetEdge*,      bool(*)(const EicStlKey*, const EicStlKey*)> eEntry;
typedef std::map     <const EicStlKey*, EicStlFacet*,          bool(*)(const EicStlKey*, const EicStlKey*)> fEntry;
// Edge chains are not necessarily unique -> multimap;
//typedef std::multimap<const EicStlKey*, EicStlFacetEdgeChain*, bool(*)(const EicStlKey*, const EicStlKey*)> cEntry;

class EicStlMediaGroup {
 public:
  EicStlMediaGroup();
  ~EicStlMediaGroup() {};

  const vEntry *vertices( void )                         const { return mVertices; };
  const eEntry *edges( void )                            const { return mEdges; };
  const fEntry *facets( void )                           const { return mFacets; };
  const std::vector<EicStlAssembly*> &assemblies( void ) const { return mAssemblies; };

  void AddVertex(EicStlVertex *vertex)                         { (*mVertices)[vertex->key()] = vertex; };

  int SplitIntoAssemblies(double tolerance = 0.0, bool easy = false);

  std::multimap<double, EicStlVertex*> xCoord, yCoord, zCoord;
  std::vector<EicStlFacet*> mFbuffer;

 private:

  /*! Ordered map of vertices in this STL file */
  vEntry *mVertices; //!
  /*! Ordered map of vertex edges in this STL file */
  eEntry *mEdges; //!
  /*! Ordered map of facets in this STL file */
  fEntry *mFacets; //!
  /*! Array of identified independent assemblies */
  std::vector<EicStlAssembly*> mAssemblies; //!
  /*! Ordered multimap of edge chains in this STL file; well, they are not necessarily unique? */
  //cEntry *chains; //!

  void MergeDegenerateVertices(double tolerance);
  void AddVertexToCurrentGroup(std::vector<std::vector<EicStlVertex*> > &ngroups,
			       EicStlVertex *vtx);
  void CalculateFacets( void );
  void AllocateEdges( void );

  // As long as every edge has even number of owner facets the model is Ok up 
  // to the facet orientation; once every edge has equal number of facets, which 
  // want to own it in direct and reversed orientation, we are all happy; if in 
  // addition to this each edge is owned by only two facets, the model does not 
  // require more sophisticated facet orientation checks;
  bool mEdgeTotalOwnerCountOk, mEdgeSimpleSharingOk;

  /// Couple facet to the currently populated assembly
  ///
  /// @param facet  facet pointer
  void CoupleFacetToAssembly(EicStlFacet *facet, bool flip);
};

// The idea in having std::pair<unsigned, const TGeoMedium*> instead of just 
// 'const TGeoMedium*' as a key is to possibly benefit from having multiple 
// "solid - endsolid" clauses in input STL file (when user clearly knows that
// certain fraction of a file belong to different solid volume assemblies; this 
// would for sure make sense if one merge several ASCII STL files into one; 
typedef std::map<std::pair<unsigned, const TGeoMedium*>, EicStlMediaGroup> gEntry;

#endif
