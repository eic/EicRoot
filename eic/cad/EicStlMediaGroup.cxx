//
// AYK (ayk@bnl.gov), 2014/01/08; revamped in Oct'2017;
//
//  EicRoot CAD manipulation routines; facets/vertices/etc with a given medium type;
//

#include <assert.h>

#include <TMath.h>
#include <TRotation.h>

#include <EicStlMediaGroup.h>

// =======================================================================================

EicStlMediaGroup::EicStlMediaGroup(): 
  mEdgeTotalOwnerCountOk(true), mEdgeSimpleSharingOk(true)
{
  mVertices = new vEntry(EicStlKeyCompare);
  mEdges    = new eEntry(EicStlKeyCompare);
  mFacets   = new fEntry(EicStlKeyCompare);
} // EicStlMediaGroup::EicStlMediaGroup()

// ---------------------------------------------------------------------------------------

void EicStlMediaGroup::AddVertexToCurrentGroup(std::vector<std::vector<EicStlVertex*> > &ngroups,
						   EicStlVertex *vtx)
{
  std::vector<EicStlVertex*> &current = ngroups[ngroups.size()-1];
  current.push_back(vtx);
  vtx->SetMergedFlag();

  for(unsigned iq=0; iq<vtx->DegenerateNeighbors3D().size(); iq++) {
    EicStlVertex *qtx = vtx->DegenerateNeighbors3D()[iq];
    if (!qtx->IsMerged()) AddVertexToCurrentGroup(ngroups, qtx);
  } //for iq
} // EicStlMediaGroup::AddVertexToCurrentGroup()

// ---------------------------------------------------------------------------------------

