//
// AYK (ayk@bnl.gov), 2014/03/11; revamped in Oct'2017;
//
//  EicRoot CompositeShape ROOT shape; NB: several sections of commented out 
//  debugging stuff were wiped out of this file on 2017/10/04;
//

#include <assert.h>

#include <TVirtualGeoPainter.h>
#include <TVirtualViewer3D.h>
#include <TBuffer3D.h>
#include <TBuffer3DTypes.h>
#include <TGeoBoolNode.h>
#include <TGeoManager.h>
#include <TPad.h>

#ifdef _OPENCASCADE_
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

#include <StlAPI_Writer.hxx>

//#include <EicStlEdge.h>
#include <EicStlFactory.h>

typedef std::map<const EicStlKey*, unsigned, bool(*)(const EicStlKey*, const EicStlKey*)> qEntry;
#endif

#include <EicCompositeShape.h>

// =======================================================================================

int EicCompositeShape::LocalFillBuffer3D(double stl_quality_coefficient)
{
  // Yes, basically a dummy call in case if OpenCascade libraries are not available; 
  // NB: this call is needed only on the EicRoot side (where object is created);
  // for other purposes like visualization it is not really needed;
#ifdef _OPENCASCADE_
  // For less typing;
  EicBuffer3D *lb = mLocalBuffer3D;

  // Ok, first fill out the bounding box entries;
  {
    Bnd_Box Boundary;
    BRepBndLib::Add(*mSolid, Boundary);

    Boundary.Get(lb->xMin, lb->yMin, lb->zMin, lb->xMax, lb->yMax, lb->zMax);
  }

  // Pass this solid through OpenCascade STL maker; unify with dumpAsStlSolid() later;
  {
    char buffer[128];
    snprintf(buffer, 128-1, "./%05d-tmp.stl", getpid()); 

    {
      StlAPI_Writer writer; 

      BRepMesh_IncrementalMesh(*mSolid, stl_quality_coefficient);
 
      writer.ASCIIMode() = false;
      writer.Write(*mSolid, buffer);
    }

    {
      EicStlFactory stl_factory("", buffer, 0);

      // And now figure out imported vertex/edge/facet count and fill the 3D buffer;
      // assume no fixes are needed (reconsider later); only one 'mgroup' in fact, so 
      // no real loop here;
      //printf("%d mgroup(s)\n", stl_factory.mgroups().size());
      for (gEntry::iterator it=stl_factory.mgroups().begin(); it!=stl_factory.mgroups().end(); it++) {
	EicStlMediaGroup *mgroup = &it->second;

	//
	// FIXME: unify the codes in the below if();
	//

	// Split into assemblies and make sure there is only one; otherwise (if split 
	// procedure failed) fall back to "visually-correct-only" version;
#if _TODAY_
	if (mgroup->SplitIntoAssemblies()) {
#endif
	  mgroup->SplitIntoAssemblies(0.0, true);
	  lb->fNbPnts = mgroup->facets()->size() * 3;
	  lb->fNbSegs = mgroup->facets()->size() * 3;
	  lb->fNbPols = mgroup->facets()->size();

	  // '3': XYZ per point;
	  lb->fPntsCapacity = lb->fNbPnts * 3;
	  lb->fPnts = new Double_t[lb->fPntsCapacity];
	  // '2': end point indices;
	  lb->fSegsCapacity = lb->fNbSegs * 2;
	  lb->fSegs = new Int_t   [lb->fSegsCapacity];
	  // '3': edge indices; 
	  lb->fPolsCapacity = lb->fNbPols * 3;
	  lb->fPols = new Int_t   [lb->fPolsCapacity];

	  unsigned vCounter = 0, eCounter = 0, fCounter = 0;
	  //printf("--> %d\n", mgroup->facets()->size());
	  for(fEntry::const_iterator ft=mgroup->facets()->begin(); ft!=mgroup->facets()->end(); ft++) { 
	    EicStlFacet *facet = ft->second;
	    
	    // Allocate facet edge indices;
	    lb->fPols[3*fCounter+0] = eCounter+0;
	    lb->fPols[3*fCounter+1] = eCounter+1;
	    lb->fPols[3*fCounter+2] = eCounter+2;
	    fCounter++;

	    // Allocate edge endpoint indices;
	    lb->fSegs[2*eCounter+0] = vCounter+0;
	    lb->fSegs[2*eCounter+1] = vCounter+1;
	    eCounter++;
	    lb->fSegs[2*eCounter+0] = vCounter+1;
	    lb->fSegs[2*eCounter+1] = vCounter+2;
	    eCounter++;
	    lb->fSegs[2*eCounter+0] = vCounter+2;
	    lb->fSegs[2*eCounter+1] = vCounter+0;
	    eCounter++;

	    // Allocate vertices; use "natural" ordering as it was given in the STL file dump;
	    for(unsigned vt=0; vt<3; vt++) {
	      EicStlVertex *vertex = facet->mVbuffer[vt];
	      const double *arr = vertex->key()->GetData();
	      
	      for(unsigned iq=0; iq<3; iq++)
		lb->fPnts[vCounter*3 + iq] = arr[iq]; 
	      
	      vCounter++;
	    } //for vt
	  } //for ft
	  //printf("%d %d %d\n", vCounter, eCounter, fCounter);
#if _TODAY_
	} else {
	  if (mgroup->assemblies().size() != 1) 
	    printf("mgroup->assemblies().size(): %d\n", mgroup->assemblies().size());
	  assert(mgroup->assemblies().size() == 1);
	  
	  //for(unsigned ass=0; ass<mgroup->assemblies.size(); ass++) {
	  //EicStlAssembly *assembly = mgroup->assemblies[ass];
	  EicStlAssembly *assembly = mgroup->assemblies()[0];
	  
	  // For now assume vertices can not disappear during healing procedure;
	  lb->fNbPnts = mgroup->vertices()->size();
	  // Triangular facets, each edge is shared by two facets;
	  lb->fNbSegs = assembly->facets().size() * 3 / 2;
	  lb->fNbPols = assembly->facets().size();
	  
	  //printf("vtx count: %d; edge count: %d; facet count: %d\n",
	  //     mgroup->vertices()->size(), lb->fNbSegs, lb->fNbPols);
	  
	  // '3': XYZ per point;
	  lb->fPntsCapacity = lb->fNbPnts * 3;
	  lb->fPnts = new Double_t[lb->fPntsCapacity];
	  // '2': end point indices;
	  lb->fSegsCapacity = lb->fNbSegs * 2;
	  lb->fSegs = new Int_t   [lb->fSegsCapacity];
	  // '3': edge indices; 
	  lb->fPolsCapacity = lb->fNbPols * 3;
	  lb->fPols = new Int_t   [lb->fPolsCapacity];
	  
	  {
	    unsigned eCounter = 0, vCounter = 0;
	    qEntry edgeKeys(EicStlKeyCompare), vtxKeys(EicStlKeyCompare); 
	    
	    // Loop through all facets and assign segment (edge) indices;
	    for(unsigned fc=0; fc<assembly->facets().size(); fc++) {
	      EicStlFacet *facet = assembly->facets()[fc].first;
	      bool orientation = assembly->facets()[fc].second;
	      
	      for(unsigned ee=0; ee<3; ee++) {
		const EicStlFacetEdge *edge = facet->edge(ee);
		  
		if (edgeKeys.find(edge->key())  == edgeKeys.end()) {
		  // New edge -> check vertices;
		  unsigned evCounter = 0;
		  for(vEntry::const_iterator vt=edge->vertices()->begin(); vt!=edge->vertices()->end() ; vt++) { 
		    EicStlVertex *vertex = vt->second;
		    
		    if (vtxKeys.find(vertex->key()) == vtxKeys.end()) {
		      const double *arr = vertex->key()->GetData();
		      
		      // Vertex coordinates; FIXME: (obsolete) float -> double;
		      for(unsigned iq=0; iq<3; iq++)
			lb->fPnts[vCounter*3 + iq] = arr[iq]; 
		      
		      vtxKeys[vertex->key()] = vCounter++;
		    } //if
		    
		    // Segment entry;
		    lb->fSegs[eCounter*2 + evCounter++] = vtxKeys[vertex->key()];
		  } //for vv
		  
		  edgeKeys[edge->key()] = eCounter++;
		} //if
		
		// Facet entry;
		lb->fPols[fc*3 + (orientation ? 2-ee : ee)] = edgeKeys[edge->key()];
	      } //for ee
	    } //for fc
	    
	    //printf("%d %d\n", edgeKeys.size(), vtxKeys.size()); exit(0);
	  }
	} //if
#endif
      } //for it
    }

    //unlink(buffer);
  }
#endif

  return 0;
} // EicCompositeShape::LocalFillBuffer3D()

