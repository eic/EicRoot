//
// AYK (ayk@bnl.gov), 2014/01/08
//
//  EicRoot CAD manipulation routines; facet assembly class;
//

#include <EicStlFacet.h>

#ifndef _EIC_STL_FACET_ASSEMBLY_
#define _EIC_STL_FACET_ASSEMBLY_

/// A collection of facets which compose a distinct solid object in the input STL file
class EicStlAssembly {
 public:
  /// Main constructor
  ///
  EicStlAssembly() {};
  /// Destructor
  ///
  ~EicStlAssembly() {};

  void AddFacet(EicStlFacet* facet, bool flip) { 
    mFacets.push_back(std::pair<EicStlFacet*, bool>(facet, flip));
  };
  const std::vector<std::pair<EicStlFacet*, bool> > &facets( void ) const { return mFacets; };

 private:
  /*! Facet pointers; vector arrangement suffices here */
  std::vector<std::pair<EicStlFacet*, bool> > mFacets;
};

#endif