void EicStlMediaGroup::MergeDegenerateVertices(double tolerance)
{
  if (!tolerance) return;

  // Loop through all vertices once and find their 3D neighbors;
  for (vEntry::iterator vt=mVertices->begin(); vt!=mVertices->end(); vt++) {
    //printf("Next vertex!\n");
    EicStlVertex *vtx = vt->second;
    TVector3 vtx3d(vtx->key()->GetData()[0], vtx->key()->GetData()[1], vtx->key()->GetData()[2]);

    // FIXME: yes, do it, please (a loop, etc);
    {
      double xMin = vtx->key()->GetData()[0] - tolerance, xMax = vtx->key()->GetData()[0] + tolerance;
      std::multimap<double, EicStlVertex*>::iterator xLower = xCoord.lower_bound(xMin);
      std::multimap<double, EicStlVertex*>::iterator xUpper = xCoord.upper_bound(xMax);
      for(std::multimap<double, EicStlVertex*>::iterator ix = xLower; ix != xUpper; ix++) {
	EicStlVertex *qtx = ix->second;
	
	if (qtx != vtx) vtx->IncrementDegenerateNeighbor1D(qtx);//]++;
	//printf("  --> %f\n", qtx->key()->GetData()[0]);
      } //for ix
    }
    {
      double xMin = vtx->key()->GetData()[1] - tolerance, xMax = vtx->key()->GetData()[1] + tolerance;
      std::multimap<double, EicStlVertex*>::iterator xLower = yCoord.lower_bound(xMin);
      std::multimap<double, EicStlVertex*>::iterator xUpper = yCoord.upper_bound(xMax);
      for(std::multimap<double, EicStlVertex*>::iterator ix = xLower; ix != xUpper; ix++) {
	EicStlVertex *qtx = ix->second;
	
	if (qtx != vtx) vtx->IncrementDegenerateNeighbor1D(qtx);//]++;
	//printf("  --> %f\n", qtx->key()->GetData()[0]);
      } //for ix
    }
    {
      double xMin = vtx->key()->GetData()[2] - tolerance, xMax = vtx->key()->GetData()[2] + tolerance;
      std::multimap<double, EicStlVertex*>::iterator xLower = zCoord.lower_bound(xMin);
      std::multimap<double, EicStlVertex*>::iterator xUpper = zCoord.upper_bound(xMax);
      for(std::multimap<double, EicStlVertex*>::iterator ix = xLower; ix != xUpper; ix++) {
	EicStlVertex *qtx = ix->second;
	
	if (qtx != vtx) vtx->IncrementDegenerateNeighbor1D(qtx);//]++;
	//printf("  --> %f\n", qtx->key()->GetData()[0]);
      } //for ix
    }

    // Now check 3D distances for those neighbors, which are suspiciously close in all 3 projections;
    for(std::map<EicStlVertex*, unsigned>::const_iterator iq = vtx->DegenerateNeighbors1D().begin(); 
	iq != vtx->DegenerateNeighbors1D().end(); iq++) {
      EicStlVertex *qtx = iq->first;
      if (iq->second != 3) continue;

      TVector3 qtx3d(qtx->key()->GetData()[0], qtx->key()->GetData()[1], qtx->key()->GetData()[2]);
      double dist = (qtx3d - vtx3d).Mag();
      if (dist < tolerance) {
	printf("  ... %d %f\n", iq->second, dist);
	vtx->AddDegenerateNeighbors3D(qtx);
      } //if
    } //for iq
  } //for vt

  // Loop for the second time and construct neighbor classes;
  {
    std::vector<std::vector<EicStlVertex*> > ngroups;

    for (vEntry::iterator vt=mVertices->begin(); vt!=mVertices->end(); vt++) {
      EicStlVertex *vtx = vt->second;
      if (vtx->IsMerged() || !vtx->DegenerateNeighbors3D().size()) continue;
      
      {
	// Add empty vector and recursively add vertices to it; NB: since this 
	// vertex was not yet accounted in any neighbor group and neighbor 
	// assignment procedure was "symmetric" and neighbor array has non-zero
	// size, there definitely should be other unaccounted neighbors there;
	std::vector<EicStlVertex*> gr; 
	ngroups.push_back(gr);
	AddVertexToCurrentGroup(ngroups, vtx);
      }
    } //for vt

    //printf("%d\n", ngroups.size());
    for(unsigned iq=0; iq<ngroups.size(); iq++) {
      // NB: make calculation symmetric wrt the neighbor vertices;
      TVector3 vsum;

      for(unsigned iv=0; iv<ngroups[iq].size(); iv++) {
	EicStlVertex *qtx = ngroups[iq][iv];

	TVector3 vadd(qtx->key()->GetData()[0], qtx->key()->GetData()[1], qtx->key()->GetData()[2]);
	vsum += vadd;
      } //for iv
      
      vsum *= 1.0/ngroups[iq].size();

      //printf("  %d -> %d; %f %f %f\n", iq, ngroups[iq].size(), vsum[0], vsum[1], vsum[2]);

      // FIXME: do this better later;
      double buffer[3] = {vsum[0], vsum[1], vsum[2]};
      // Create brand new vertex; NB: per construction it can happen that such 
      // vertex does exist already; take this case into account;
      EicStlVertex *vnew = new EicStlVertex(buffer);
      if (mVertices->find(vnew->key()) == mVertices->end()) {
	(*mVertices)[vnew->key()] = vnew;
      } else {
	EicStlVertex *vcopy = (*mVertices)[vnew->key()];
	delete vnew;
	vnew = vcopy;
      } //if

      // Assign merged vertex pointers for neighbor group members;
      for(unsigned iv=0; iv<ngroups[iq].size(); iv++) {
	EicStlVertex *qtx = ngroups[iq][iv];

	qtx->SetMergedVertex(vnew);
      } //for iv
    } //for iq   
  }
} // EicStlMediaGroup::MergeDegenerateVertices()

// ---------------------------------------------------------------------------------------

void EicStlMediaGroup::CalculateFacets( void )
{
  // Loop through all buffered facets, calculate them (now that all vertices are known)
  // and allocate in an ordered map;
  for(unsigned fc=0; fc<mFbuffer.size(); fc++) {
    EicStlFacet *facet = mFbuffer[fc];
    
    // Facet is most likely degenerate -> kill it right away;
    if (facet->Calculate()) {
      delete facet;
      continue;
    } //if

    if (mFacets->find(facet->key()) == mFacets->end()) {
      // Facet is new -> allocate in the ordered map and set counter to 1;
      (*mFacets)[facet->key()] = facet;
      facet->SetCounter(1);
    } else {
      // Facet is duplicate (perhaps up to orientation) -> increment counter;
      mFacets->find(facet->key())->second->IncrementCounter();
      delete facet;
    } //if
  } //for fc

  // Do not need facet buffer any longer; 
  mFbuffer.clear();

  // Clean up unused vertices; FIXME: check this in-progress erase(), please;
  for (vEntry::iterator vt=mVertices->begin(); vt!=mVertices->end(); ) {
    EicStlVertex *vtx = vt->second;
  
    if (vtx->IsMerged() && vtx->GetMergedVertex() != vtx) {
      mVertices->erase(vt++);
      delete vtx;
    } 
    else
      vt++;
  } //for vt

  // Build vertex->facet relationships;
  for (fEntry::iterator fc=mFacets->begin(); fc!=mFacets->end(); fc++) {
    EicStlFacet *facet = fc->second;

    for(vEntry::const_iterator vt=facet->vertices()->begin(); vt!=facet->vertices()->end() ; vt++) 
      vt->second->AddFacet(facet);
  } //for fc
} // EicStlMediaGroup::CalculateFacets()