// ---------------------------------------------------------------------------------------

void EicCompositeShape::LocalSetPoints(Double_t *points) const
{
  // Fill box points.
  if (!points) return;

  EicBuffer3D *lb = mLocalBuffer3D;
  
  for(unsigned pt=0; pt<lb->fNbPnts; pt++)
    for(unsigned iq=0; iq<3; iq++)
      points[pt*3+iq] = lb->fPnts[pt*3+iq];
} // EicCompositeShape::LocalSetPoints()

// ---------------------------------------------------------------------------------------

void EicCompositeShape::LocalSetSegsAndPols(TBuffer3D &buff) const
{
  // Fills TBuffer3D structure for segments and polygons.
  Int_t c = GetBasicColor();

  //printf("@@@ basic color -> %d\n", GetBasicColor()); 
  EicBuffer3D *lb = mLocalBuffer3D;
  
  for(unsigned sg=0; sg<lb->fNbSegs; sg++) {
    buff.fSegs[sg*3] = c;

    for(unsigned iq=0; iq<2; iq++)
       buff.fSegs[sg*3+iq+1] = lb->fSegs[sg*2+iq];
  } //for 

  for(unsigned fc=0; fc<lb->fNbPols; fc++) {
    buff.fPols[fc*5+0] = c;
    buff.fPols[fc*5+1] = 3;

    // Change segment sequence (orientation) to make ROOT happy; 
    for(unsigned iq=0; iq<3; iq++)
       buff.fPols[fc*5+iq+2] = lb->fPols[fc*3+2-iq];
  } //for 
} // EicCompositeShape::LocalSetSegsAndPols()