// ---------------------------------------------------------------------------------------

void EicStlMediaGroup::AllocateEdges( void )
{
  for (fEntry::iterator fc=mFacets->begin(); fc != mFacets->end(); fc++) {
    EicStlFacet *facet = fc->second;

    // Initialize edges; FIXME: this style here and below is awkward;
    EicStlVertex *vv[3];
    unsigned counter = 0;
    for(vEntry::const_iterator vt=facet->vertices()->begin(); vt!=facet->vertices()->end() ; vt++) 
      vv[counter++] = vt->second;

    EicStlFacetEdge *e1 = new EicStlFacetEdge(vv[0], vv[1]);
    EicStlFacetEdge *e2 = new EicStlFacetEdge(vv[1], vv[2]);
    EicStlFacetEdge *e3 = new EicStlFacetEdge(vv[2], vv[0]);
    EicStlFacetEdge *ee[3] = {e1, e2, e3};

    for(unsigned iq=0; iq<3; iq++) {
      EicStlFacetEdge *edge = ee[iq];
 
      if (mEdges->find(edge->key()) == mEdges->end()) {
	(*mEdges)[edge->key()] = edge;
      } else {
	ee[iq] = (*mEdges)[edge->key()];
	delete edge;
      } //if
     
      ee[iq]->AddFacet(facet);
    } //for iq

    for(unsigned iq=0; iq<3; iq++)
      facet->SetEdge(iq, ee[iq]);
  } //for fc

  for (eEntry::iterator ee=mEdges->begin(); ee != mEdges->end(); ee++) {
    EicStlFacetEdge *edge = ee->second;
  
    unsigned owner_counter = 0;
    for(unsigned fc=0; fc<edge->facets().size(); fc++)
      owner_counter += edge->facets()[fc]->GetCounter();
    
    if (owner_counter%2) {
      printf("-> %d\n", owner_counter);

      for (vEntry::const_iterator vt=edge->vertices()->begin(); vt!=edge->vertices()->end(); vt++) {
	EicStlVertex *vtx = vt->second;
      
	vtx->Print();
	//printf(" --> %7.2f %7.2f %7.2f\n", vtx->);
      } //fot vt
    }
    if (owner_counter%2)    mEdgeTotalOwnerCountOk  = false;
    if (owner_counter != 2) mEdgeSimpleSharingOk    = false;

    //printf("edge -> %d facet(s)\n", edge->facets.size());
  } //for ee
} // EicStlMediaGroup::AllocateEdges()

// ---------------------------------------------------------------------------------------

void EicStlMediaGroup::CoupleFacetToAssembly(EicStlFacet *facet, bool flip)
{
  // Facet accounted already -> sanity check;
  assert(facet->GetCounter());

  // Arrange direct and reverse links;
  EicStlAssembly *current = mAssemblies[mAssemblies.size()-1];
  current->AddFacet(facet, flip);

  facet->DecrementCounter();

  // Loop through all 3 edges, find matching facet (which is trivial in case of 
  // 'edge->mTotalOwnerCount == 2' and more complicated otherwise) and add it to this
  // same assembly recursively;
  for(unsigned iq=0; iq<3; iq++) {
    const EicStlFacetEdge *edge = facet->edge(iq);
    bool orientation = iq <= 1;

    if (edge->facets().size() == 2) {
      for(unsigned fc=0; fc<edge->facets().size(); fc++) {
	EicStlFacet *other_facet = edge->facets()[fc]; 
	
	if (/*other_facet == facet ||*/ !other_facet->GetCounter()) continue;
	
	bool edge_orientations_match = true;
	
	// FIXME: do it better later;
	for(unsigned ie=0; ie<3; ie++) {
	  const EicStlFacetEdge *other_edge = other_facet->edge(ie);
	  bool other_orientation = ie <= 1;
	  
	  if (other_edge == edge) {
	    edge_orientations_match = orientation != other_orientation;
	    break;
	  } //if
	} //for ie
	
	{
	  bool other_flip = (flip  && edge_orientations_match) || 
	    (!flip && !edge_orientations_match);

	  CoupleFacetToAssembly(other_facet, other_flip);
	  break;
	}
      } //for fc
    } else {
      assert(0);
    
      // Calculate edge 3D axis;
      TVector3 v1 = TVector3(edge->vertices()-> begin()->second->key()->GetData());
      TVector3 v2 = TVector3(edge->vertices()->rbegin()->second->key()->GetData());
      TVector3 axis = (v2 - v1).Unit();

      TVector3 n0 = (flip ? 1.0 : -1.0)*facet->GetNormal();
      TVector3 n02d = (n0 - n0.Dot(axis)*axis).Unit();

      //TVector3 nn[edge->facets.size()
      for(unsigned fc=0; fc<edge->facets().size(); fc++) {
	EicStlFacet *other_facet = edge->facets()[fc]; 
	
	if (/*other_facet == facet ||*/ !other_facet->GetCounter()) continue;
	
	bool edge_orientations_match = true;
	
	// FIXME: do it better later;
	for(unsigned ie=0; ie<3; ie++) {
	  const EicStlFacetEdge *other_edge = other_facet->edge(ie);
	  bool other_orientation = ie <= 1;
	  
	  if (other_edge == edge) {
	    //edge_orientations_match = facet->edges[iq].second != other_facet->edges[ie].second;
	    edge_orientations_match = orientation != other_orientation;
	    break;
	  } //if
	} //for ie
	
	{
	  bool other_flip = (flip  && edge_orientations_match) || 
	    (!flip && !edge_orientations_match);

	  TVector3 n1 = (other_flip ? 1.0 : -1.0)*other_facet->GetNormal();
	  TVector3 n12d = (n1 - n1.Dot(axis)*axis).Unit();

	  double alfa = acos(n12d.Dot(n02d));
	  printf("alfa: %f\n", (180.0/TMath::Pi())*alfa);
	  
	  TRotation rp, rm; rp.Rotate(alfa, axis); rm.Rotate(-alfa, axis); 
	  TVector3 dp = rp * n12d - n02d, dm = rm * n12d - n02d;
	  //printf("%f %f\n", dp.Mag(), dm.Mag());

	  //CoupleFacetToAssembly(other_facet, other_flip);
	  //break;
	}
      } //for fc
    } //if
  } //for ee
} // EicStlMediaGroup::CoupleFacetToAssembly()

// ---------------------------------------------------------------------------------------