// ---------------------------------------------------------------------------------------

//
// Follow the strange logic to split this stuff into FillBuffer3D() and GetBuffer3D();
//

void EicCompositeShape::FillBuffer3D(TBuffer3D & buffer, Int_t reqSections, Bool_t localFrame) const
{
  // Fills the supplied buffer, with sections in desired frame
  // See TBuffer3D.h for explanation of sections, frame etc.
  TGeoShape::FillBuffer3D(buffer, reqSections, localFrame); 

  // For less typing;
  EicBuffer3D *lb = mLocalBuffer3D;

  double origin[3] = {
    (lb->xMin + lb->xMax)/2., 
    (lb->yMin + lb->yMax)/2., 
    (lb->zMin + lb->zMax)/2.};
  double halfLengths[3] = {
    (lb->xMax - lb->xMin)/2., 
    (lb->yMax - lb->yMin)/2., 
    (lb->zMax - lb->zMin)/2.};

   if (reqSections & TBuffer3D::kBoundingBox) {
     buffer.SetAABoundingBox(origin, halfLengths);

     if (!buffer.fLocalFrame) {
       TransformPoints(buffer.fBBVertex[0], 8);
     } //if

     buffer.SetSectionsValid(TBuffer3D::kBoundingBox);
   } //if
} // EicCompositeShape::FillBuffer3D()

// ---------------------------------------------------------------------------------------

const TBuffer3D &EicCompositeShape::GetBuffer3D(Int_t reqSections, Bool_t localFrame) const
{
  // Ok, leave this buffer static as it was in the original ROOT codes; TBuffer3D::SetRawSizes() 
  // seem to perform delete/new operations, also according to docs after viewer->AddObject() 
  // buffer can be reused; fine;
  static TBuffer3D buffer(TBuffer3DTypes::kGeneric);

  FillBuffer3D(buffer, reqSections, localFrame);

  //printf("%d %d %d\n", localBuffer3D->NbPnts(), localBuffer3D->NbSegs(), 
  //	 localBuffer3D->NbPols()); exit(0);

   // TODO: A box itself has nothing more as already described
   // by bounding box. How will viewer interpret?
   if (reqSections & TBuffer3D::kRawSizes) {
     if (buffer.SetRawSizes(mLocalBuffer3D->NbPnts(), 3 * mLocalBuffer3D->NbPnts(), 
			    mLocalBuffer3D->NbSegs(), 3 * mLocalBuffer3D->NbSegs(), 
			    mLocalBuffer3D->NbPols(), 5 * mLocalBuffer3D->NbPols())) {
       buffer.SetSectionsValid(TBuffer3D::kRawSizes);
     } //if
   } //if
   if ((reqSections & TBuffer3D::kRaw) && buffer.SectionsValid(TBuffer3D::kRawSizes)) {
     LocalSetPoints(buffer.fPnts);
     if (!buffer.fLocalFrame) {
       TransformPoints(buffer.fPnts, buffer.NbPnts());
     } //if
     
     LocalSetSegsAndPols(buffer);
     buffer.SetSectionsValid(TBuffer3D::kRaw);
   } //if    

   return buffer;
} // EicCompositeShape::GetBuffer3D()

// ---------------------------------------------------------------------------------------

Bool_t EicCompositeShape::PaintComposite(Option_t *option) const
{
  // Paint this composite shape into the current 3D viewer
  // Returns bool flag indicating if the caller should continue to
  // paint child objects
  Bool_t addChildren = kTRUE;
  
  TVirtualGeoPainter *painter = gGeoManager->GetGeomPainter();
  TVirtualViewer3D * viewer = gPad->GetViewer3D();
  if (!painter || !viewer) return kFALSE;
  
  // Does viewer prefer local frame positions?
  Bool_t localFrame = viewer->PreferLocalFrame();
  // Perform first fetch of buffer from the shape and try adding it
  // to the viewer
  const TBuffer3D & buffer = 
    GetBuffer3D(TBuffer3D::kCore|TBuffer3D::kBoundingBox/*|TBuffer3D::kShapeSpecific*/, localFrame); 
  Int_t reqSections = viewer->AddObject(buffer, &addChildren);
  
  // If the viewer requires additional sections fetch from the shape (if possible)
  // and add again
  if (reqSections != TBuffer3D::kNone) {
    GetBuffer3D(reqSections, localFrame);
    viewer->AddObject(buffer, &addChildren);
  } //if
  
  //printf("@@@   returning %d (true=%d) ...\n", addChildren, kTRUE);
  return addChildren;
} // EicCompositeShape::PaintComposite()

// =======================================================================================

ClassImp(EicCompositeShape)
ClassImp(EicBuffer3D)