int EicStlMediaGroup::SplitIntoAssemblies(double tolerance, bool easy)
{
  // First do easy fix: merge vertices, which are "too close" to each other;
  MergeDegenerateVertices(tolerance);

  // Calculate facets and build vertex->facet relationships;
  CalculateFacets();
  if (easy) return -1;

  // Then resolve edges and check generic model consistency;
  AllocateEdges();
  // Now if any of the edges has odd number of owners, we quit; FIXME: later on 
  // may want to try filling "triangular holes" as well; 
  if (!mEdgeTotalOwnerCountOk) return -1;
  printf("EicStlMediaGroup::SplitIntoAssemblies() -> ok#1\n");
  
  // FIXME: consider more complicated cases later; for now fall out if any edge is 
  // owned by more than two facets;
  //assert(mEdgeSimpleSharingOk); 
  if (!mEdgeSimpleSharingOk) return -1;
  printf("EicStlMediaGroup::SplitIntoAssemblies() -> ok#2\n");

  // The rest is a bit tricky and not necessarily most efficient; but all the algorithms
  // are (seemingly :-) linear in the number of facets, so why bother; we iteratively 
  // allocate facets into independent assemblies (assuming they do not overlap, so the 
  // surface orientation should be such, that it surrounds a finite volume) in the 
  // following way:
  //
  //  - take one of the (few) remaining facets, which is further away from the (0,0,0);
  //  - its orientation is "known" from geometric considerations;
  //  - attach recursively its neighbor facets, using simple edge ownership for edges
  //    owned by exactly two facets and geometric considerations for edges owned by 
  //    more than two facets (so 4, 6, etc);
  //
  // The procedure is repeated until "free" facet list is exhausted;
  //
  // One can indeed perform a check on "hole" volumes later; for now assume all 
  // volumes are not overlapping with each other;
  {
    // Create and populate <distance,vertex*> multimap;
    std::multimap<double, EicStlVertex*> distances;
    for (vEntry::iterator vt=mVertices->begin(); vt != mVertices->end(); vt++) {
      EicStlVertex *vtx = vt->second;

      //printf("%d facets at this vertex\n", vtx->facets.size()); 
      distances.insert(std::pair<double, EicStlVertex*>(vtx->DistanceToOrigin(), vtx));
    } //for vt
    printf("distances size: %d\n", distances.size()); //return 0;

    // Loop through the remaining vertices with "free" associated facets, until no one left;
    for( ; distances.size(); ) {
      //printf("size: %d\n", distances.size()); //return 0;

      // Yes, I need rbegin() here, so the largest disance from (0,0,0);
      EicStlVertex *vtx = distances.rbegin()->second;
      printf("distance: %f\n", distances.rbegin()->first);

      {
	// Calculate 3D unit vector from (0,0,0) to this vertex; I guess it can not 
	// happen that this vertex itself is (0,0,0), right?;
	TVector3 nv = TVector3(vtx->key()->GetData()).Unit();

	// Loop through all facets associated with this vertex and find the one with 
	// the normal closest to the 90 degrees to <nv> ; NB: at least one facet should be 
	// available (in fact at least three), otherwise vertex should have been removed
	// from the multimap already;
	std::multimap<double, EicStlFacet*> diffs;
	for(std::set<EicStlFacet*>::iterator fc = vtx->facets().begin(); fc != vtx->facets().end(); fc++) {
	  EicStlFacet *facet = *fc;

	  if (!facet->GetCounter()) continue;

	  double scal = fabs(nv.Dot(facet->GetNormal()));
	  //printf("scal: %f\n", scal);

	  diffs.insert(std::pair<double, EicStlFacet*>(scal, facet));
	} //for fc

	//printf("Here-0!\n");
	assert(diffs.size());
	//printf("There!\n");

	// Start new assembly;
	EicStlAssembly *assembly = new EicStlAssembly();
	mAssemblies.push_back(assembly);

	{
	  // Yes, I need rbegin() here (the largest scalar product), so the facet 
	  // which normal vector is as close to <nv> as possible; 
	  EicStlFacet *facet = diffs.rbegin()->second;

	  // And account this facet and all its neighbors in this assembly;
	  //printf("Here-1!\n");
	  CoupleFacetToAssembly(facet, nv.Dot(facet->GetNormal()) < 0.0);
	  //printf("Here-2!\n");
	}
      }

      // Purge remaining facets multimap;
      for(std::multimap<double, EicStlVertex*>::iterator vt=distances.begin(); 
	  vt!=distances.end(); ) {
	EicStlVertex *vtx = vt->second;
	
	bool free_facets_exist = false;
	for(std::set<EicStlFacet*>::iterator fc = vtx->facets().begin(); fc != vtx->facets().end(); fc++) {
	  EicStlFacet *facet = *fc;
	
	  if (facet->GetCounter()) {
	    free_facets_exist = true;
	    break;
	  } //if
	} //for fc

	if (!free_facets_exist) {
	  //printf("size: %d\n", distances.size());
	  distances.erase(vt++);
	}
	else
	  vt++;
      } //for it
    } //for inf
  }
  
  {
    printf("%d preliminary assembly(ies) found!\n", mAssemblies.size()); 

    for(unsigned ass=0; ass<mAssemblies.size(); ass++) {
      EicStlAssembly *assembly = mAssemblies[ass];

      printf("#%02d: %5d facet(s)\n", ass, assembly->facets().size());
    } //for ass
  } 

  return 0;
} // EicStlMediaGroup::SplitIntoAssemblies()

// =======================================================================================
